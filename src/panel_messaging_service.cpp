#include "panel_messaging_service.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "data_request.h"
#include "diag_operation_in_progress.h"
#include "user_settings.h"
#include "util_enum.h"
#include "util_validator.h"

MessagingServicePanel::MessagingServicePanel(QWidget* parent, const QString& api_key) : QWidget{ parent }, api_key{ api_key }
{
	QGroupBox* send_group_box = new QGroupBox{ "Send Message", this };
	{
		topic_edit = new QLineEdit{ send_group_box };

		QWidget* type_panel = new QWidget{ send_group_box };
		{
			type_radio_json = new QRadioButton{ get_enum_string(DatastoreEntryType::Json), type_panel };
			type_radio_json->setChecked(true);
			type_radio_json->setCheckable(true);

			type_radio_string = new QRadioButton{ get_enum_string(DatastoreEntryType::String), type_panel };
			type_radio_string->setChecked(false);
			type_radio_string->setCheckable(true);

			type_radio_number = new QRadioButton{ get_enum_string(DatastoreEntryType::Number), type_panel };
			type_radio_number->setChecked(false);
			type_radio_number->setCheckable(true);

			type_radio_bool = new QRadioButton{ get_enum_string(DatastoreEntryType::Bool), type_panel };
			type_radio_bool->setChecked(false);
			type_radio_bool->setCheckable(true);

			QVBoxLayout* type_layout = new QVBoxLayout{ type_panel };
			type_layout->setContentsMargins(QMargins{ 2, 2, 2, 2 });
			type_layout->setSpacing(0);
			type_layout->addWidget(type_radio_json);
			type_layout->addWidget(type_radio_string);
			type_layout->addWidget(type_radio_number);
			type_layout->addWidget(type_radio_bool);
		}

		message_edit = new QTextEdit{ send_group_box };

		send_button = new QPushButton{ "Send", send_group_box};
		connect(send_button, &QPushButton::clicked, this, &MessagingServicePanel::pressed_send);

		QFormLayout* send_layout = new QFormLayout{ send_group_box };
		send_layout->addRow("Topic", topic_edit);
		send_layout->addRow("Type", type_panel);
		send_layout->addRow("Message", message_edit);
		send_layout->addRow("", send_button);
	}

	QHBoxLayout* layout = new QHBoxLayout{ this };
	layout->addWidget(send_group_box);

	selected_universe_changed();
}

void MessagingServicePanel::selected_universe_changed()
{
	const bool enabled = UserSettings::get()->get_selected_universe().has_value();
	send_button->setEnabled(enabled);
}

void MessagingServicePanel::pressed_send()
{
	if (UserSettings::get()->get_selected_universe())
	{
		const long long universe_id = UserSettings::get()->get_selected_universe()->universe_id();

		const QString topic = topic_edit->text();
		if (topic.size() == 0)
		{
			QMessageBox* msg_box = new QMessageBox{ this };
			msg_box->setWindowTitle("Topic Error");
			msg_box->setText("Topic must be set.");
			msg_box->exec();
			return;
		}
		if (topic.size() > 80)
		{
			QMessageBox* msg_box = new QMessageBox{ this };
			msg_box->setWindowTitle("Topic Error");
			msg_box->setText("Topic can't be more than 80 characters.");
			msg_box->exec();
			return;
		}

		const QString unencoded_message = message_edit->toPlainText();
		if (unencoded_message.size() == 0)
		{
			QMessageBox* msg_box = new QMessageBox{ this };
			msg_box->setWindowTitle("Message Error");
			msg_box->setText("Message must be set.");
			msg_box->exec();
			return;
		}

		const DatastoreEntryType data_type = get_selected_type();
		{
			bool data_valid = false;
			switch (data_type)
			{
			case DatastoreEntryType::Error:
				data_valid = false;
				break;
			case DatastoreEntryType::Bool:
				data_valid = DataValidator::is_bool(unencoded_message);
				break;
			case DatastoreEntryType::Number:
				data_valid = DataValidator::is_number(unencoded_message);
				break;
			case DatastoreEntryType::String:
				data_valid = true;
				break;
			case DatastoreEntryType::Json:
				data_valid = DataValidator::is_json(unencoded_message);
				break;
			}
			if (data_valid == false)
			{
				QMessageBox* msg_box = new QMessageBox{ this };
				msg_box->setWindowTitle("Message Error");
				msg_box->setText(QString{ "Message is not a valid " } + get_enum_string(data_type) + ".");
				msg_box->exec();
				return;
			}
		}

		PostMessagingServiceMessageRequest req{ this, api_key, universe_id, topic, data_type, unencoded_message };
		OperationInProgressDialog diag{ this, &req };
		diag.exec();
	}
}

DatastoreEntryType MessagingServicePanel::get_selected_type() const
{
	if (type_radio_json->isChecked())
	{
		return DatastoreEntryType::Json;
	}
	else if (type_radio_string->isChecked())
	{
		return DatastoreEntryType::String;
	}
	else if (type_radio_number->isChecked())
	{
		return DatastoreEntryType::Number;
	}
	else if (type_radio_bool->isChecked())
	{
		return DatastoreEntryType::Bool;
	}
	else
	{
		return DatastoreEntryType::Error;
	}
}
