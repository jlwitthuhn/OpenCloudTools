#pragma once

#include <QObject>
#include <QWidget>

class QLineEdit;

class OrderedDatastoreEntryFull;

class ViewOrderedDatastoreEntryWindow : public QWidget
{
	Q_OBJECT
public:
	enum class EditMode
	{
		View,
		Edit,
		Increment,
	};

	ViewOrderedDatastoreEntryWindow(QWidget* parent, const QString& api_key, const OrderedDatastoreEntryFull& details, EditMode edit_mode = EditMode::View);

private:
	QLineEdit* universe_id_edit = nullptr;
	QLineEdit* datastore_name_edit = nullptr;
	QLineEdit* scope_edit = nullptr;
	QLineEdit* key_name_edit = nullptr;
	QLineEdit* value_edit = nullptr;

	// Increment mode only
	QLineEdit* increment_edit = nullptr;

	// Edit mode only
	QLineEdit* new_value_edit = nullptr;

	QString api_key;
};
