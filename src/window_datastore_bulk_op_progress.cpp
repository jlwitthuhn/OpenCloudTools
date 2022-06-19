#include "window_datastore_bulk_op_progress.h"

#include <utility>

#include <Qt>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "data_request.h"
#include "user_settings.h"

DatastoreBulkOperationProgressWindow::DatastoreBulkOperationProgressWindow(QWidget* parent, const QString& api_key, const long long universe_id, const QString& scope, const QString& key_prefix, std::vector<QString> datastore_names) :
	QWidget{ parent, Qt::Window },
	api_key{ api_key },
	universe_id{ universe_id },
	scope{ scope },
	key_prefix{ key_prefix },
	progress{ datastore_names.size() },
	datastore_names{ datastore_names }
{
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
	connect(close_button, &QPushButton::clicked, this, &DatastoreBulkOperationProgressWindow::close);

	QVBoxLayout* const layout = new QVBoxLayout{ this };
	layout->addWidget(progress_label);
	layout->addWidget(progress_bar);
	layout->addWidget(text_box);
	layout->addWidget(close_button);

	send_next_enumerate_keys_request();
}

void DatastoreBulkOperationProgressWindow::update_ui()
{
	if (progress.is_done()) {
		progress_label->setText(progress_label_done());
		progress_bar->setMaximum(1);
		progress_bar->setValue(1);
	}
	else if (progress.is_enumerating())
	{
		progress_label->setText("Enumerating entries...");
		progress_bar->setMaximum(0);
		progress_bar->setValue(0);
	}
	else
	{
		const std::optional<size_t> opt_entry_total = progress.get_entry_total();
		if (opt_entry_total)
		{
			progress_label->setText(progress_label_working(*opt_entry_total));
			progress_bar->setValue(static_cast<int>(progress.get_progress()));
			progress_bar->setMaximum(static_cast<int>(DownloadProgress::MAXIMUM));
		}
		else
		{
			progress_label->setText("Error");
		}
	}
}

void DatastoreBulkOperationProgressWindow::send_next_enumerate_keys_request()
{
	const size_t current_index = progress.get_current_datastore_index();
	if (current_index < datastore_names.size())
	{
		const QString this_datastore_name = datastore_names[current_index];

		get_entries_request = new GetStandardDatastoreEntriesRequest{ this, api_key, universe_id, this_datastore_name, scope, key_prefix };
		get_entries_request->set_http_429_count(http_429_count);
		connect(get_entries_request, &GetStandardDatastoreEntriesRequest::received_http_429, this, &DatastoreBulkOperationProgressWindow::handle_received_http_429);
		connect(get_entries_request, &GetStandardDatastoreEntriesRequest::status_message, this, &DatastoreBulkOperationProgressWindow::handle_status_message);
		connect(get_entries_request, &GetStandardDatastoreEntriesRequest::request_complete, this, &DatastoreBulkOperationProgressWindow::handle_enumerate_keys_response);
		get_entries_request->send_request();

		update_ui();
	}
	else
	{
		send_next_entry_request();
	}
}

void DatastoreBulkOperationProgressWindow::handle_status_message(const QString message)
{
	text_box->append(message);
	update_ui();
}

void DatastoreBulkOperationProgressWindow::handle_enumerate_keys_response()
{
	if (get_entries_request)
	{
		const std::vector<StandardDatastoreEntry>& new_entries = get_entries_request->get_datastore_entries();
		pending_entries.insert(pending_entries.end(), new_entries.begin(), new_entries.end());
		progress.advance_datastore_done();
		progress.set_entry_total(pending_entries.size());

		get_entries_request->deleteLater();
		get_entries_request = nullptr;

		send_next_enumerate_keys_request();
	}
}

void DatastoreBulkOperationProgressWindow::handle_received_http_429()
{
	http_429_count++;
}

DatastoreBulkOperationProgressWindow::DownloadProgress::DownloadProgress(const size_t datastore_total) : datastore_total{ datastore_total }
{

}

bool DatastoreBulkOperationProgressWindow::DownloadProgress::is_enumerating() const
{
	return datastore_done < datastore_total;
}

bool DatastoreBulkOperationProgressWindow::DownloadProgress::is_done() const
{
	return entry_total.has_value() && entry_done >= *entry_total;
}

size_t DatastoreBulkOperationProgressWindow::DownloadProgress::get_current_datastore_index() const
{
	return datastore_done;
}

size_t DatastoreBulkOperationProgressWindow::DownloadProgress::get_current_entry_index() const
{
	return entry_done;
}

size_t DatastoreBulkOperationProgressWindow::DownloadProgress::get_progress() const
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

void DatastoreBulkOperationProgressWindow::DownloadProgress::advance_datastore_done()
{
	datastore_done++;
}

void DatastoreBulkOperationProgressWindow::DownloadProgress::advance_entry_done()
{
	entry_done++;
}

