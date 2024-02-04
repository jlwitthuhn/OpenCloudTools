#pragma once

#include <QObject>
#include <QString>

#include "panel_datastore_base.h"

class QCheckBox;
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

	void view_entry(const QModelIndex& index);

	virtual void handle_datastore_entry_double_clicked(const QModelIndex& index) override;
	virtual void handle_search_text_changed() override;

	void handle_recent_datastores_changed();
	void handle_save_recent_datastores_toggled();

	virtual void clear_model() override;
	virtual void refresh_datastore_list() override;

	void pressed_find(bool ascending);
	void pressed_find_ascending();
	void pressed_find_descending();
	void pressed_remove_datastore();

	QCheckBox* save_recent_datastores_check = nullptr;
	QPushButton* remove_datastore_button = nullptr;

	QPushButton* find_ascending_button = nullptr;
	QPushButton* find_descending_button = nullptr;
};
