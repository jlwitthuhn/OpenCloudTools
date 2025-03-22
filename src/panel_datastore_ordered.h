#pragma once

#include <memory>

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QWidget>

#include "window_ordered_datastore_entry_view.h"

class QCheckBox;
class QLineEdit;
class QListWidget;
class QModelIndex;
class QPushButton;
class QTreeView;

class OrderedDatastoreEntryQTableModel;
class UniverseProfile;

class OrderedDatastorePanel : public QWidget
{
	Q_OBJECT
public:
	OrderedDatastorePanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe);

private:
	void gui_refresh();

	void set_table_model(OrderedDatastoreEntryQTableModel* entry_model);

	QModelIndex get_selected_single_index() const;

	void view_entry(const QModelIndex& index, ViewOrderedDatastoreEntryWindow::EditMode edit_mode);

	void handle_datastore_entry_double_clicked(const QModelIndex& index);
	void handle_search_text_changed();
	void handle_selected_datastore_changed();
	void handle_selected_datastore_entry_changed();
	void handle_show_datastore_filter_changed();

	void handle_add_entry_text_changed();

	void handle_recent_datastores_changed();
	void handle_save_recent_datastores_toggled();

	void refresh_datastore_list();

	void pressed_find(bool ascending);
	void pressed_find_ascending();
	void pressed_find_descending();
	void pressed_remove_datastore();
	void pressed_submit_new_entry();
	void pressed_view_entry();

	void pressed_entry_delete();
	void pressed_entry_edit();
	void pressed_entry_increment();

	QString api_key;
	std::weak_ptr<UniverseProfile> attached_universe;
	QMetaObject::Connection conn_universe_ordered_datastores_changed;

	// Left bar
	QListWidget* list_datastore_index = nullptr;
	QLineEdit* edit_datastore_index_filter = nullptr;
	QCheckBox* check_save_recent_datastores = nullptr;
	QPushButton* button_remove_datastore = nullptr;

	// Search panel
	QLineEdit* edit_search_datastore_name = nullptr;
	QLineEdit* edit_search_datastore_scope = nullptr;

	QPushButton* button_search_find_ascending = nullptr;
	QPushButton* button_search_find_descending = nullptr;
	QLineEdit* edit_search_find_limit = nullptr;

	QTreeView* tree_view_main = nullptr;

	QPushButton* button_entry_view = nullptr;

	QPushButton* button_entry_increment = nullptr;
	QPushButton* button_entry_edit = nullptr;
	QPushButton* button_entry_delete = nullptr;

	// Add panel
	QLineEdit* edit_add_entry_datastore_name = nullptr;
	QLineEdit* edit_add_entry_scope = nullptr;
	QLineEdit* edit_add_entry_key_name = nullptr;
	QLineEdit* edit_add_entry_value = nullptr;

	QPushButton* button_add_entry_submit = nullptr;
};
