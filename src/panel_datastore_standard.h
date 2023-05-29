#pragma once

#include <QObject>

#include "panel_datastore_base.h"

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPushButton;
class QString;
class QTextEdit;
class QWidget;

class StandardDatastoreEntry;
class StandardDatastoreEntryQTableModel;

class StandardDatastorePanel : public BaseDatastorePanel
{
	Q_OBJECT
public:
	StandardDatastorePanel(QWidget* parent, const QString& api_key);

	virtual void selected_universe_changed() override;

private:
	void set_datastore_entry_model(StandardDatastoreEntryQTableModel* entry_model);

	std::vector<StandardDatastoreEntry> get_selected_entries() const;

	void view_entry(const QModelIndex& index);
	void view_versions(const QModelIndex& index);
	void edit_entry(const QModelIndex& index);
	void delete_entry(const QModelIndex& index);
	void delete_entry_list(const std::vector<StandardDatastoreEntry>& entry_list);

	virtual void handle_datastore_entry_double_clicked(const QModelIndex& index) override;
	virtual void handle_search_text_changed() override;
	virtual void handle_selected_datastore_changed() override;
	virtual void handle_selected_datastore_entry_changed() override;

	void handle_add_entry_text_changed();

	virtual void pressed_right_click_datastore_list(const QPoint& pos) override;
	virtual void pressed_right_click_entry_list(const QPoint& pos) override;

	void pressed_delete_entry();
	void pressed_edit_entry();
	void pressed_fetch_datastores();
	void pressed_find_all();
	void pressed_find_prefix();
	void pressed_submit_new_entry();
	void pressed_view_entry();
	void pressed_view_versions();

	virtual void clear_model() override;
	virtual void refresh_datastore_list() override;

	// Left bar
	QCheckBox* select_datastore_show_hidden_check = nullptr;
	QPushButton* select_datastore_fetch_button = nullptr;

	// Search panel
	QLineEdit* search_datastore_key_prefix_edit = nullptr;

	QPushButton* find_all_button = nullptr;
	QPushButton* find_prefix_button = nullptr;

	QPushButton* view_entry_button = nullptr;
	QPushButton* view_versions_button = nullptr;

	QPushButton* edit_entry_button = nullptr;
	QPushButton* delete_entry_button = nullptr;

	// Add panel
	QLineEdit* add_datastore_name_edit = nullptr;
	QLineEdit* add_datastore_scope_edit = nullptr;
	QLineEdit* add_datastore_key_name_edit = nullptr;

	QComboBox* add_entry_type_combo = nullptr;

	QTextEdit* add_entry_data_edit = nullptr;
	QTextEdit* add_entry_userids_edit = nullptr;
	QTextEdit* add_entry_attributes_edit = nullptr;

	QPushButton* add_entry_submit_button = nullptr;
};
