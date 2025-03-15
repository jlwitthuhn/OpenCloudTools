#pragma once

#include <QMenuBar>
#include <QObject>

#include <vector>

class QAction;
class QMainWindow;

class MyMainWindowMenuBar : public QMenuBar
{
	Q_OBJECT

public:
	MyMainWindowMenuBar(QMainWindow* parent);

signals:
	void request_show_http_log();
	void request_close();

private:
	void handle_autoclose_changed();
	void handle_qt_theme_changed();

	void pressed_change_api_key();
	void pressed_toggle_autoclose();
	void pressed_toggle_datastore_name_filter();
	void pressed_toggle_less_verbose_bulk();

	std::vector<QAction*> theme_actions;

	QAction* action_toggle_autoclose = nullptr;
	QAction* action_toggle_datastore_name_filter = nullptr;
	QAction* action_toggle_less_verbose_bulk = nullptr;
};
