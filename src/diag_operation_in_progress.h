#pragma once

#include <QDialog>
#include <QObject>
#include <QString>

class DataRequest;
class QCheckBox;
class QPushButton;
class QTextEdit;
class QWidget;

class OperationInProgressDialog : public QDialog
{
	Q_OBJECT
public:
	OperationInProgressDialog(QWidget* parent, DataRequest* request);

private:
	void handle_request_complete();
	void handle_status_message(QString message);
	void handle_checkbox_changed();

	DataRequest* request = nullptr;
	QTextEdit* text_box = nullptr;
	QCheckBox* close_automatically_box = nullptr;
	QPushButton* close_button = nullptr;
};
