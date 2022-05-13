#include "diag_operation_in_progress.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "data_request.h"
#include "user_settings.h"
#include "util_wed.h"

OperationInProgressDialog::OperationInProgressDialog(QWidget* parent, DataRequest* request) : QDialog{ parent }, request{ request }
{
	setWindowTitle("Progress");

	QLabel* top_label = new QLabel{ request->get_title_string(), this};

	text_box = new QTextEdit{ this };
	text_box->setReadOnly(true);
	text_box->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	text_box->setText("");

	close_automatically_box = new QCheckBox{ "Close this window automatically", this};
	close_automatically_box->setChecked(UserSettings::get()->get_autoclose_progress_window());
	connect(close_automatically_box, &QCheckBox::stateChanged, this, &OperationInProgressDialog::handle_checkbox_changed);

	close_button = new QPushButton{ "Cancel", this };
	connect(close_button, &QPushButton::clicked, this, &OperationInProgressDialog::close);

	setMinimumWidth(330);
	setMinimumHeight(200);

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->addWidget(top_label);
	layout->addWidget(text_box);
	layout->addWidget(close_automatically_box);
	layout->addWidget(close_button);

	connect(request, &DataRequest::request_complete, this, &OperationInProgressDialog::handle_request_complete);
	connect(request, &DataRequest::status_message, this, &OperationInProgressDialog::handle_status_message);

	resize(330, 240);

	if (std::optional<QString> message = wed())
	{
		text_box->append(*message);
	}

	handle_checkbox_changed();
}

void OperationInProgressDialog::handle_request_complete()
{
	close_button->setText("Close");
	if (close_automatically_box->isChecked())
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
	UserSettings::get()->set_autoclose_progress_window(close_automatically_box->isChecked());
}
