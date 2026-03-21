/* stask - simple task manager
 * See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stask.h"
#include "config.h"

static void
usage(void)
{
	fprintf(stderr,
		"usage: stask [-g]\n"
		"       stask [ls]\n"
		"       stask new <list>\n"
		"       stask rm <list>\n"
		"       stask <list>\n"
		"       stask <list> add <text>\n"
		"       stask <list> done <id>\n"
		"       stask <list> undo <id>\n"
		"       stask <list> del <id>\n");
	exit(1);
}

static void
cmd_ls(TaskDB *tdb)
{
	List *lists;
	int i, n, total, done;

	if (list_all(tdb, &lists, &n) < 0) {
		fprintf(stderr, "stask: cannot list\n");
		return;
	}

	if (n == 0) {
		printf("No lists. Create one with: stask new <name>\n");
		free(lists);
		return;
	}

	printf("%-20s %10s %10s  %s\n",
		"LIST", "TASKS", "DONE", "CREATED");
	for (i = 0; i < n; i++) {
		task_count(tdb, lists[i].id, &total, &done);
		printf("%-20s %10d %10d  %s\n",
			lists[i].name, total, done,
			lists[i].created);
	}
	free(lists);
}

static void
cmd_show(TaskDB *tdb, const char *name)
{
	List list;
	Task *tasks;
	int i, n;

	if (list_by_name(tdb, name, &list) < 0) {
		fprintf(stderr,
			"stask: list '%s' not found\n", name);
		return;
	}

	if (task_all(tdb, list.id, &tasks, &n) < 0) {
		fprintf(stderr, "stask: cannot read tasks\n");
		return;
	}

	if (n == 0) {
		printf("%s: empty\n", name);
		free(tasks);
		return;
	}

	printf("%s:\n", name);
	for (i = 0; i < n; i++) {
		printf("  [%c] %-4d %-40s %s\n",
			tasks[i].done ? 'x' : ' ',
			tasks[i].id,
			tasks[i].text,
			tasks[i].added);
	}
	free(tasks);
}

static void
cmd_new(TaskDB *tdb, const char *name)
{
	if (list_create(tdb, name) < 0)
		fprintf(stderr,
			"stask: cannot create '%s' "
			"(already exists?)\n", name);
	else
		printf("Created list '%s'\n", name);
}

static void
cmd_rm(TaskDB *tdb, const char *name)
{
	if (list_delete(tdb, name) < 0)
		fprintf(stderr,
			"stask: cannot remove '%s'\n", name);
	else
		printf("Removed list '%s'\n", name);
}

static void
cmd_add(TaskDB *tdb, const char *listname, const char *text)
{
	List list;

	if (list_by_name(tdb, listname, &list) < 0) {
		fprintf(stderr,
			"stask: list '%s' not found\n",
			listname);
		return;
	}
	if (task_add(tdb, list.id, text) < 0)
		fprintf(stderr, "stask: cannot add task\n");
	else
		printf("Added to '%s'\n", listname);
}

static void
cmd_done(TaskDB *tdb, int id)
{
	if (task_done(tdb, id) < 0)
		fprintf(stderr,
			"stask: cannot mark done: %d\n", id);
}

static void
cmd_undo(TaskDB *tdb, int id)
{
	if (task_undo(tdb, id) < 0)
		fprintf(stderr,
			"stask: cannot undo: %d\n", id);
}

static void
cmd_del(TaskDB *tdb, int id)
{
	if (task_del(tdb, id) < 0)
		fprintf(stderr,
			"stask: cannot delete: %d\n", id);
}

int
main(int argc, char *argv[])
{
	TaskDB tdb;
	int id;

	memset(&tdb, 0, sizeof(tdb));

	/* gui mode */
	if (argc >= 2 && strcmp(argv[1], "-g") == 0) {
		if (db_open(&tdb) < 0)
			return 1;
		id = gui_run(&tdb, argc, argv);
		db_close(&tdb);
		return id;
	}

	if (db_open(&tdb) < 0)
		return 1;

	if (argc < 2 || strcmp(argv[1], "ls") == 0) {
		cmd_ls(&tdb);
	} else if (strcmp(argv[1], "new") == 0) {
		if (argc < 3) usage();
		cmd_new(&tdb, argv[2]);
	} else if (strcmp(argv[1], "rm") == 0) {
		if (argc < 3) usage();
		cmd_rm(&tdb, argv[2]);
	} else if (argc == 2) {
		/* stask <list> - show tasks */
		cmd_show(&tdb, argv[1]);
	} else if (argc >= 3) {
		/* stask <list> <cmd> [args] */
		if (strcmp(argv[2], "add") == 0) {
			if (argc < 4) usage();
			cmd_add(&tdb, argv[1], argv[3]);
		} else if (strcmp(argv[2], "done") == 0) {
			if (argc < 4) usage();
			id = atoi(argv[3]);
			cmd_done(&tdb, id);
		} else if (strcmp(argv[2], "undo") == 0) {
			if (argc < 4) usage();
			id = atoi(argv[3]);
			cmd_undo(&tdb, id);
		} else if (strcmp(argv[2], "del") == 0) {
			if (argc < 4) usage();
			id = atoi(argv[3]);
			cmd_del(&tdb, id);
		} else {
			usage();
		}
	} else {
		usage();
	}

	db_close(&tdb);
	return 0;
}
