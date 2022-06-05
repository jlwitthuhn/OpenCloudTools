#pragma once

#include <QString>
#include <QMainWindow>

class QComboBox;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTreeView;

class MyMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MyMainWindow(QWidget* parent, QString title, QString api_key);

private:
	void selected_universe_changed();
	void selected_datastore_changed();
	void search_text_changed();
	void universe_list_changed();

	void pressed_add_universe();
	void pressed_remove_universe();
	void pressed_change_key();
	void pressed_delete_entry();
	void pressed_download();
	void pressed_edit_entry();
	void pressed_fetch_datastores();
	void pressed_find_all();
	void pressed_find_prefix();
	void pressed_view_entry();
	void pressed_view_versions();
	void pressed_toggle_autoclose();

	void handle_autoclose_changed();
	void handle_datastore_entry_selection_changed();

	QString api_key;

	QAction* action_toggle_autoclose = nullptr;

	QComboBox* select_universe_combo = nullptr;

	QPushButton* del_universe_button = nullptr;

	QListWidget* select_datastore_list = nullptr;
	QPushButton* select_datastore_fetch_button = nullptr;
	QPushButton* select_datastore_download_button = nullptr;

	QLineEdit* datastore_name_edit = nullptr;
	QLineEdit* datastore_scope_edit = nullptr;
	QLineEdit* datastore_key_name_edit = nullptr;

	QPushButton* find_all_button = nullptr;
	QPushButton* find_prefix_button = nullptr;

	QLineEdit* find_limit_edit = nullptr;

	QTreeView* datastore_entry_tree = nullptr;

	QPushButton* view_entry_button = nullptr;
	QPushButton* view_versions_button = nullptr;

	QPushButton* edit_entry_button = nullptr;
	QPushButton* delete_entry_button = nullptr;
};
