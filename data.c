/* stask - simple task manager
 * See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "stask.h"

static void
now_str(char *buf, int len)
{
	time_t t;
	struct tm *tm;

	t = time(NULL);
	tm = localtime(&t);
	strftime(buf, len, "%Y-%m-%d", tm);
}

static int
ensure_dir(const char *path)
{
	char tmp[1024];
	char *p;

	snprintf(tmp, sizeof(tmp), "%s", path);
	for (p = tmp + 1; *p; p++) {
		if (*p == '/') {
			*p = '\0';
			mkdir(tmp, 0755);
			*p = '/';
		}
	}
	return mkdir(tmp, 0755);
}

int
db_open(TaskDB *tdb)
{
	const char *home;
	char dir[1024];
	int rc;
	const char *sql;

	home = getenv("HOME");
	if (!home)
		home = "/tmp";

	snprintf(dir, sizeof(dir),
		"%s/.local/share/stask", home);
	ensure_dir(dir);

	snprintf(tdb->dbpath, sizeof(tdb->dbpath),
		"%s/stask.db", dir);
	snprintf(tdb->docdir, sizeof(tdb->docdir),
		"%s", dir);

	rc = sqlite3_open(tdb->dbpath, &tdb->db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "stask: cannot open db: %s\n",
			sqlite3_errmsg(tdb->db));
		return -1;
	}

	sql =
		"CREATE TABLE IF NOT EXISTS lists ("
		"  id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"  name TEXT UNIQUE NOT NULL,"
		"  created TEXT NOT NULL"
		");"
		"CREATE TABLE IF NOT EXISTS tasks ("
		"  id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"  list_id INTEGER NOT NULL,"
		"  text TEXT NOT NULL,"
		"  done INTEGER NOT NULL DEFAULT 0,"
		"  added TEXT NOT NULL,"
		"  FOREIGN KEY(list_id) REFERENCES lists(id)"
		"    ON DELETE CASCADE"
		");";

	rc = sqlite3_exec(tdb->db, sql, NULL, NULL, NULL);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "stask: schema error: %s\n",
			sqlite3_errmsg(tdb->db));
		return -1;
	}

	/* enable foreign keys */
	sqlite3_exec(tdb->db,
		"PRAGMA foreign_keys = ON;",
		NULL, NULL, NULL);

	return 0;
}

void
db_close(TaskDB *tdb)
{
	if (tdb->db)
		sqlite3_close(tdb->db);
	tdb->db = NULL;
}

