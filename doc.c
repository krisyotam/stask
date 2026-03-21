/* stask - simple task manager
 * See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "stask.h"

static void
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
	mkdir(tmp, 0755);
}

void
doc_dir(TaskDB *tdb, const char *listname, char *buf, int len)
{
	snprintf(buf, len, "%s/docs/%s", tdb->docdir, listname);
}

void
doc_path(TaskDB *tdb, const char *listname, int task_id,
	char *buf, int len)
{
	snprintf(buf, len, "%s/docs/%s/%d.md",
		tdb->docdir, listname, task_id);
}

int
doc_create(TaskDB *tdb, const char *listname,
	int task_id, const char *title, const char *date)
{
	char path[1024], dir[1024];
	FILE *fp;

	doc_dir(tdb, listname, dir, sizeof(dir));
	ensure_dir(dir);

	doc_path(tdb, listname, task_id, path, sizeof(path));
	fp = fopen(path, "w");
	if (!fp) {
		fprintf(stderr, "stask: cannot create %s\n", path);
		return -1;
	}

	fprintf(fp, "---\n");
	fprintf(fp, "title: \"%s\"\n", title);
	fprintf(fp, "date: %s\n", date);
	fprintf(fp, "done: false\n");
	fprintf(fp, "---\n\n");

	fclose(fp);
	return 0;
}

int
doc_delete(TaskDB *tdb, const char *listname, int task_id)
{
	char path[1024];

	doc_path(tdb, listname, task_id, path, sizeof(path));
	return unlink(path);
}

int
doc_delete_all(TaskDB *tdb, const char *listname)
{
	Task *tasks;
	int i, n;
	List list;
	char dir[1024];

	if (list_by_name(tdb, listname, &list) < 0)
		return -1;

	if (task_all(tdb, list.id, &tasks, &n) == 0) {
		for (i = 0; i < n; i++)
			doc_delete(tdb, listname, tasks[i].id);
		free(tasks);
	}

	doc_dir(tdb, listname, dir, sizeof(dir));
	rmdir(dir); /* only succeeds if empty */
	return 0;
}

int
doc_edit(TaskDB *tdb, const char *listname, int task_id)
{
	char path[1024];
	const char *editor;
	char cmd[2048];
	int ret;

	doc_path(tdb, listname, task_id, path, sizeof(path));

	if (access(path, F_OK) != 0) {
		fprintf(stderr,
			"stask: no doc for task %d\n", task_id);
		return -1;
	}

	editor = getenv("EDITOR");
	if (!editor) {
		if (access("/usr/bin/nvim", X_OK) == 0)
			editor = "nvim";
		else if (access("/usr/bin/vim", X_OK) == 0)
			editor = "vim";
		else
			editor = "vi";
	}

	snprintf(cmd, sizeof(cmd), "%s '%s'", editor, path);
	ret = system(cmd);

	if (ret == 0)
		doc_sync_title(tdb, listname, task_id);

	return ret;
}

int
doc_show(TaskDB *tdb, const char *listname, int task_id)
{
	char path[1024];
	FILE *fp;
	char line[1024];

	doc_path(tdb, listname, task_id, path, sizeof(path));

	fp = fopen(path, "r");
	if (!fp) {
		fprintf(stderr,
			"stask: no doc for task %d\n", task_id);
		return -1;
	}

	while (fgets(line, sizeof(line), fp))
		fputs(line, stdout);

	fclose(fp);
	return 0;
}

/* parse title from yaml frontmatter */
static int
parse_title(const char *path, char *title, int len)
{
	FILE *fp;
	char line[1024];
	int in_fm = 0;
	char *p, *end;

	fp = fopen(path, "r");
	if (!fp)
		return -1;

	while (fgets(line, sizeof(line), fp)) {
		/* trim trailing newline */
		p = line + strlen(line) - 1;
		while (p >= line && (*p == '\n' || *p == '\r'))
			*p-- = '\0';

		if (strcmp(line, "---") == 0) {
			if (!in_fm) {
				in_fm = 1;
				continue;
			} else {
				break; /* end of frontmatter */
			}
		}

		if (!in_fm)
			continue;

		if (strncmp(line, "title:", 6) == 0) {
			p = line + 6;
			while (*p == ' ' || *p == '\t')
				p++;
			/* strip quotes */
			if (*p == '"' || *p == '\'') {
				p++;
				end = p + strlen(p) - 1;
				if (end > p && (*end == '"' || *end == '\''))
					*end = '\0';
			}
			snprintf(title, len, "%s", p);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);
	return -1;
}

/* update done field in frontmatter */
static int
update_fm_field(const char *path, const char *field,
	const char *value)
{
	FILE *fp;
	char *buf;
	long fsize;
	char line[1024], out[65536];
	int in_fm = 0, found = 0, olen = 0;
	char *p, *src;

	fp = fopen(path, "r");
	if (!fp)
		return -1;

	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	buf = malloc(fsize + 1);
	if (!buf) {
		fclose(fp);
		return -1;
	}
	fsize = fread(buf, 1, fsize, fp);
	buf[fsize] = '\0';
	fclose(fp);

	src = buf;
	out[0] = '\0';

	while (*src) {
		/* read a line */
		p = strchr(src, '\n');
		if (p) {
			int llen = (int)(p - src);
			if (llen >= (int)sizeof(line) - 1)
				llen = sizeof(line) - 2;
			memcpy(line, src, llen);
			line[llen] = '\0';
			src = p + 1;
		} else {
			snprintf(line, sizeof(line), "%s", src);
			src += strlen(src);
		}

		if (strcmp(line, "---") == 0) {
			if (!in_fm) {
				in_fm = 1;
			} else {
				in_fm = 0;
			}
			olen += snprintf(out + olen,
				sizeof(out) - olen, "%s\n", line);
			continue;
		}

		if (in_fm && strncmp(line, field,
				strlen(field)) == 0
			&& line[strlen(field)] == ':') {
			olen += snprintf(out + olen,
				sizeof(out) - olen,
				"%s: %s\n", field, value);
			found = 1;
		} else {
			olen += snprintf(out + olen,
				sizeof(out) - olen,
				"%s\n", line);
		}
	}

	free(buf);

	if (!found)
		return -1;

	fp = fopen(path, "w");
	if (!fp)
		return -1;
	fwrite(out, 1, olen, fp);
	fclose(fp);
	return 0;
}

int
doc_sync_title(TaskDB *tdb, const char *listname,
	int task_id)
{
	char path[1024], title[512];

	doc_path(tdb, listname, task_id, path, sizeof(path));

	if (access(path, F_OK) != 0)
		return -1;

	if (parse_title(path, title, sizeof(title)) < 0)
		return -1;

	if (title[0] == '\0')
		return -1;

	return task_update_text(tdb, task_id, title);
}

/* called from data.c when task_done/task_undo changes */
void
doc_sync_done(TaskDB *tdb, const char *listname,
	int task_id, int done)
{
	char path[1024];

	doc_path(tdb, listname, task_id, path, sizeof(path));

	if (access(path, F_OK) != 0)
		return;

	update_fm_field(path, "done",
		done ? "true" : "false");
}
