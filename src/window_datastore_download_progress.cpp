#include "window_datastore_download_progress.h"

#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

DownloadDatastoreProgressWindow::DownloadDatastoreProgressWindow(QWidget* parent, const QString& api_key, const long long universe_id, std::vector<QString> datastore_names, std::unique_ptr<SqliteDatastoreWriter> writer) :
	QWidget{ parent, Qt::Window },
	api_key{ api_key },
	universe_id{ universe_id },
	progress{ datastore_names.size() },
	writer{ std::move(writer) },
	datastore_names{ datastore_names }
{
	setWindowTitle("Download Progress");

	QGroupBox* bars_box = new QGroupBox{ "Progress", this };
	{
		label_overall = new QLabel{ "", bars_box };
		bar_overall = new QProgressBar{ bars_box };
		bar_overall->setMinimumWidth(360);
		bar_overall->setTextVisible(false);
		bar_overall->setMaximum(DownloadProgress::MAXIMUM);

		label_entry = new QLabel{ "", bars_box };
		bar_entry = new QProgressBar{ bars_box };
		bar_entry->setMinimumWidth(360);
		bar_entry->setTextVisible(false);
		bar_entry->setRange(0, 0);

		QVBoxLayout* const bars_layout = new QVBoxLayout{ bars_box };
		bars_layout->addWidget(label_overall);
		bars_layout->addWidget(bar_overall);
		bars_layout->addWidget(label_entry);
		bars_layout->addWidget(bar_entry);
	}

	text_box = new QTextEdit{ this };
	text_box->setReadOnly(true);
	text_box->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	text_box->setText("");

	close_button = new QPushButton{ "Stop", this };
	connect(close_button, &QPushButton::clicked, this, &DownloadDatastoreProgressWindow::close);

	QVBoxLayout* const layout = new QVBoxLayout{ this };
	layout->addWidget(bars_box);
	layout->addWidget(text_box);
	layout->addWidget(close_button);

	send_list_keys_request();
}

void DownloadDatastoreProgressWindow::update_ui()
{
	if (progress.is_done()) {
		label_overall->setText("Download complete");
		bar_overall->setMaximum(1);
		bar_overall->setValue(1);
		label_entry->setText("Complete");
		bar_entry->setMaximum(1);
		bar_entry->setValue(1);
	}
	else
	{
		QString progress_text = QString{ "Downloading datastore %1/%2..." }.arg(progress.get_current_datastore_index() + 1).arg(datastore_names.size());
		label_overall->setText(progress_text);
		bar_overall->setValue(progress.get_overall_progress());
		std::optional<size_t> local_progress = progress.get_local_progress();
		if (local_progress)
		{
			label_entry->setText("Downloading...");
			bar_entry->setMaximum(DownloadProgress::MAXIMUM);
			bar_entry->setValue(*local_progress);
		}
		else
		{
			label_entry->setText("Enumerating entries...");
			bar_entry->setRange(0, 0);
		}
	}
}

void DownloadDatastoreProgressWindow::send_list_keys_request()
{
	if (progress.is_done())
	{
		close_button->setText("Close");
		handle_status_message("Download complete");
		return;
	}

	const size_t current_index = progress.get_current_datastore_index();
	const QString this_datastore_name = datastore_names[current_index];

	get_entries_request = new GetStandardDatastoreEntriesRequest { this, api_key, universe_id, this_datastore_name, "", "" };
	connect(get_entries_request, &GetStandardDatastoreEntriesRequest::status_message, this, &DownloadDatastoreProgressWindow::handle_status_message);
	connect(get_entries_request, &GetStandardDatastoreEntriesRequest::request_complete, this, &DownloadDatastoreProgressWindow::handle_enumerate_entries_complete);
	get_entries_request->send_request();

	progress.clear_entry_done();
	progress.clear_entry_total();

	update_ui();
}

void DownloadDatastoreProgressWindow::send_next_details_request()
{
	if (pending_entries.size() > 0)
	{
		StandardDatastoreEntry entry = pending_entries.back();
		pending_entries.pop_back();

		get_entry_details_request = new GetStandardDatastoreEntryRequest{ this, api_key, universe_id, entry.get_datastore_name(), entry.get_scope(), entry.get_key() };
		connect(get_entry_details_request, &GetStandardDatastoreEntryRequest::status_message, this, &DownloadDatastoreProgressWindow::handle_status_message);
		connect(get_entry_details_request, &GetStandardDatastoreEntryRequest::request_complete, this, &DownloadDatastoreProgressWindow::handle_get_entry_details_complete);
		get_entry_details_request->send_request();
	}
	else
	{
		progress.advance_datastore_done();
		send_list_keys_request();
	}
}

void DownloadDatastoreProgressWindow::handle_status_message(const QString message)
{
	text_box->append(message);
	update_ui();
}

void DownloadDatastoreProgressWindow::handle_enumerate_entries_complete()
{
	if (get_entries_request)
	{
		pending_entries = get_entries_request->get_datastore_entries();
		progress.clear_entry_done();
		progress.set_entry_total(pending_entries.size());

		get_entries_request->deleteLater();
		get_entries_request = nullptr;

		send_next_details_request();
	}
}

void DownloadDatastoreProgressWindow::handle_get_entry_details_complete()
{
	if (get_entry_details_request)
	{
		std::optional<GetStandardDatastoreEntryDetailsResponse> response = get_entry_details_request->get_response();
		if (response)
		{
			writer->write(response->get_details());
		}
		progress.advance_entry_done();
		get_entry_details_request->deleteLater();
		get_entry_details_request = nullptr;
		send_next_details_request();
	}
}

DownloadDatastoreProgressWindow::DownloadProgress::DownloadProgress(const size_t datastore_total) : datastore_total{ datastore_total }
{

}

bool DownloadDatastoreProgressWindow::DownloadProgress::is_done() const
{
	return datastore_done >= datastore_total;
}

size_t DownloadDatastoreProgressWindow::DownloadProgress::get_current_datastore_index() const
{
	return datastore_done;
}

size_t DownloadDatastoreProgressWindow::DownloadProgress::get_overall_progress() const
{
	if (datastore_total == 0)
	{
		return MAXIMUM;
	}
	else
	{
		double overall = static_cast<double>(datastore_done) / static_cast<double>(datastore_total);
		std::optional<size_t> local = get_local_progress();
		if (local)
		{
			overall += (static_cast<double>(*local) / static_cast<double>(MAXIMUM)) / static_cast<double>(datastore_total);
		}
		return static_cast<size_t>(overall * MAXIMUM);
	}
}

std::optional<size_t> DownloadDatastoreProgressWindow::DownloadProgress::get_local_progress() const
{
	if (entry_total)
	{
		return static_cast<size_t>( (static_cast<double>(entry_done) / static_cast<double>(*entry_total)) * MAXIMUM );
	}
	else
	{
		return std::nullopt;
	}
}

void DownloadDatastoreProgressWindow::DownloadProgress::advance_datastore_done()
{
	datastore_done++;
}

void DownloadDatastoreProgressWindow::DownloadProgress::advance_entry_done()
{
	entry_done++;
}

void DownloadDatastoreProgressWindow::DownloadProgress::clear_entry_done()
{
	entry_done = 0;
}

void DownloadDatastoreProgressWindow::DownloadProgress::set_entry_total(const size_t total)
{
	entry_total = total;
}

void DownloadDatastoreProgressWindow::DownloadProgress::clear_entry_total()
{
	entry_total = std::nullopt;
}
