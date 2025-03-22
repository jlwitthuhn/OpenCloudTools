#pragma once

#include <memory>

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QWidget>

class QCheckBox;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTextEdit;

class UniverseProfile;

class MessagingServicePanel : public QWidget
{
	Q_OBJECT
public:
	MessagingServicePanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe);

private:
	void gui_refresh();

	void handle_add_used_topics_toggled();
	void handle_recent_topic_list_changed();
	void handle_selected_topic_changed();

	void pressed_add_topic();
	void pressed_remove_topic();
	void pressed_send();

	QString api_key;
	std::weak_ptr<UniverseProfile> attached_universe;
	QMetaObject::Connection conn_universe_recent_topic_list_changed;

	QListWidget* topic_history_list;
	QCheckBox* add_used_topics_check = nullptr;
	QPushButton* add_topic_button = nullptr;
	QPushButton* remove_topic_button = nullptr;

	QLineEdit* topic_edit = nullptr;
	QTextEdit* message_edit = nullptr;
	QPushButton* send_button = nullptr;
};


class MessagingServiceAddTopicWindow : public QWidget
{
	Q_OBJECT
public:
	MessagingServiceAddTopicWindow(QWidget* parent, const std::shared_ptr<UniverseProfile>& universe);

private:
	void handle_text_changed();

	void pressed_add();

	QLineEdit* name_edit = nullptr;

	QPushButton* add_button = nullptr;

	std::weak_ptr<UniverseProfile> attached_universe;
};
