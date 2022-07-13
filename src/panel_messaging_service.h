#pragma once

#include <QObject>
#include <QString>
#include <QWidget>

class QCheckBox;
class QLineEdit;
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

	void pressed_send();

	QString api_key;

	QCheckBox* add_used_topics_check = nullptr;

	QLineEdit* topic_edit = nullptr;
	QTextEdit* message_edit = nullptr;
	QPushButton* send_button = nullptr;
};
