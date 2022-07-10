#include "panel_messaging_service.h"

#include <memory>
#include <optional>

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMargins>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "api_key.h"
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

		message_edit = new QTextEdit{ send_group_box };

		send_button = new QPushButton{ "Send", send_group_box};
		connect(send_button, &QPushButton::clicked, this, &MessagingServicePanel::pressed_send);

		QFormLayout* send_layout = new QFormLayout{ send_group_box };
		send_layout->addRow("Topic", topic_edit);
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

		PostMessagingServiceMessageRequest req{ this, api_key, universe_id, topic, unencoded_message };
		OperationInProgressDialog diag{ this, &req };
		diag.exec();
	}
}
