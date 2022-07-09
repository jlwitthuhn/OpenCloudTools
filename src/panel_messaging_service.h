#pragma once

#include <QObject>
#include <QString>
#include <QWidget>

#include "util_enum.h"

class QLineEdit;
class QPushButton;
class QRadioButton;
class QTextEdit;

class MessagingServicePanel : public QWidget
{
	Q_OBJECT
public:
	MessagingServicePanel(QWidget* parent, const QString& api_key);

	void selected_universe_changed();

private:
	void pressed_send();

	DatastoreEntryType get_selected_type() const;

	QString api_key;

	QLineEdit* topic_edit = nullptr;
	QRadioButton* type_radio_json = nullptr;
	QRadioButton* type_radio_string = nullptr;
	QRadioButton* type_radio_number = nullptr;
	QRadioButton* type_radio_bool = nullptr;
	QTextEdit* message_edit = nullptr;
	QPushButton* send_button = nullptr;
};
