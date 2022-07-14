#include "panel_messaging_service.h"

#include <memory>
#include <optional>

#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMargins>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

#include "data_request.h"
#include "diag_operation_in_progress.h"
#include "profile.h"
#include "util_enum.h"
#include "util_validator.h"

MessagingServicePanel::MessagingServicePanel(QWidget* parent, const QString& api_key) : QWidget{ parent }, api_key{ api_key }
{
	connect(UserProfile::get().get(), &UserProfile::recent_topic_list_changed, this, &MessagingServicePanel::handle_recent_topic_list_changed);

	QGroupBox* topic_history_group_box = new QGroupBox{ "Topic History" };
	{
		QSizePolicy topic_history_size_policy{ QSizePolicy::Preferred, QSizePolicy::Preferred };
		topic_history_size_policy.setHorizontalStretch(3);
		topic_history_group_box->setSizePolicy(topic_history_size_policy);

		topic_history_list = new QListWidget{ topic_history_group_box };

		add_used_topics_check = new QCheckBox{ "Add used topics", topic_history_group_box };
		connect(add_used_topics_check, &QCheckBox::stateChanged, this, &MessagingServicePanel::handle_add_used_topics_toggled);

		add_topic_button = new QPushButton{ "Add...", topic_history_group_box };
		connect(add_topic_button, &QPushButton::clicked, this, &MessagingServicePanel::pressed_add_topic);

		remove_topic_button = new QPushButton{ "Remove", topic_history_group_box };
		connect(remove_topic_button, &QPushButton::clicked, this, &MessagingServicePanel::pressed_remove_topic);

		QVBoxLayout* topic_history_layout = new QVBoxLayout{ topic_history_group_box };
		topic_history_layout->addWidget(topic_history_list);
		topic_history_layout->addWidget(add_used_topics_check);
		topic_history_layout->addWidget(add_topic_button);
		topic_history_layout->addWidget(remove_topic_button);
	}

	QGroupBox* send_group_box = new QGroupBox{ "Send Message", this };
	{
		QSizePolicy send_size_policy{ QSizePolicy::Preferred, QSizePolicy::Preferred };
		send_size_policy.setHorizontalStretch(7);
		send_group_box->setSizePolicy(send_size_policy);

		topic_edit = new QLineEdit{ send_group_box };

		message_edit = new QTextEdit{ send_group_box };

		send_button = new QPushButton{ "Send", send_group_box};
		connect(send_button, &QPushButton::clicked, this, &MessagingServicePanel::pressed_send);

		QFormLayout* send_layout = new QFormLayout{ send_group_box };
		send_layout->addRow("Topic", topic_edit);
		send_layout->addRow("Message", message_edit);
		send_layout->addRow("", send_button);
	}

	QSplitter* splitter = new QSplitter{ this };
	splitter->addWidget(topic_history_group_box);
	splitter->addWidget(send_group_box);

	QHBoxLayout* layout = new QHBoxLayout{ this };
	layout->addWidget(splitter);

	selected_universe_changed();
}

void MessagingServicePanel::selected_universe_changed()
{
	const UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
	const bool enabled = selected_universe != nullptr;
	if (selected_universe)
	{
		add_used_topics_check->setChecked(selected_universe->get_save_recent_message_topics());
	}
	else
	{
		add_used_topics_check->setChecked(false);
	}
	add_used_topics_check->setEnabled(enabled);
	add_topic_button->setEnabled(enabled);
	remove_topic_button->setEnabled(enabled);
	send_button->setEnabled(enabled);
	handle_recent_topic_list_changed();
}

void MessagingServicePanel::handle_add_used_topics_toggled()
{
	if (UniverseProfile* const selected_universe = UserProfile::get_selected_universe())
	{
		selected_universe->set_save_recent_message_topics(add_used_topics_check->isChecked());
	}
}

void MessagingServicePanel::handle_recent_topic_list_changed()
{
	if (const UniverseProfile* const selected_universe = UserProfile::get_selected_universe())
	{
		topic_history_list->clear();
		for (const QString& this_topic : selected_universe->get_recent_topic_set())
		{
			topic_history_list->addItem(this_topic);
		}
	}
	else
	{
		topic_history_list->clear();
	}
}

void MessagingServicePanel::pressed_add_topic()
{
	if (UniverseProfile* const selected_universe = UserProfile::get_selected_universe())
	{
		MessagingServiceAddTopicWindow* add_window = new MessagingServiceAddTopicWindow{ this };
		add_window->show();
	}
}

void MessagingServicePanel::pressed_remove_topic()
{
	if (UniverseProfile* const selected_universe = UserProfile::get_selected_universe())
	{
		QList<QListWidgetItem*> selected = topic_history_list->selectedItems();
		if (selected.size() == 1)
		{
			selected_universe->remove_recent_topic(selected.front()->text());
		}
	}
}

void MessagingServicePanel::pressed_send()
{
	if (UniverseProfile* this_universe = UserProfile::get_selected_universe())
	{
		const long long universe_id = UserProfile::get_selected_universe()->get_universe_id();

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

		if (add_used_topics_check->isChecked())
		{
			this_universe->add_recent_topic(topic);
		}

		PostMessagingServiceMessageRequest req{ this, api_key, universe_id, topic, unencoded_message };
		OperationInProgressDialog diag{ this, &req };
		diag.exec();
	}
}

MessagingServiceAddTopicWindow::MessagingServiceAddTopicWindow(QWidget* parent) : QWidget{ parent, Qt::Window }
{
	setAttribute(Qt::WA_DeleteOnClose);

	setWindowTitle("Add Topic");
	setWindowModality(Qt::WindowModality::ApplicationModal);

	QWidget* info_panel = new QWidget{ this };
	{
		name_edit = new QLineEdit{ info_panel };
		connect(name_edit, &QLineEdit::textChanged, this, &MessagingServiceAddTopicWindow::handle_text_changed);

		QFormLayout* info_layout = new QFormLayout{ info_panel };
		info_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		info_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		info_layout->addRow("Topic name", name_edit);
	}

	QWidget* button_panel = new QWidget{ this };
	button_panel->setMinimumWidth(280);
	{
		add_button = new QPushButton{ "Add", button_panel };
		connect(add_button, &QPushButton::clicked, this, &MessagingServiceAddTopicWindow::pressed_add);

		QPushButton* cancel_button = new QPushButton{ "Cancel", button_panel };
		connect(add_button, &QPushButton::clicked, this, &MessagingServiceAddTopicWindow::close);

		QHBoxLayout* button_layout = new QHBoxLayout{ button_panel };
		button_layout->addWidget(add_button);
		button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		button_layout->addWidget(cancel_button);
		connect(cancel_button, &QPushButton::clicked, this, &QWidget::close);
	}

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->addWidget(info_panel);
	layout->addWidget(button_panel);

	handle_text_changed();
}

void MessagingServiceAddTopicWindow::handle_text_changed()
{
	add_button->setEnabled(name_edit->text().size() > 0);
}

void MessagingServiceAddTopicWindow::pressed_add()
{
	UserProfile::get_selected_universe()->add_recent_topic(name_edit->text());;
	close();
}
