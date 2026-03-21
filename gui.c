/* stask - simple task manager
 * See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include "stask.h"
#include "config.h"

typedef struct {
	TaskDB *tdb;
	int dark;
	const Theme *theme;

	GtkWidget *window;
	GtkWidget *theme_btn;
	GtkWidget *list_box;
	GtkWidget *task_box;
	GtkWidget *list_label;
	GtkWidget *entry;
	GtkWidget *status;
	GtkCssProvider *theme_css;

	int cur_list;       /* selected list id, -1 = none */
	char cur_name[128]; /* selected list name */
} Gui;

static Gui gui;

/* theme CSS */

static void
apply_theme_css(Gui *g)
{
	char css[4096];
	const Theme *t;
	int bg[3], fg[3], bd[3], mt[3], cd[3], mfg[3];
	int pr[3], pfg[3], ac[3], afg[3];

	t = g->theme;
	bg[0] = (int)(t->bg[0] * 255);
	bg[1] = (int)(t->bg[1] * 255);
	bg[2] = (int)(t->bg[2] * 255);
	fg[0] = (int)(t->fg[0] * 255);
	fg[1] = (int)(t->fg[1] * 255);
	fg[2] = (int)(t->fg[2] * 255);
	bd[0] = (int)(t->border[0] * 255);
	bd[1] = (int)(t->border[1] * 255);
	bd[2] = (int)(t->border[2] * 255);
	mt[0] = (int)(t->muted[0] * 255);
	mt[1] = (int)(t->muted[1] * 255);
	mt[2] = (int)(t->muted[2] * 255);
	cd[0] = (int)(t->card[0] * 255);
	cd[1] = (int)(t->card[1] * 255);
	cd[2] = (int)(t->card[2] * 255);
	mfg[0] = (int)(t->muted_fg[0] * 255);
	mfg[1] = (int)(t->muted_fg[1] * 255);
	mfg[2] = (int)(t->muted_fg[2] * 255);
	pr[0] = (int)(t->primary[0] * 255);
	pr[1] = (int)(t->primary[1] * 255);
	pr[2] = (int)(t->primary[2] * 255);
	pfg[0] = (int)(t->primary_fg[0] * 255);
	pfg[1] = (int)(t->primary_fg[1] * 255);
	pfg[2] = (int)(t->primary_fg[2] * 255);
	ac[0] = (int)(t->accent[0] * 255);
	ac[1] = (int)(t->accent[1] * 255);
	ac[2] = (int)(t->accent[2] * 255);
	afg[0] = (int)(t->accent_fg[0] * 255);
	afg[1] = (int)(t->accent_fg[1] * 255);
	afg[2] = (int)(t->accent_fg[2] * 255);

	snprintf(css, sizeof(css),
		"window, box, paned {"
		"  background: rgb(%d,%d,%d);"
		"  color: rgb(%d,%d,%d);"
		"}"
		"headerbar {"
		"  background: rgb(%d,%d,%d);"
		"  border-bottom: 1px solid rgb(%d,%d,%d);"
		"  color: rgb(%d,%d,%d);"
		"}"
		"headerbar button {"
		"  background: rgb(%d,%d,%d);"
		"  color: rgb(%d,%d,%d);"
		"  border: 1px solid rgb(%d,%d,%d);"
		"}"
		"headerbar button:hover {"
		"  background: rgb(%d,%d,%d);"
		"}"
		"label {"
		"  color: rgb(%d,%d,%d);"
		"}"
		"entry {"
		"  background: rgb(%d,%d,%d);"
		"  color: rgb(%d,%d,%d);"
		"  border: 1px solid rgb(%d,%d,%d);"
		"  caret-color: rgb(%d,%d,%d);"
		"}"
		"listboxrow {"
		"  background: rgb(%d,%d,%d);"
		"  color: rgb(%d,%d,%d);"
		"  padding: 6px 10px;"
		"  border-bottom: 1px solid rgb(%d,%d,%d);"
		"}"
		"listboxrow:selected {"
		"  background: rgb(%d,%d,%d);"
		"  color: rgb(%d,%d,%d);"
		"}"
		"separator {"
		"  background: rgb(%d,%d,%d);"
		"  min-width: 1px; min-height: 1px;"
		"}"
		"checkbutton {"
		"  color: rgb(%d,%d,%d);"
		"}"
		"checkbutton indicator {"
		"  border: 1px solid rgb(%d,%d,%d);"
		"  background: rgb(%d,%d,%d);"
		"}"
		".task-done label {"
		"  color: rgb(%d,%d,%d);"
		"}"
		".list-header {"
		"  font-weight: bold;"
		"  font-size: 14px;"
		"  padding: 8px 12px;"
		"}"
		".task-date {"
		"  color: rgb(%d,%d,%d);"
		"  font-size: 11px;"
		"}"
		"popover > contents {"
		"  background: rgb(%d,%d,%d);"
		"  border: 1px solid rgb(%d,%d,%d);"
		"}"
		"popover button {"
		"  background: transparent;"
		"  color: rgb(%d,%d,%d);"
		"  padding: 6px 12px;"
		"}"
		"popover button:hover {"
		"  background: rgb(%d,%d,%d);"
		"}",
		/* window bg */ bg[0], bg[1], bg[2],
		/* window fg */ fg[0], fg[1], fg[2],
		/* headerbar bg */ bg[0], bg[1], bg[2],
		/* headerbar border */ bd[0], bd[1], bd[2],
		/* headerbar fg */ fg[0], fg[1], fg[2],
		/* btn bg */ mt[0], mt[1], mt[2],
		/* btn fg */ fg[0], fg[1], fg[2],
		/* btn border */ bd[0], bd[1], bd[2],
		/* btn hover */ bd[0], bd[1], bd[2],
		/* label */ fg[0], fg[1], fg[2],
		/* entry bg */ cd[0], cd[1], cd[2],
		/* entry fg */ fg[0], fg[1], fg[2],
		/* entry border */ bd[0], bd[1], bd[2],
		/* entry caret */ pr[0], pr[1], pr[2],
		/* listrow bg */ cd[0], cd[1], cd[2],
		/* listrow fg */ fg[0], fg[1], fg[2],
		/* listrow border */ bd[0], bd[1], bd[2],
		/* listrow sel bg */ ac[0], ac[1], ac[2],
		/* listrow sel fg */ afg[0], afg[1], afg[2],
		/* separator */ bd[0], bd[1], bd[2],
		/* checkbutton fg */ fg[0], fg[1], fg[2],
		/* check border */ bd[0], bd[1], bd[2],
		/* check bg */ cd[0], cd[1], cd[2],
		/* done label */ mfg[0], mfg[1], mfg[2],
		/* date label */ mfg[0], mfg[1], mfg[2],
		/* popover bg */ cd[0], cd[1], cd[2],
		/* popover border */ bd[0], bd[1], bd[2],
		/* popover btn fg */ fg[0], fg[1], fg[2],
		/* popover hover */ mt[0], mt[1], mt[2]
	);

	if (!g->theme_css) {
		g->theme_css = gtk_css_provider_new();
		gtk_style_context_add_provider_for_display(
			gdk_display_get_default(),
			GTK_STYLE_PROVIDER(g->theme_css),
			GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
	gtk_css_provider_load_from_string(g->theme_css, css);
}

static void
toggle_theme(Gui *g)
{
	g->dark = !g->dark;
	g->theme = g->dark ? &theme_dark : &theme_light;
	apply_theme_css(g);
	if (g->theme_btn)
		gtk_button_set_label(
			GTK_BUTTON(g->theme_btn),
			g->dark ? "Light" : "Dark");
}

/* refresh task list — forward declaration */

static void
refresh_tasks(Gui *g);

static void
on_check_toggled(GtkCheckButton *cb, gpointer data)
{
	Gui *g;
	int task_id;
	gboolean active;

	g = &gui;
	task_id = GPOINTER_TO_INT(data);
	active = gtk_check_button_get_active(cb);
	if (active)
		task_done(g->tdb, task_id);
	else
		task_undo(g->tdb, task_id);
	refresh_tasks(g);
}

/* refresh list sidebar */

static void
on_list_selected(GtkListBox *box, GtkListBoxRow *row,
	gpointer data)
{
	Gui *g;
	const char *name;
	GtkWidget *label;
	List list;

	(void)box;
	g = (Gui *)data;

	if (!row) {
		g->cur_list = -1;
		g->cur_name[0] = '\0';
		refresh_tasks(g);
		return;
	}

	label = gtk_widget_get_first_child(
		GTK_WIDGET(row));
	if (!label)
		return;
	name = gtk_label_get_text(GTK_LABEL(label));
	if (list_by_name(g->tdb, name, &list) < 0)
		return;

	g->cur_list = list.id;
	snprintf(g->cur_name, sizeof(g->cur_name),
		"%s", list.name);
	refresh_tasks(g);
}

static void
refresh_lists(Gui *g)
{
	List *lists;
	GtkWidget *row, *label;
	int i, n;

	while ((row = gtk_widget_get_first_child(
			GTK_WIDGET(g->list_box))))
		gtk_list_box_remove(
			GTK_LIST_BOX(g->list_box), row);

	if (list_all(g->tdb, &lists, &n) < 0)
		return;

	for (i = 0; i < n; i++) {
		label = gtk_label_new(lists[i].name);
		gtk_label_set_xalign(
			GTK_LABEL(label), 0.0);
		gtk_list_box_append(
			GTK_LIST_BOX(g->list_box), label);

		/* re-select current list */
		if (lists[i].id == g->cur_list) {
			row = gtk_widget_get_parent(label);
			gtk_list_box_select_row(
				GTK_LIST_BOX(g->list_box),
				GTK_LIST_BOX_ROW(row));
		}
	}
	free(lists);
}

/* new list dialog */

static void
on_new_list_response(GtkDialog *dlg, int response,
	gpointer data)
{
	Gui *g;
	GtkWidget *entry;
	const char *name;

	g = (Gui *)data;
	if (response == GTK_RESPONSE_OK) {
		entry = g_object_get_data(
			G_OBJECT(dlg), "entry");
		name = gtk_editable_get_text(
			GTK_EDITABLE(entry));
		if (name && name[0]) {
			list_create(g->tdb, name);
			refresh_lists(g);
		}
	}
	gtk_window_destroy(GTK_WINDOW(dlg));
}

static void
on_new_list(GtkButton *btn, gpointer data)
{
	Gui *g;
	GtkWidget *dlg, *box, *entry, *label;

	(void)btn;
	g = (Gui *)data;

	dlg = gtk_dialog_new_with_buttons("New List",
		GTK_WINDOW(g->window),
		GTK_DIALOG_MODAL
		| GTK_DIALOG_DESTROY_WITH_PARENT,
		"Create", GTK_RESPONSE_OK,
		"Cancel", GTK_RESPONSE_CANCEL,
		NULL);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
	gtk_widget_set_margin_start(box, 16);
	gtk_widget_set_margin_end(box, 16);
	gtk_widget_set_margin_top(box, 12);
	gtk_widget_set_margin_bottom(box, 12);

	label = gtk_label_new("List name:");
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);
	gtk_box_append(GTK_BOX(box), label);

	entry = gtk_entry_new();
	gtk_box_append(GTK_BOX(box), entry);

	g_object_set_data(G_OBJECT(dlg), "entry", entry);

	gtk_box_append(
		GTK_BOX(gtk_dialog_get_content_area(
			GTK_DIALOG(dlg))),
		box);

	g_signal_connect(dlg, "response",
		G_CALLBACK(on_new_list_response), g);
	gtk_window_present(GTK_WINDOW(dlg));
}

/* delete list */

static void
on_del_list(GtkButton *btn, gpointer data)
{
	Gui *g;

	(void)btn;
	g = (Gui *)data;
	if (g->cur_list < 0)
		return;

	list_delete(g->tdb, g->cur_name);
	g->cur_list = -1;
	g->cur_name[0] = '\0';
	refresh_lists(g);
	refresh_tasks(g);
}

/* add task via entry */

static void
on_entry_activate(GtkEntry *entry, gpointer data)
{
	Gui *g;
	GtkEntryBuffer *buf;
	const char *text;

	g = (Gui *)data;
	if (g->cur_list < 0)
		return;

	buf = gtk_entry_get_buffer(entry);
	text = gtk_entry_buffer_get_text(buf);
	if (!text || !text[0])
		return;

	task_add(g->tdb, g->cur_list, text);
	gtk_entry_buffer_set_text(buf, "", 0);
	refresh_tasks(g);
}

/* delete selected task */

static void
on_del_task(GtkButton *btn, gpointer data)
{
	Gui *g;
	GtkListBoxRow *row;
	GtkWidget *hbox, *check;
	int task_id;

	(void)btn;
	g = (Gui *)data;

	row = gtk_list_box_get_selected_row(
		GTK_LIST_BOX(g->task_box));
	if (!row)
		return;

	hbox = gtk_widget_get_first_child(
		GTK_WIDGET(row));
	if (!hbox)
		return;

	/* first child of hbox is the checkbutton */
	check = gtk_widget_get_first_child(hbox);
	if (!check)
		return;

	/* get task id from the signal data — we need
	 * another approach: store id on the row */
	task_id = GPOINTER_TO_INT(
		g_object_get_data(
			G_OBJECT(check), "task-id"));
	if (task_id > 0) {
		task_del(g->tdb, task_id);
		refresh_tasks(g);
	}
}

static void
refresh_tasks(Gui *g)
{
	Task *tasks;
	GtkWidget *row, *hbox, *check, *label, *date;
	char buf[64];
	int i, n, total, done;

	while ((row = gtk_widget_get_first_child(
			GTK_WIDGET(g->task_box))))
		gtk_list_box_remove(
			GTK_LIST_BOX(g->task_box), row);

	if (g->cur_list < 0) {
		gtk_label_set_text(
			GTK_LABEL(g->list_label),
			"Select a list");
		gtk_label_set_text(
			GTK_LABEL(g->status), "");
		return;
	}

	gtk_label_set_text(
		GTK_LABEL(g->list_label), g->cur_name);

	if (task_all(g->tdb, g->cur_list, &tasks, &n) < 0)
		return;

	for (i = 0; i < n; i++) {
		hbox = gtk_box_new(
			GTK_ORIENTATION_HORIZONTAL, 8);

		check = gtk_check_button_new();
		gtk_check_button_set_active(
			GTK_CHECK_BUTTON(check),
			tasks[i].done);
		g_object_set_data(G_OBJECT(check),
			"task-id",
			GINT_TO_POINTER(tasks[i].id));
		g_signal_connect(check, "toggled",
			G_CALLBACK(on_check_toggled),
			GINT_TO_POINTER(tasks[i].id));
		gtk_box_append(GTK_BOX(hbox), check);

		label = gtk_label_new(tasks[i].text);
		gtk_label_set_xalign(
			GTK_LABEL(label), 0.0);
		gtk_widget_set_hexpand(label, TRUE);
		gtk_label_set_ellipsize(
			GTK_LABEL(label),
			PANGO_ELLIPSIZE_END);
		gtk_box_append(GTK_BOX(hbox), label);

		date = gtk_label_new(tasks[i].added);
		gtk_widget_add_css_class(date, "task-date");
		gtk_box_append(GTK_BOX(hbox), date);

		gtk_list_box_append(
			GTK_LIST_BOX(g->task_box), hbox);

		if (tasks[i].done) {
			row = gtk_widget_get_parent(hbox);
			gtk_widget_add_css_class(
				row, "task-done");
		}
	}

	task_count(g->tdb, g->cur_list, &total, &done);
	snprintf(buf, sizeof(buf),
		" %s | %d tasks, %d done",
		g->cur_name, total, done);
	gtk_label_set_text(GTK_LABEL(g->status), buf);

	free(tasks);
}

/* toolbar button helper */

static GtkWidget *
make_btn(const char *label, GCallback cb, gpointer data)
{
	GtkWidget *btn;

	btn = gtk_button_new_with_label(label);
	g_signal_connect(btn, "clicked", cb, data);
	return btn;
}

static void
on_theme_toggle(GtkButton *btn, gpointer data)
{
	(void)btn;
	toggle_theme((Gui *)data);
}

static void
activate(GtkApplication *app, gpointer data)
{
	GtkWidget *window, *header, *paned;
	GtkWidget *left_box, *right_box;
	GtkWidget *scroll_lists, *scroll_tasks;
	GtkWidget *entry, *status, *list_label;
	GtkWidget *entry_box;
	GtkCssProvider *css;
	Gui *g;

	(void)data;
	g = &gui;

	/* squared corners CSS */
	css = gtk_css_provider_new();
	gtk_css_provider_load_from_string(css, app_css);
	gtk_style_context_add_provider_for_display(
		gdk_display_get_default(),
		GTK_STYLE_PROVIDER(css),
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	apply_theme_css(g);

	/* window */
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "stask");
	gtk_window_set_default_size(GTK_WINDOW(window),
		800, 600);
	g->window = window;

	/* header bar */
	header = gtk_header_bar_new();

	g->theme_btn = make_btn(
		g->dark ? "Light" : "Dark",
		G_CALLBACK(on_theme_toggle), g);
	gtk_header_bar_pack_start(
		GTK_HEADER_BAR(header), g->theme_btn);
	gtk_header_bar_pack_start(
		GTK_HEADER_BAR(header),
		gtk_separator_new(
			GTK_ORIENTATION_VERTICAL));
	gtk_header_bar_pack_start(
		GTK_HEADER_BAR(header),
		make_btn("New List",
			G_CALLBACK(on_new_list), g));
	gtk_header_bar_pack_start(
		GTK_HEADER_BAR(header),
		make_btn("Delete List",
			G_CALLBACK(on_del_list), g));
	gtk_header_bar_pack_end(
		GTK_HEADER_BAR(header),
		make_btn("Delete Task",
			G_CALLBACK(on_del_task), g));
	gtk_window_set_titlebar(
		GTK_WINDOW(window), header);

	/* main layout: paned */
	paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_paned_set_position(GTK_PANED(paned), 200);
	gtk_window_set_child(GTK_WINDOW(window), paned);

	/* left: list sidebar */
	left_box = gtk_box_new(
		GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_size_request(left_box, 150, -1);

	scroll_lists = gtk_scrolled_window_new();
	gtk_widget_set_vexpand(scroll_lists, TRUE);

	g->list_box = gtk_list_box_new();
	gtk_list_box_set_selection_mode(
		GTK_LIST_BOX(g->list_box),
		GTK_SELECTION_SINGLE);
	g_signal_connect(g->list_box, "row-selected",
		G_CALLBACK(on_list_selected), g);
	gtk_scrolled_window_set_child(
		GTK_SCROLLED_WINDOW(scroll_lists),
		g->list_box);
	gtk_box_append(GTK_BOX(left_box), scroll_lists);

	gtk_paned_set_start_child(
		GTK_PANED(paned), left_box);
	gtk_paned_set_resize_start_child(
		GTK_PANED(paned), FALSE);
	gtk_paned_set_shrink_start_child(
		GTK_PANED(paned), FALSE);

	/* right: task panel */
	right_box = gtk_box_new(
		GTK_ORIENTATION_VERTICAL, 0);

	/* list name header */
	list_label = gtk_label_new("Select a list");
	gtk_widget_add_css_class(
		list_label, "list-header");
	gtk_label_set_xalign(
		GTK_LABEL(list_label), 0.0);
	gtk_box_append(GTK_BOX(right_box), list_label);
	gtk_box_append(GTK_BOX(right_box),
		gtk_separator_new(
			GTK_ORIENTATION_HORIZONTAL));
	g->list_label = list_label;

	/* task list */
	scroll_tasks = gtk_scrolled_window_new();
	gtk_widget_set_vexpand(scroll_tasks, TRUE);

	g->task_box = gtk_list_box_new();
	gtk_list_box_set_selection_mode(
		GTK_LIST_BOX(g->task_box),
		GTK_SELECTION_SINGLE);
	gtk_scrolled_window_set_child(
		GTK_SCROLLED_WINDOW(scroll_tasks),
		g->task_box);
	gtk_box_append(GTK_BOX(right_box), scroll_tasks);

	/* add task entry */
	gtk_box_append(GTK_BOX(right_box),
		gtk_separator_new(
			GTK_ORIENTATION_HORIZONTAL));

	entry_box = gtk_box_new(
		GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_margin_start(entry_box, 8);
	gtk_widget_set_margin_end(entry_box, 8);
	gtk_widget_set_margin_top(entry_box, 6);
	gtk_widget_set_margin_bottom(entry_box, 6);

	entry = gtk_entry_new();
	gtk_entry_set_placeholder_text(
		GTK_ENTRY(entry),
		"Add task (press Enter)");
	gtk_widget_set_hexpand(entry, TRUE);
	g_signal_connect(entry, "activate",
		G_CALLBACK(on_entry_activate), g);
	gtk_box_append(GTK_BOX(entry_box), entry);
	g->entry = entry;

	gtk_box_append(GTK_BOX(right_box), entry_box);

	gtk_paned_set_end_child(
		GTK_PANED(paned), right_box);
	gtk_paned_set_resize_end_child(
		GTK_PANED(paned), TRUE);
	gtk_paned_set_shrink_end_child(
		GTK_PANED(paned), FALSE);

	/* status bar */
	status = gtk_label_new("");
	gtk_widget_set_halign(status, GTK_ALIGN_START);
	gtk_widget_set_margin_start(status, 6);
	gtk_widget_set_margin_end(status, 6);
	gtk_widget_set_margin_top(status, 3);
	gtk_widget_set_margin_bottom(status, 3);
	g->status = status;
	/* status goes outside paned, at window bottom */

	/* wrap paned + status in a vbox */
	{
		GtkWidget *vbox;

		vbox = gtk_box_new(
			GTK_ORIENTATION_VERTICAL, 0);
		gtk_box_append(GTK_BOX(vbox), paned);
		gtk_box_append(GTK_BOX(vbox),
			gtk_separator_new(
				GTK_ORIENTATION_HORIZONTAL));
		gtk_box_append(GTK_BOX(vbox), status);
		gtk_window_set_child(
			GTK_WINDOW(window), vbox);
	}

	/* load lists */
	refresh_lists(g);

	gtk_window_present(GTK_WINDOW(window));
}

int
gui_run(TaskDB *tdb, int argc, char *argv[])
{
	GtkApplication *app;
	int ret;

	memset(&gui, 0, sizeof(gui));
	gui.tdb = tdb;
	gui.dark = default_dark;
	gui.theme = gui.dark ? &theme_dark : &theme_light;
	gui.cur_list = -1;

	app = gtk_application_new("org.stask.app",
		G_APPLICATION_NON_UNIQUE);
	g_signal_connect(app, "activate",
		G_CALLBACK(activate), NULL);
	ret = g_application_run(
		G_APPLICATION(app), 1, argv);
	g_object_unref(app);
	return ret;
}
