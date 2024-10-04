#pragma once

#include <QMainWindow>
#include <QObject>

class QLineEdit;
class QTreeWidget;

class MyNewMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MyNewMainWindow();

private:
	void on_active_api_key_changed();

	void pressed_change_key();

	void rebuild_universe_tree();

	QLineEdit* edit_api_key_name = nullptr;

	QTreeWidget* tree_universe = nullptr;
};
