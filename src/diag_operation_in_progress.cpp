#include "diag_operation_in_progress.h"

#include <algorithm>
#include <memory>
#include <optional>

#include <Qt>
#include <QCheckBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "data_request.h"
#include "profile.h"
#include "util_wed.h"

class QWidget;

OperationInProgressDialog::OperationInProgressDialog(QWidget* parent, DataRequest* const request) : QDialog{ parent }
{
	request_list.push_back(request);
	constructor_common();
}

OperationInProgressDialog::OperationInProgressDialog(QWidget* parent, const std::vector<DataRequest*>& request_list) : QDialog{ parent }, respect_close_automatically{ false }, request_list{ request_list }
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

	text_box = new QTextEdit{ this };
	text_box->setReadOnly(true);
	text_box->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	text_box->setText("");

	close_automatically_box = new QCheckBox{ "Close this window automatically", this };
	close_automatically_box->setChecked(UserProfile::get()->get_autoclose_progress_window());
	connect(close_automatically_box, &QCheckBox::stateChanged, this, &OperationInProgressDialog::handle_checkbox_changed);
	if (respect_close_automatically == false)
	{
		close_automatically_box->setHidden(true);
	}

	close_button = new QPushButton{ "Cancel", this };
	connect(close_button, &QPushButton::clicked, this, &OperationInProgressDialog::close);

	setMinimumWidth(330);
	setMinimumHeight(240);

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->addWidget(top_label);
	layout->addWidget(progress_bar);
	layout->addWidget(text_box);
	layout->addWidget(close_automatically_box);
	layout->addWidget(close_button);


	resize(330, 280);

	if (std::optional<QString> message = wed())
	{
		text_box->append(*message);
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

		connect(pending_request, &DataRequest::received_http_429, this, &OperationInProgressDialog::handle_received_http_429);
		connect(pending_request, &DataRequest::request_complete, this, &OperationInProgressDialog::handle_request_complete);
		connect(pending_request, &DataRequest::status_error, this, &OperationInProgressDialog::handle_status_message);
		connect(pending_request, &DataRequest::status_info, this, &OperationInProgressDialog::handle_status_message);

		pending_request->send_request();
	}
	else
	{
		handle_all_requests_complete();
	}
}

void OperationInProgressDialog::handle_request_complete()
{
	if (pending_request)
	{
		pending_request = nullptr;
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

void OperationInProgressDialog::handle_status_message(const QString message)
{
	text_box->append(message);
}

void OperationInProgressDialog::handle_checkbox_changed()
{
	UserProfile::get()->set_autoclose_progress_window(close_automatically_box->isChecked());
}

void OperationInProgressDialog::handle_received_http_429()
{
	http_429_count++;
}
