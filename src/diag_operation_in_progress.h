#pragma once

#include <utility>
#include <vector>

#include <QDialog>
#include <QObject>
#include <QString>

class QCheckBox;
class QProgressBar;
class QPushButton;
class QTextEdit;
class QWidget;

class DataRequest;

class OperationInProgressDialog : public QDialog
{
	Q_OBJECT
public:
	OperationInProgressDialog(QWidget* parent, DataRequest* request);
	OperationInProgressDialog(QWidget* parent, const std::vector<DataRequest*>& request_list);

	virtual int exec() override;

private:
	void constructor_common();

	std::pair<int, int> get_progress() const;

	void send_next_request();

	void handle_request_complete();
	void handle_all_requests_complete();
	void handle_status_message(QString message);
	void handle_checkbox_changed();
	void handle_received_http_429();

	std::vector<DataRequest*> request_list;
	DataRequest* pending_request = nullptr;
	size_t requests_complete = 0;

	size_t http_429_count = 0;

	QProgressBar* progress_bar = nullptr;
	QTextEdit* text_box = nullptr;
	QCheckBox* close_automatically_box = nullptr;
	QPushButton* close_button = nullptr;
};
