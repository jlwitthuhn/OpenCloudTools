#pragma once

#include <QObject>
#include <QString>
#include <QWidget>

class QCheckBox;
class QLineEdit;
class QListWidget;
class QModelIndex;
class QPushButton;
class QTreeView;

class ExploreDatastorePanel : public QWidget
{
	Q_OBJECT
public:
	ExploreDatastorePanel(QWidget* parent, const QString& api_key);

	void selected_universe_changed();

private:
	void view_entry(const QModelIndex& index);

	void handle_datastore_entry_double_clicked(const QModelIndex& index);
	void handle_search_text_changed();
	void handle_selected_datastore_changed();
	void handle_selected_datastore_entry_changed();
	void handle_show_hidden_datastores_toggled();

	void pressed_delete_entry();
	void pressed_edit_entry();
	void pressed_fetch_datastores();
	void pressed_find_all();
	void pressed_find_prefix();
	void pressed_right_click_datastore_list(const QPoint& pos);
	void pressed_view_entry();
	void pressed_view_versions();

	QString api_key;

	QListWidget* select_datastore_list = nullptr;
	QPushButton* select_datastore_fetch_button = nullptr;
	QCheckBox* select_datastore_show_hidden_check = nullptr;

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
