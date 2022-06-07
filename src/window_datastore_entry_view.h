#pragma once

#include <optional>

#include <QObject>
#include <QString>
#include <QWidget>

#include "util_enum.h"

class DatastoreEntryWithDetails;
class QLineEdit;
class QPushButton;
class QTextEdit;

class ViewDatastoreEntryWindow : public QWidget
{
	Q_OBJECT
public:
	ViewDatastoreEntryWindow(QWidget* parent, const QString& api_key, const DatastoreEntryWithDetails& details, ViewEditMode view_edit_mode = ViewEditMode::View);

private:
	static std::optional<QString> format_json(const QString& input_json);

	void pressed_save();

	QLineEdit* universe_id_edit = nullptr;
	QLineEdit* datastore_name_edit = nullptr;
	QLineEdit* scope_edit = nullptr;
	QLineEdit* key_name_edit = nullptr;
	QLineEdit* version_edit = nullptr;

	QTextEdit* data_edit = nullptr;
	QTextEdit* userids_edit = nullptr;
	QTextEdit* attributes_edit = nullptr;

	QTextEdit* new_data_edit = nullptr;
	QTextEdit* new_userids_edit = nullptr;
	QTextEdit* new_attributes_edit = nullptr;

	QPushButton* save_button = nullptr;

	DatastoreEntryType data_type;

	QString api_key;
};