void DatastoreBulkOperationProgressWindow::DownloadProgress::set_entry_total(const size_t total)
{
	entry_total = total;
}

std::optional<size_t> DatastoreBulkOperationProgressWindow::DownloadProgress::get_entry_total() const
{
	return entry_total;
}

DatastoreBulkDeleteProgressWindow::DatastoreBulkDeleteProgressWindow(
	QWidget* const parent,
	const QString& api_key,
	const long long universe_id,
	const QString& scope,
	const QString& key_prefix,
	const std::vector<QString> datastore_names,
	const bool hide_datastores_when_done ) :
	DatastoreBulkOperationProgressWindow{ parent, api_key, universe_id, scope, key_prefix, datastore_names },
	hide_datastores_when_done{ hide_datastores_when_done }
{
	setWindowTitle("Delete Progress");
}

QString DatastoreBulkDeleteProgressWindow::progress_label_done() const
{
	return "Delete complete";
}

QString DatastoreBulkDeleteProgressWindow::progress_label_working(const size_t total) const
{
	return QString{ "Deleting entry %1/%2..." }.arg(progress.get_current_entry_index() + 1).arg(total);
}

void DatastoreBulkDeleteProgressWindow::send_next_entry_request()
{
	if (pending_entries.size() > 0)
	{
		StandardDatastoreEntry entry = pending_entries.back();
		pending_entries.pop_back();

		delete_entry_request = new DeleteStandardDatastoreEntryRequest{ this, api_key, universe_id, entry.get_datastore_name(), entry.get_scope(), entry.get_key() };
		delete_entry_request->set_http_429_count(http_429_count);
		connect(delete_entry_request, &DeleteStandardDatastoreEntryRequest::received_http_429, this, &DatastoreBulkDeleteProgressWindow::handle_received_http_429);
		connect(delete_entry_request, &DeleteStandardDatastoreEntryRequest::status_message, this, &DatastoreBulkDeleteProgressWindow::handle_status_message);
		connect(delete_entry_request, &DeleteStandardDatastoreEntryRequest::request_complete, this, &DatastoreBulkDeleteProgressWindow::handle_entry_response);
		delete_entry_request->send_request();
	}
	else
	{
		if (hide_datastores_when_done)
		{
			for (const QString& this_name : datastore_names)
			{
				UserSettings::get()->add_hidden_datastore(this_name);
				handle_status_message( QString{ "Hid datastore: '%1'" }.arg(this_name) );
			}
		}
		close_button->setText("Close");
		handle_status_message("Bulk delete complete");
	}
}

void DatastoreBulkDeleteProgressWindow::handle_entry_response()
{
	if (delete_entry_request)
	{
		progress.advance_entry_done();
		delete_entry_request->deleteLater();
		delete_entry_request = nullptr;
		send_next_entry_request();
	}
}

DatastoreBulkDownloadProgressWindow::DatastoreBulkDownloadProgressWindow(
	QWidget* parent,
	const QString& api_key,
	long long universe_id,
	const QString& scope,
	const QString& key_prefix,
	std::vector<QString> datastore_names,
	std::unique_ptr<SqliteDatastoreWriter> writer) :
	DatastoreBulkOperationProgressWindow{ parent, api_key, universe_id, scope, key_prefix, datastore_names },
	writer{ std::move(writer) }
{
	setWindowTitle("Download Progress");
}

QString DatastoreBulkDownloadProgressWindow::progress_label_done() const
{
	return "Download complete";
}

QString DatastoreBulkDownloadProgressWindow::progress_label_working(const size_t total) const
{
	return QString{ "Downloading entry %1/%2..." }.arg(progress.get_current_entry_index() + 1).arg(total);
}

void DatastoreBulkDownloadProgressWindow::send_next_entry_request()
{
	if (pending_entries.size() > 0)
	{
		StandardDatastoreEntry entry = pending_entries.back();
		pending_entries.pop_back();

		get_entry_details_request = new GetStandardDatastoreEntryRequest{ this, api_key, universe_id, entry.get_datastore_name(), entry.get_scope(), entry.get_key() };
		get_entry_details_request->set_http_429_count(http_429_count);
		connect(get_entry_details_request, &GetStandardDatastoreEntriesRequest::received_http_429, this, &DatastoreBulkDownloadProgressWindow::handle_received_http_429);
		connect(get_entry_details_request, &GetStandardDatastoreEntryRequest::status_message, this, &DatastoreBulkDownloadProgressWindow::handle_status_message);
		connect(get_entry_details_request, &GetStandardDatastoreEntryRequest::request_complete, this, &DatastoreBulkDownloadProgressWindow::handle_entry_response);
		get_entry_details_request->send_request();
	}
	else
	{
		close_button->setText("Close");
		handle_status_message("Download complete");
	}
}

void DatastoreBulkDownloadProgressWindow::handle_entry_response()
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
		send_next_entry_request();
	}
}
