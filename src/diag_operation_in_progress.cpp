#include "diag_operation_in_progress.h"

#include <algorithm>
#include <memory>
#include <optional>

#include <Qt>
#include <QCheckBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

#include "data_request.h"
#include "profile.h"
#include "util_wed.h"
#include "widget_text_log.h"

class QWidget;

OperationInProgressDialog::OperationInProgressDialog(QWidget* const parent, const std::shared_ptr<DataRequest>& request) : QDialog{ parent }
{
	request_list.push_back(request);
	constructor_common();
}

OperationInProgressDialog::OperationInProgressDialog(QWidget* const parent, const std::vector<std::shared_ptr<DataRequest>>& request_list) : QDialog{ parent }, respect_close_automatically{ false }, request_list{ request_list }
{
	constructor_common();
}

int OperationInProgressDialog::exec()
{
	send_next_request();
	return QDialog::exec();
}

void OperationInProgressDialog::constructor_common()
{
	// We are going to pop from the back, so reverse the vector we get
	std::reverse(request_list.begin(), request_list.end());

	setWindowTitle("Progress");

	top_label = new QLabel{ "Preparing...", this };

	progress_bar = new QProgressBar{ this };
	progress_bar->setMaximum(0);
	progress_bar->setTextVisible(false);
	progress_bar->setValue(0);

	text_log = new TextLogWidget{ this };

	close_automatically_box = new QCheckBox{ "Close this window automatically", this };
	close_automatically_box->setChecked(UserProfile::get()->get_autoclose_progress_window());
	connect(close_automatically_box, &QCheckBox::stateChanged, this, &OperationInProgressDialog::handle_checkbox_changed);
	if (respect_close_automatically == false)
	{
		close_automatically_box->setHidden(true);
	}

	retry_button = new QPushButton{ "Retry", this };
	retry_button->setEnabled(false);
	connect(retry_button, &QPushButton::clicked, this, &OperationInProgressDialog::handle_clicked_retry);

	close_button = new QPushButton{ "Cancel", this };
	connect(close_button, &QPushButton::clicked, this, &OperationInProgressDialog::close);

	setMinimumWidth(330);
	setMinimumHeight(240);

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->addWidget(top_label);
	layout->addWidget(progress_bar);
	layout->addWidget(text_log);
	layout->addWidget(close_automatically_box);
	layout->addWidget(retry_button);
	layout->addWidget(close_button);

	resize(330, 280);

	if (std::optional<QString> message = wed())
	{
		text_log->append(*message);
	}

	handle_checkbox_changed();
}

std::pair<int, int> OperationInProgressDialog::get_progress() const
{
	const size_t total = request_list.size() + requests_complete + (pending_request ? 1 : 0);
	if (total > 1)
	{
		return std::make_pair(static_cast<int>(requests_complete), static_cast<int>(total));
	}
	else
	{
		return std::make_pair<int, int>(0, 0);
	}
}

void OperationInProgressDialog::send_next_request()
{
	const std::pair<int, int> progress = get_progress();
	progress_bar->setValue(progress.first);
	progress_bar->setMaximum(progress.second);
	if (request_list.size() > 0)
	{
		pending_request = request_list.back();
		request_list.pop_back();

		pending_request->set_http_429_count(http_429_count);
		top_label->setText(pending_request->get_title_string());

		connect(pending_request.get(), &DataRequest::received_http_429, this, &OperationInProgressDialog::handle_received_http_429);
		connect(pending_request.get(), &DataRequest::request_success, this, &OperationInProgressDialog::handle_request_complete);
		connect(pending_request.get(), &DataRequest::status_error, this, &OperationInProgressDialog::handle_status_error);
		connect(pending_request.get(), &DataRequest::status_info, this, &OperationInProgressDialog::handle_status_info);

		pending_request->send_request();
	}
	else
	{
		handle_all_requests_complete();
	}
}

void OperationInProgressDialog::handle_clicked_retry()
{
	if (pending_request && pending_request->request_status() == DataRequestStatus::Error)
	{
		retry_button->setEnabled(false);
		pending_request->force_retry();
	}
	else
	{
		handle_status_info("This error is not retryable, please report this as a bug in the dev forum thread");
	}
}

void OperationInProgressDialog::handle_request_complete()
{
	if (pending_request)
	{
		pending_request.reset();
		requests_complete++;

		send_next_request();
	}
}

void OperationInProgressDialog::handle_all_requests_complete()
{
	progress_bar->setMaximum(1);
	progress_bar->setValue(1);
	close_button->setText("Close");
	if (respect_close_automatically && close_automatically_box->isChecked())
	{
		close();
	}
}

void OperationInProgressDialog::handle_status_error(const QString message)
{
	handle_status_info(message);
	retry_button->setEnabled(true);
}

void OperationInProgressDialog::handle_status_info(const QString message)
{
	text_log->append(message);
}

void OperationInProgressDialog::handle_checkbox_changed()
{
	UserProfile::get()->set_autoclose_progress_window(close_automatically_box->isChecked());
}

void OperationInProgressDialog::handle_received_http_429()
{
	http_429_count++;
}
