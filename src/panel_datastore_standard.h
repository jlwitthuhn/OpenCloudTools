#pragma once

#include <memory>
#include <vector>

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QWidget>

class QCheckBox;
class QLineEdit;
class QListWidget;
class QModelIndex;
class QPoint;
class QPushButton;
class QTreeView;

class StandardDatastoreEntryName;
class StandardDatastoreEntryQTableModel;

class UniverseProfile;

class StandardDatastorePanel : public QWidget
{
	Q_OBJECT

public:
	StandardDatastorePanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe);

private:
	void gui_refresh();

	void set_table_model(StandardDatastoreEntryQTableModel* entry_model);

	std::vector<StandardDatastoreEntryName> get_selected_entries() const;
	QModelIndex get_selected_single_index() const;

	void view_entry(const QModelIndex& index);
	void view_versions(const QModelIndex& index);
	void edit_entry(const QModelIndex& index);
	void delete_entry(const QModelIndex& index);
	void delete_entry_list(const std::vector<StandardDatastoreEntryName>& entry_list);

	void handle_datastore_entry_double_clicked(const QModelIndex& index);
	void handle_search_text_changed();
	void handle_selected_datastore_changed();
	void handle_selected_datastore_entry_changed();
	void handle_show_datastore_filter_changed();
	void handle_show_hidden_datastores_toggled();

	void pressed_right_click_datastore_list(const QPoint& pos);
	void pressed_right_click_entry_list(const QPoint& pos);

	void pressed_delete_entry();
	void pressed_edit_entry();
	void pressed_fetch_datastores();
	void pressed_find_all();
	void pressed_find_prefix();
	void pressed_view_entry();
	void pressed_view_versions();

	void refresh_datastore_list();

	QString api_key;
	std::weak_ptr<UniverseProfile> attached_universe;
	QMetaObject::Connection conn_universe_hidden_datastores_changed;

	// Index bar
	QListWidget* list_datastore_index = nullptr;
	QLineEdit* edit_datastore_index_filter = nullptr;
	QCheckBox* check_datastore_index_show_hidden = nullptr;
	QPushButton* button_datastore_index_fetch = nullptr;

	// Search panel
	QLineEdit* edit_search_datastore_name = nullptr;
	QLineEdit* edit_search_datastore_scope = nullptr;
	QLineEdit* edit_search_datastore_key_prefix = nullptr;

	QPushButton* button_search_find_all = nullptr;
	QPushButton* button_search_find_prefix = nullptr;
	QLineEdit* edit_search_find_limit = nullptr;

	QTreeView* tree_view_main = nullptr;

	QPushButton* button_entry_view = nullptr;
	QPushButton* button_entry_view_version = nullptr;

	QPushButton* button_entry_edit = nullptr;
	QPushButton* button_entry_delete = nullptr;
};
