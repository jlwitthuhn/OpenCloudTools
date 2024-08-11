#include "util_alert.h"

#include <QMessageBox>
#include <QString>

void alert_error_blocking(const std::string& title, const std::string& message, QWidget* const parent)
{
	QMessageBox* const message_box = new QMessageBox{ parent };
	message_box->setWindowTitle(QString::fromStdString(title));
	message_box->setIcon(QMessageBox::Critical);
	message_box->setText(QString::fromStdString(message));
	message_box->exec();
}
