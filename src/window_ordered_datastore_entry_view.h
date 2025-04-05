#pragma once

#include <QObject>
#include <QString>
#include <QWidget>

class QLineEdit;
class QPushButton;

class OrderedDatastoreEntryFull;

class ViewOrderedDatastoreEntryWindow : public QWidget
{
	Q_OBJECT
public:
    enum class EditMode : std::uint8_t
	{
		View,
		Edit,
		Increment,
	};

	ViewOrderedDatastoreEntryWindow(QWidget* parent, const QString& api_key, const OrderedDatastoreEntryFull& details, EditMode edit_mode = EditMode::View);

private:
	static bool validate_contains_long(QLineEdit* line);

	void display(const OrderedDatastoreEntryFull& details);
	void refresh();

	void changed_increment();
	void changed_new_value();

	void pressed_increment();
	void pressed_new_value();

	QLineEdit* universe_id_edit = nullptr;
	QLineEdit* datastore_name_edit = nullptr;
	QLineEdit* scope_edit = nullptr;
	QLineEdit* key_name_edit = nullptr;
	QLineEdit* value_edit = nullptr;

	// Increment mode only
	QLineEdit* increment_edit = nullptr;
	QPushButton* increment_submit = nullptr;

	// Edit mode only
	QLineEdit* new_value_edit = nullptr;
	QPushButton* new_value_submit = nullptr;

	QString api_key;
};