int
list_create(TaskDB *tdb, const char *name)
{
	sqlite3_stmt *stmt;
	char date[32];
	int rc;

	now_str(date, sizeof(date));
	rc = sqlite3_prepare_v2(tdb->db,
		"INSERT INTO lists (name, created) VALUES (?, ?);",
		-1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return -1;

	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, date, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	return rc == SQLITE_DONE ? 0 : -1;
}

int
list_delete(TaskDB *tdb, const char *name)
{
	sqlite3_stmt *stmt;
	int rc;

	/* delete tasks first (in case FK not enforced) */
	rc = sqlite3_prepare_v2(tdb->db,
		"DELETE FROM tasks WHERE list_id = "
		"(SELECT id FROM lists WHERE name = ?);",
		-1, &stmt, NULL);
	if (rc == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, name, -1,
			SQLITE_STATIC);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}

	rc = sqlite3_prepare_v2(tdb->db,
		"DELETE FROM lists WHERE name = ?;",
		-1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return -1;

	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	return rc == SQLITE_DONE ? 0 : -1;
}

int
list_all(TaskDB *tdb, List **out, int *count)
{
	sqlite3_stmt *stmt;
	int rc, n, cap;
	List *arr;

	*out = NULL;
	*count = 0;

	rc = sqlite3_prepare_v2(tdb->db,
		"SELECT id, name, created FROM lists "
		"ORDER BY name;",
		-1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return -1;

	n = 0;
	cap = 16;
	arr = malloc(cap * sizeof(List));
	if (!arr) {
		sqlite3_finalize(stmt);
		return -1;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		if (n >= cap) {
			cap *= 2;
			arr = realloc(arr, cap * sizeof(List));
			if (!arr) {
				sqlite3_finalize(stmt);
				return -1;
			}
		}
		arr[n].id = sqlite3_column_int(stmt, 0);
		snprintf(arr[n].name, sizeof(arr[n].name),
			"%s", sqlite3_column_text(stmt, 1));
		snprintf(arr[n].created,
			sizeof(arr[n].created),
			"%s", sqlite3_column_text(stmt, 2));
		n++;
	}

	sqlite3_finalize(stmt);
	*out = arr;
	*count = n;
	return 0;
}

int
list_by_name(TaskDB *tdb, const char *name, List *out)
{
	sqlite3_stmt *stmt;
	int rc;

	rc = sqlite3_prepare_v2(tdb->db,
		"SELECT id, name, created FROM lists "
		"WHERE name = ?;",
		-1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return -1;

	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return -1;
	}

	out->id = sqlite3_column_int(stmt, 0);
	snprintf(out->name, sizeof(out->name),
		"%s", sqlite3_column_text(stmt, 1));
	snprintf(out->created, sizeof(out->created),
		"%s", sqlite3_column_text(stmt, 2));
	sqlite3_finalize(stmt);
	return 0;
}

int
task_add(TaskDB *tdb, int list_id, const char *text)
{
	sqlite3_stmt *stmt;
	char date[32];
	int rc;

	now_str(date, sizeof(date));
	rc = sqlite3_prepare_v2(tdb->db,
		"INSERT INTO tasks (list_id, text, done, added)"
		" VALUES (?, ?, 0, ?);",
		-1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return -1;

	sqlite3_bind_int(stmt, 1, list_id);
	sqlite3_bind_text(stmt, 2, text, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, date, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	return rc == SQLITE_DONE ? 0 : -1;
}

int
task_del(TaskDB *tdb, int task_id)
{
	sqlite3_stmt *stmt;
	int rc;

	rc = sqlite3_prepare_v2(tdb->db,
		"DELETE FROM tasks WHERE id = ?;",
		-1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return -1;

	sqlite3_bind_int(stmt, 1, task_id);
	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	return rc == SQLITE_DONE ? 0 : -1;
}

int
task_done(TaskDB *tdb, int task_id)
{
	sqlite3_stmt *stmt;
	int rc;

	rc = sqlite3_prepare_v2(tdb->db,
		"UPDATE tasks SET done = 1 WHERE id = ?;",
		-1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return -1;

	sqlite3_bind_int(stmt, 1, task_id);
	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	return rc == SQLITE_DONE ? 0 : -1;
}

int
task_undo(TaskDB *tdb, int task_id)
{
	sqlite3_stmt *stmt;
	int rc;

	rc = sqlite3_prepare_v2(tdb->db,
		"UPDATE tasks SET done = 0 WHERE id = ?;",
		-1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return -1;

	sqlite3_bind_int(stmt, 1, task_id);
	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	return rc == SQLITE_DONE ? 0 : -1;
}

int
task_all(TaskDB *tdb, int list_id, Task **out, int *count)
{
	sqlite3_stmt *stmt;
	int rc, n, cap;
	Task *arr;

	*out = NULL;
	*count = 0;

	rc = sqlite3_prepare_v2(tdb->db,
		"SELECT id, list_id, text, done, added "
		"FROM tasks WHERE list_id = ? "
		"ORDER BY done ASC, id ASC;",
		-1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return -1;

	sqlite3_bind_int(stmt, 1, list_id);

	n = 0;
	cap = 32;
	arr = malloc(cap * sizeof(Task));
	if (!arr) {
		sqlite3_finalize(stmt);
		return -1;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		if (n >= cap) {
			cap *= 2;
			arr = realloc(arr, cap * sizeof(Task));
			if (!arr) {
				sqlite3_finalize(stmt);
				return -1;
			}
		}
		arr[n].id = sqlite3_column_int(stmt, 0);
		arr[n].list_id = sqlite3_column_int(stmt, 1);
		snprintf(arr[n].text, sizeof(arr[n].text),
			"%s", sqlite3_column_text(stmt, 2));
		arr[n].done = sqlite3_column_int(stmt, 3);
		snprintf(arr[n].added, sizeof(arr[n].added),
			"%s", sqlite3_column_text(stmt, 4));
		n++;
	}

	sqlite3_finalize(stmt);
	*out = arr;
	*count = n;
	return 0;
}

int
task_count(TaskDB *tdb, int list_id, int *total, int *done)
{
	sqlite3_stmt *stmt;
	int rc;

	*total = 0;
	*done = 0;

	rc = sqlite3_prepare_v2(tdb->db,
		"SELECT COUNT(*), SUM(done) FROM tasks "
		"WHERE list_id = ?;",
		-1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return -1;

	sqlite3_bind_int(stmt, 1, list_id);
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		*total = sqlite3_column_int(stmt, 0);
		*done = sqlite3_column_int(stmt, 1);
	}
	sqlite3_finalize(stmt);
	return 0;
}

int
task_get(TaskDB *tdb, int task_id, Task *out)
{
	sqlite3_stmt *stmt;
	int rc;

	rc = sqlite3_prepare_v2(tdb->db,
		"SELECT id, list_id, text, done, added "
		"FROM tasks WHERE id = ?;",
		-1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return -1;

	sqlite3_bind_int(stmt, 1, task_id);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return -1;
	}

	out->id = sqlite3_column_int(stmt, 0);
	out->list_id = sqlite3_column_int(stmt, 1);
	snprintf(out->text, sizeof(out->text),
		"%s", sqlite3_column_text(stmt, 2));
	out->done = sqlite3_column_int(stmt, 3);
	snprintf(out->added, sizeof(out->added),
		"%s", sqlite3_column_text(stmt, 4));
	sqlite3_finalize(stmt);
	return 0;
}

int
task_update_text(TaskDB *tdb, int task_id, const char *text)
{
	sqlite3_stmt *stmt;
	int rc;

	rc = sqlite3_prepare_v2(tdb->db,
		"UPDATE tasks SET text = ? WHERE id = ?;",
		-1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return -1;

	sqlite3_bind_text(stmt, 1, text, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, task_id);
	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	return rc == SQLITE_DONE ? 0 : -1;
}

char *
task_list_name(TaskDB *tdb, int list_id)
{
	sqlite3_stmt *stmt;
	int rc;
	static char name[128];

	rc = sqlite3_prepare_v2(tdb->db,
		"SELECT name FROM lists WHERE id = ?;",
		-1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return NULL;

	sqlite3_bind_int(stmt, 1, list_id);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return NULL;
	}

	snprintf(name, sizeof(name),
		"%s", sqlite3_column_text(stmt, 0));
	sqlite3_finalize(stmt);
	return name;
}
