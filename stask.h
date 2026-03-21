/* stask - simple task manager
 * See LICENSE file for copyright and license details. */

#ifndef STASK_H
#define STASK_H

#include <sqlite3.h>

enum { TASK_TODO, TASK_DONE };

typedef struct {
	double bg[4];
	double card[4];
	double fg[4];
	double border[4];
	double muted[4];
	double muted_fg[4];
	double primary[4];
	double primary_fg[4];
	double sel[4];
	double accent[4];
	double accent_fg[4];
} Theme;

typedef struct {
	int id;
	char name[128];
	char created[32];
} List;

typedef struct {
	int id;
	int list_id;
	char text[512];
	int done;
	char added[32];
} Task;

typedef struct {
	sqlite3 *db;
	char dbpath[1024];
	char docdir[1024];
} TaskDB;

/* data.c */
int db_open(TaskDB *tdb);
void db_close(TaskDB *tdb);

int list_create(TaskDB *tdb, const char *name);
int list_delete(TaskDB *tdb, const char *name);
int list_all(TaskDB *tdb, List **out, int *count);
int list_by_name(TaskDB *tdb, const char *name, List *out);

int task_add(TaskDB *tdb, int list_id, const char *text);
int task_del(TaskDB *tdb, int task_id);
int task_done(TaskDB *tdb, int task_id);
int task_undo(TaskDB *tdb, int task_id);
int task_all(TaskDB *tdb, int list_id, Task **out, int *count);
int task_count(TaskDB *tdb, int list_id, int *total, int *done);
int task_get(TaskDB *tdb, int task_id, Task *out);
int task_update_text(TaskDB *tdb, int task_id, const char *text);
char *task_list_name(TaskDB *tdb, int list_id);

/* doc.c */
void doc_path(TaskDB *tdb, const char *listname, int task_id,
	char *buf, int len);
void doc_dir(TaskDB *tdb, const char *listname,
	char *buf, int len);
int doc_create(TaskDB *tdb, const char *listname,
	int task_id, const char *title, const char *date);
int doc_delete(TaskDB *tdb, const char *listname, int task_id);
int doc_delete_all(TaskDB *tdb, const char *listname);
int doc_edit(TaskDB *tdb, const char *listname, int task_id);
int doc_show(TaskDB *tdb, const char *listname, int task_id);
int doc_sync_title(TaskDB *tdb, const char *listname,
	int task_id);
void doc_sync_done(TaskDB *tdb, const char *listname,
	int task_id, int done);

/* gui.c */
int gui_run(TaskDB *tdb, int argc, char *argv[]);

#endif /* STASK_H */
