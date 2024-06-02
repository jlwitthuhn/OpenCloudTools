#pragma once

#include <QObject>
#include <QString>

#include "panel_datastore_base.h"
#include "window_ordered_datastore_entry_view.h"

class QCheckBox;
class QLineEdit;
class QPushButton;
class QWidget;

class OrderedDatastoreEntryQTableModel;

class OrderedDatastorePanel : public BaseDatastorePanel
{
	Q_OBJECT
public:
	OrderedDatastorePanel(QWidget* parent, const QString& api_key);

	virtual void selected_universe_changed() override;

private:
	void set_datastore_entry_model(OrderedDatastoreEntryQTableModel* entry_model);

	void view_entry(const QModelIndex& index, ViewOrderedDatastoreEntryWindow::EditMode edit_mode);

	virtual void handle_datastore_entry_double_clicked(const QModelIndex& index) override;
	virtual void handle_search_text_changed() override;
	virtual void handle_selected_datastore_changed() override;
	virtual void handle_selected_datastore_entry_changed() override;

	void handle_add_entry_text_changed();

	void handle_recent_datastores_changed();
	void handle_save_recent_datastores_toggled();

	virtual void clear_model() override;
	virtual void refresh_datastore_list() override;

	void pressed_find(bool ascending);
	void pressed_find_ascending();
	void pressed_find_descending();
	void pressed_remove_datastore();
	void pressed_submit_new_entry();
	void pressed_view_entry();

	void pressed_edit();
	void pressed_increment();

	// Left bar
	QCheckBox* save_recent_datastores_check = nullptr;
	QPushButton* remove_datastore_button = nullptr;

	// Search panel
	QPushButton* find_ascending_button = nullptr;
	QPushButton* find_descending_button = nullptr;

	QPushButton* view_entry_button = nullptr;

	QPushButton* increment_entry_button = nullptr;
	QPushButton* edit_entry_button = nullptr;

	// Add panel
	QLineEdit* add_entry_datastore_name_edit = nullptr;
	QLineEdit* add_entry_scope_edit = nullptr;
	QLineEdit* add_entry_key_name_edit = nullptr;
	QLineEdit* add_entry_value_edit = nullptr;

	QPushButton* add_entry_submit_button = nullptr;
};
