#pragma once

#include <QObject>
#include <QString>
#include <QWidget>

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
	void pressed_send();

	QString api_key;

	QLineEdit* topic_edit = nullptr;
	QTextEdit* message_edit = nullptr;
	QPushButton* send_button = nullptr;
};
