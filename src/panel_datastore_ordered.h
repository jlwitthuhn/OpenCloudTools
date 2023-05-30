#pragma once

#include <QObject>

#include "panel_datastore_base.h"

class QPushButton;

class OrderedDatastorePanel : public BaseDatastorePanel
{
	Q_OBJECT
public:
	OrderedDatastorePanel(QWidget* parent, const QString& api_key);

private:
	virtual void handle_search_text_changed() override;

	virtual void clear_model() override;
	virtual void refresh_datastore_list() override;

	void pressed_find_ascending();
	void pressed_find_descending();

	QPushButton* find_ascending_button = nullptr;
	QPushButton* find_descending_button = nullptr;
};
