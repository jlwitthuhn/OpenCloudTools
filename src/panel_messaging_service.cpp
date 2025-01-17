#include "panel_messaging_service.h"

#include <memory>
#include <set>

#include <Qt>
#include <QtGlobal>
#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMargins>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QSizePolicy>
#include <QTextEdit>
#include <QVBoxLayout>

#include "assert.h"
#include "data_request.h"
#include "diag_operation_in_progress.h"
#include "profile.h"

MessagingServicePanel::MessagingServicePanel(QWidget* parent, const QString& api_key) : QWidget{ parent }, api_key{ api_key }
{
	QGroupBox* topic_history_group_box = new QGroupBox{ "Topic History" };
	{
		QSizePolicy topic_history_size_policy{ QSizePolicy::Preferred, QSizePolicy::Preferred };
		topic_history_size_policy.setHorizontalStretch(3);
		topic_history_group_box->setSizePolicy(topic_history_size_policy);

		topic_history_list = new QListWidget{ topic_history_group_box };
		connect(topic_history_list, &QListWidget::itemSelectionChanged, this, &MessagingServicePanel::handle_selected_topic_changed);

		add_used_topics_check = new QCheckBox{ "Add used topics", topic_history_group_box };
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
		connect(add_used_topics_check, &QCheckBox::checkStateChanged, this, &MessagingServicePanel::handle_add_used_topics_toggled);
#else
		connect(add_used_topics_check, &QCheckBox::stateChanged, this, &MessagingServicePanel::handle_add_used_topics_toggled);
#endif

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
		message_edit->setAcceptRichText(false);

		send_button = new QPushButton{ "Send", send_group_box};
		send_button->setSizePolicy(QSizePolicy{ QSizePolicy::Expanding, QSizePolicy::Preferred });
		connect(send_button, &QPushButton::clicked, this, &MessagingServicePanel::pressed_send);

		QFormLayout* send_layout = new QFormLayout{ send_group_box };
		send_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		send_layout->addRow("Topic", topic_edit);
		send_layout->addRow("Message", message_edit);
		send_layout->addRow("", send_button);
	}

	QSplitter* splitter = new QSplitter{ this };
	splitter->addWidget(topic_history_group_box);
	splitter->addWidget(send_group_box);
	splitter->setSizes({ 200, 500 });

	QHBoxLayout* layout = new QHBoxLayout{ this };
	layout->addWidget(splitter);

	change_universe(nullptr);
}

void MessagingServicePanel::change_universe(const std::shared_ptr<UniverseProfile>& universe)
{
	attached_universe = universe;

	QObject::disconnect(conn_universe_recent_topic_list_changed);
	if (universe)
	{
		conn_universe_recent_topic_list_changed = connect(universe.get(), &UniverseProfile::recent_topic_list_changed, this, &MessagingServicePanel::handle_recent_topic_list_changed);
	}

	if (universe)
	{
		add_used_topics_check->setChecked(universe->get_save_recent_message_topics());
	}
	else
	{
		add_used_topics_check->setChecked(false);
	}
	handle_recent_topic_list_changed();
	gui_refresh();
}

void MessagingServicePanel::gui_refresh()
{
	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		setEnabled(false);
		return;
	}

	setEnabled(true);
}

void MessagingServicePanel::handle_add_used_topics_toggled()
{
	if (const std::shared_ptr<UniverseProfile> universe = attached_universe.lock())
	{
		universe->set_save_recent_message_topics(add_used_topics_check->isChecked());
	}
}

void MessagingServicePanel::handle_recent_topic_list_changed()
{
	topic_history_list->clear();
	if (const std::shared_ptr<const UniverseProfile> universe = attached_universe.lock())
	{
		for (const QString& this_topic : universe->get_recent_topic_set())
		{
			topic_history_list->addItem(this_topic);
		}
	}
}

void MessagingServicePanel::handle_selected_topic_changed()
{
	QList<QListWidgetItem*> selected = topic_history_list->selectedItems();
	if (selected.size() == 1)
	{
		topic_edit->setText(selected.front()->text());
	}
}

void MessagingServicePanel::pressed_add_topic()
{
	if (const std::shared_ptr<UniverseProfile> universe = attached_universe.lock())
	{
		MessagingServiceAddTopicWindow* const add_window = new MessagingServiceAddTopicWindow{ this, universe };
		add_window->show();
	}
}

void MessagingServicePanel::pressed_remove_topic()
{
	if (const std::shared_ptr<UniverseProfile> universe = attached_universe.lock())
	{
		QList<QListWidgetItem*> selected = topic_history_list->selectedItems();
		if (selected.size() == 1)
		{
			universe->remove_recent_topic(selected.front()->text());
		}
	}
}

void MessagingServicePanel::pressed_send()
{
	if (const std::shared_ptr<UniverseProfile> this_universe = attached_universe.lock())
	{
		const long long universe_id = this_universe->get_universe_id();

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

		const auto req = std::make_shared<MessagingServicePostMessageRequest>(api_key, universe_id, topic, unencoded_message);
		OperationInProgressDialog diag{ this, req };
		diag.exec();
	}
}

MessagingServiceAddTopicWindow::MessagingServiceAddTopicWindow(QWidget* const parent, const std::shared_ptr<UniverseProfile>& universe) :
	QWidget{ parent, Qt::Window },
	attached_universe{ universe }
{
	setWindowTitle("Add Topic");
	setAttribute(Qt::WA_DeleteOnClose);

	OCTASSERT(parent != nullptr);
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
	if (const std::shared_ptr<UniverseProfile> universe = attached_universe.lock())
	{
		universe->add_recent_topic(name_edit->text());
	}
	close();
}
