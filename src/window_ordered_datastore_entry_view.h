#pragma once

#include <QObject>
#include <QWidget>

#include "util_enum.h"

class QLineEdit;

class OrderedDatastoreEntryFull;

class ViewOrderedDatastoreEntryWindow : public QWidget
{
	Q_OBJECT
public:
	ViewOrderedDatastoreEntryWindow(QWidget* parent, const QString& api_key, const OrderedDatastoreEntryFull& details, ViewEditMode view_edit_mode = ViewEditMode::View);

private:
	QLineEdit* universe_id_edit = nullptr;
	QLineEdit* datastore_name_edit = nullptr;
	QLineEdit* scope_edit = nullptr;
	QLineEdit* key_name_edit = nullptr;
	QLineEdit* value_edit = nullptr;
};
