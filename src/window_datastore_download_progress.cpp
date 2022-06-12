#include "window_datastore_download_progress.h"

#include <utility>

#include <Qt>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "data_request.h"

DownloadDatastoreProgressWindow::DownloadDatastoreProgressWindow(QWidget* parent, const QString& api_key, const long long universe_id, const QString& scope, const QString& key_prefix, std::vector<QString> datastore_names, std::unique_ptr<SqliteDatastoreWriter> writer) :
	QWidget{ parent, Qt::Window },
	api_key{ api_key },
	universe_id{ universe_id },
	scope{ scope },
	key_prefix{ key_prefix },
	progress{ datastore_names.size() },
	writer{ std::move(writer) },
	datastore_names{ datastore_names }
{
	setWindowTitle("Download Progress");

	progress_label = new QLabel{ "", this };
	progress_bar = new QProgressBar{ this };
	progress_bar->setMinimumWidth(360);
	progress_bar->setTextVisible(false);
	progress_bar->setMaximum(DownloadProgress::MAXIMUM);

	text_box = new QTextEdit{ this };
	text_box->setReadOnly(true);
	text_box->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	text_box->setText("");

	close_button = new QPushButton{ "Stop", this };
	connect(close_button, &QPushButton::clicked, this, &DownloadDatastoreProgressWindow::close);

	QVBoxLayout* const layout = new QVBoxLayout{ this };
	layout->addWidget(progress_label);
	layout->addWidget(progress_bar);
	layout->addWidget(text_box);
	layout->addWidget(close_button);

	send_next_list_keys_request();
}

void DownloadDatastoreProgressWindow::update_ui()
{
	if (progress.is_done()) {
		progress_label->setText("Download complete");
		progress_bar->setMaximum(1);
		progress_bar->setValue(1);
	}
	else if (progress.is_enumerating())
	{
		progress_label->setText("Enumerating objects...");
		progress_bar->setMaximum(0);
		progress_bar->setValue(0);
	}
	else
	{
		const std::optional<size_t> opt_entry_total = progress.get_entry_total();
		if (opt_entry_total)
		{
			const QString progress_text = QString{ "Downloading entry %1/%2..." }.arg(progress.get_current_entry_index() + 1).arg(*opt_entry_total);
			progress_label->setText(progress_text);
			progress_bar->setValue(static_cast<int>(progress.get_progress()));
			progress_bar->setMaximum(static_cast<int>(DownloadProgress::MAXIMUM));
		}
		else
		{
			progress_label->setText("Download error");
		}
	}
}

void DownloadDatastoreProgressWindow::send_next_list_keys_request()
{
	const size_t current_index = progress.get_current_datastore_index();
	if (current_index < datastore_names.size())
	{
		const QString this_datastore_name = datastore_names[current_index];

		get_entries_request = new GetStandardDatastoreEntriesRequest{ this, api_key, universe_id, this_datastore_name, scope, key_prefix };
		get_entries_request->set_http_429_count(http_429_count);
		connect(get_entries_request, &GetStandardDatastoreEntriesRequest::received_http_429, this, &DownloadDatastoreProgressWindow::handle_received_http_429);
		connect(get_entries_request, &GetStandardDatastoreEntriesRequest::status_message, this, &DownloadDatastoreProgressWindow::handle_status_message);
		connect(get_entries_request, &GetStandardDatastoreEntriesRequest::request_complete, this, &DownloadDatastoreProgressWindow::handle_list_keys_complete);
		get_entries_request->send_request();

		update_ui();
	}
	else
	{
		send_next_details_request();
	}
}

void DownloadDatastoreProgressWindow::send_next_details_request()
{
	if (pending_entries.size() > 0)
	{
		StandardDatastoreEntry entry = pending_entries.back();
		pending_entries.pop_back();

		get_entry_details_request = new GetStandardDatastoreEntryRequest{ this, api_key, universe_id, entry.get_datastore_name(), entry.get_scope(), entry.get_key() };
		get_entry_details_request->set_http_429_count(http_429_count);
		connect(get_entry_details_request, &GetStandardDatastoreEntriesRequest::received_http_429, this, &DownloadDatastoreProgressWindow::handle_received_http_429);
		connect(get_entry_details_request, &GetStandardDatastoreEntryRequest::status_message, this, &DownloadDatastoreProgressWindow::handle_status_message);
		connect(get_entry_details_request, &GetStandardDatastoreEntryRequest::request_complete, this, &DownloadDatastoreProgressWindow::handle_get_entry_details_complete);
		get_entry_details_request->send_request();
	}
	else
	{
		close_button->setText("Close");
		handle_status_message("Download complete");
	}
}

void DownloadDatastoreProgressWindow::handle_status_message(const QString message)
{
	text_box->append(message);
	update_ui();
}

void DownloadDatastoreProgressWindow::handle_list_keys_complete()
{
	if (get_entries_request)
	{
		const std::vector<StandardDatastoreEntry>& new_entries = get_entries_request->get_datastore_entries();
		pending_entries.insert(pending_entries.end(), new_entries.begin(), new_entries.end());
		progress.advance_datastore_done();
		progress.set_entry_total(pending_entries.size());

		get_entries_request->deleteLater();
		get_entries_request = nullptr;

		send_next_list_keys_request();
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

void DownloadDatastoreProgressWindow::handle_received_http_429()
{
	http_429_count++;
}

DownloadDatastoreProgressWindow::DownloadProgress::DownloadProgress(const size_t datastore_total) : datastore_total{ datastore_total }
{

}

bool DownloadDatastoreProgressWindow::DownloadProgress::is_enumerating() const
{
	return datastore_done < datastore_total;
}

bool DownloadDatastoreProgressWindow::DownloadProgress::is_done() const
{
	return entry_total.has_value() && entry_done >= *entry_total;
}

size_t DownloadDatastoreProgressWindow::DownloadProgress::get_current_datastore_index() const
{
	return datastore_done;
}

size_t DownloadDatastoreProgressWindow::DownloadProgress::get_current_entry_index() const
{
	return entry_done;
}

size_t DownloadDatastoreProgressWindow::DownloadProgress::get_progress() const
{
	if (entry_total)
	{
		double progress = static_cast<double>(entry_done) / static_cast<double>(*entry_total);
		return static_cast<size_t>(progress * MAXIMUM);
	}
	else
	{
		return MAXIMUM;
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

void DownloadDatastoreProgressWindow::DownloadProgress::set_entry_total(const size_t total)
{
	entry_total = total;
}

std::optional<size_t> DownloadDatastoreProgressWindow::DownloadProgress::get_entry_total() const
{
	return entry_total;
}
