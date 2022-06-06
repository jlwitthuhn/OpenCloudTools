#pragma once

#include <optional>

#include <QWidget>

class QLineEdit;
class QListWidget;
class QPushButton;
class QTreeView;

class ExploreDatastorePanel : public QWidget
{
	Q_OBJECT
public:
	ExploreDatastorePanel(QWidget* parent, const QString& api_key, const std::optional<long long> selected_universe_id);

	void selected_universe_changed(std::optional<long long> new_universe);

private:
	void handle_datastore_entry_selection_changed();
	void search_text_changed();
	void selected_datastore_changed();

	void pressed_delete_entry();
	void pressed_download();
	void pressed_edit_entry();
	void pressed_fetch_datastores();
	void pressed_find_all();
	void pressed_find_prefix();
	void pressed_view_entry();
	void pressed_view_versions();

	QString api_key;
	std::optional<long long> selected_universe_id;

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
