#pragma once

#include <QObject>
#include <QString>
#include <QWidget>

class QCheckBox;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTextEdit;

class MessagingServicePanel : public QWidget
{
	Q_OBJECT
public:
	MessagingServicePanel(QWidget* parent, const QString& api_key);

	void selected_universe_changed();

private:
	void handle_add_used_topics_toggled();
	void handle_recent_topic_list_changed();

	void pressed_send();

	QString api_key;

	QListWidget* topic_history_list;
	QCheckBox* add_used_topics_check = nullptr;

	QLineEdit* topic_edit = nullptr;
	QTextEdit* message_edit = nullptr;
	QPushButton* send_button = nullptr;
};
