#include "window_datastore_bulk_op_progress.h"

#include <algorithm>
#include <utility>

#include <Qt>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "data_request.h"
#include "roblox_time.h"
#include "user_settings.h"

DatastoreBulkOperationProgressWindow::DatastoreBulkOperationProgressWindow(QWidget* parent, const QString& api_key, const long long universe_id, const QString& find_scope, const QString& find_key_prefix, std::vector<QString> datastore_names) :
	QWidget{ parent, Qt::Window },
	api_key{ api_key },
	universe_id{ universe_id },
	find_scope{ find_scope },
	find_key_prefix{ find_key_prefix },
	progress{ datastore_names.size() },
	datastore_names{ datastore_names }
{
	setAttribute(Qt::WA_DeleteOnClose);

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

		get_entries_request = new GetStandardDatastoreEntriesRequest{ this, api_key, universe_id, this_datastore_name, find_scope, find_key_prefix };
		get_entries_request->set_http_429_count(http_429_count);
		connect(get_entries_request, &GetStandardDatastoreEntriesRequest::received_http_429, this, &DatastoreBulkOperationProgressWindow::handle_received_http_429);
		connect(get_entries_request, &GetStandardDatastoreEntriesRequest::status_error, this, &DatastoreBulkOperationProgressWindow::handle_status_message);
		if (UserSettings::get()->get_less_verbose_bulk_operations() == false)
		{
			connect(get_entries_request, &GetStandardDatastoreEntriesRequest::status_info, this, &DatastoreBulkOperationProgressWindow::handle_status_message);
		}
		connect(get_entries_request, &GetStandardDatastoreEntriesRequest::request_complete, this, &DatastoreBulkOperationProgressWindow::handle_enumerate_keys_response);
		get_entries_request->send_request();

		handle_status_message(QString{ "Enumerating entries for '%1'..." }.arg(this_datastore_name));
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
	const bool confirm_count_before_delete ,
	const bool rewrite_before_delete,
	const bool hide_datastores_when_done) :
	DatastoreBulkOperationProgressWindow{ parent, api_key, universe_id, scope, key_prefix, datastore_names },
	confirm_count_before_delete{ confirm_count_before_delete },
	rewrite_before_delete{ rewrite_before_delete },
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
	if (confirm_count_before_delete && first_delete_request_sent == false)
	{
		QString message = QString{ "This operation will delete %1 entries. Are you sure you want to proceed?" }.arg(pending_entries.size());

		QMessageBox* msg_box = new QMessageBox{ this };
		msg_box->setWindowTitle("Confirm deletion");
		msg_box->setText(message);
		msg_box->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		if (msg_box->exec() == QMessageBox::No)
		{
			handle_status_message("Bulk delete aborted");
			close_button->setText("Close");
			return;
		}
	}

	if (pending_entries.size() > 0)
	{
		StandardDatastoreEntry entry = pending_entries.back();
		pending_entries.pop_back();

		if (rewrite_before_delete)
		{
			get_entry_request = new GetStandardDatastoreEntryDetailsRequest{ this, api_key, universe_id, entry.get_datastore_name(), entry.get_scope(), entry.get_key() };
			get_entry_request->set_http_429_count(http_429_count);
			connect(get_entry_request, &GetStandardDatastoreEntryDetailsRequest::received_http_429, this, &DatastoreBulkDeleteProgressWindow::handle_received_http_429);
			connect(get_entry_request, &GetStandardDatastoreEntryDetailsRequest::status_error, this, &DatastoreBulkDeleteProgressWindow::handle_status_message);
			if (UserSettings::get()->get_less_verbose_bulk_operations() == false)
			{
				connect(get_entry_request, &GetStandardDatastoreEntryDetailsRequest::status_info, this, &DatastoreBulkDeleteProgressWindow::handle_status_message);
			}
			connect(get_entry_request, &GetStandardDatastoreEntryDetailsRequest::request_complete, this, &DatastoreBulkDeleteProgressWindow::handle_get_entry_response);
			get_entry_request->send_request();

			handle_status_message( QString{ "Rewriting and deleting '%1'..." }.arg( entry.get_key() ) );
		}
		else
		{
			delete_entry_request = new DeleteStandardDatastoreEntryRequest{ this, api_key, universe_id, entry.get_datastore_name(), entry.get_scope(), entry.get_key() };
			delete_entry_request->set_http_429_count(http_429_count);
			connect(delete_entry_request, &DeleteStandardDatastoreEntryRequest::received_http_429, this, &DatastoreBulkDeleteProgressWindow::handle_received_http_429);
			connect(delete_entry_request, &DeleteStandardDatastoreEntryRequest::status_error, this, &DatastoreBulkDeleteProgressWindow::handle_status_message);
			if (UserSettings::get()->get_less_verbose_bulk_operations() == false)
			{
				connect(delete_entry_request, &DeleteStandardDatastoreEntryRequest::status_info, this, &DatastoreBulkDeleteProgressWindow::handle_status_message);
			}
			connect(delete_entry_request, &DeleteStandardDatastoreEntryRequest::request_complete, this, &DatastoreBulkDeleteProgressWindow::handle_delete_entry_response);
			delete_entry_request->send_request();

			handle_status_message(QString{ "Deleting '%1'..." }.arg(entry.get_key()));
		}
		first_delete_request_sent = true;
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
		handle_status_message(get_summary());
	}
}

void DatastoreBulkDeleteProgressWindow::handle_get_entry_response()
{
	if (get_entry_request)
	{
		const std::optional<DatastoreEntryWithDetails> opt_details = get_entry_request->get_details();

		get_entry_request->deleteLater();
		get_entry_request = nullptr;

		if (opt_details)
		{
			const QString datastore_name = opt_details->get_datastore_name();
			const QString scope = opt_details->get_scope();
			const QString key_name = opt_details->get_key_name();
			const std::optional<QString> userids = opt_details->get_userids();
			const std::optional<QString> attributes = opt_details->get_attributes();
			const QString body = opt_details->get_data_raw();

			post_entry_request = new PostStandardDatastoreEntryRequest{ this, api_key, universe_id, datastore_name, scope, key_name, userids, attributes, body };
			post_entry_request->set_http_429_count(http_429_count);
			connect(post_entry_request, &PostStandardDatastoreEntryRequest::received_http_429, this, &DatastoreBulkDeleteProgressWindow::handle_received_http_429);
			connect(post_entry_request, &PostStandardDatastoreEntryRequest::status_error, this, &DatastoreBulkDeleteProgressWindow::handle_status_message);
			if (UserSettings::get()->get_less_verbose_bulk_operations() == false)
			{
				connect(post_entry_request, &PostStandardDatastoreEntryRequest::status_info, this, &DatastoreBulkDeleteProgressWindow::handle_status_message);
			}
			connect(post_entry_request, &PostStandardDatastoreEntryRequest::request_complete, this, &DatastoreBulkDeleteProgressWindow::handle_post_entry_response);
			post_entry_request->send_request();
		}
		else
		{
			entries_already_deleted++;
			handle_status_message("Entry was already deleted");
			progress.advance_entry_done();
			send_next_entry_request();
		}
	}
}

void DatastoreBulkDeleteProgressWindow::handle_post_entry_response()
{
	if (post_entry_request)
	{
		const QString datastore_name = post_entry_request->get_datastore_name();
		const QString scope = post_entry_request->get_scope();
		const QString key_name = post_entry_request->get_key_name();

		post_entry_request->deleteLater();
		post_entry_request = nullptr;

		delete_entry_request = new DeleteStandardDatastoreEntryRequest{ this, api_key, universe_id, datastore_name, scope, key_name };
		delete_entry_request->set_http_429_count(http_429_count);
		connect(delete_entry_request, &DeleteStandardDatastoreEntryRequest::received_http_429, this, &DatastoreBulkDeleteProgressWindow::handle_received_http_429);
		connect(delete_entry_request, &DeleteStandardDatastoreEntryRequest::status_error, this, &DatastoreBulkDeleteProgressWindow::handle_status_message);
		if (UserSettings::get()->get_less_verbose_bulk_operations() == false)
		{
			connect(delete_entry_request, &DeleteStandardDatastoreEntryRequest::status_info, this, &DatastoreBulkDeleteProgressWindow::handle_status_message);
		}
		connect(delete_entry_request, &DeleteStandardDatastoreEntryRequest::request_complete, this, &DatastoreBulkDeleteProgressWindow::handle_delete_entry_response);
		delete_entry_request->send_request();
	}
}

void DatastoreBulkDeleteProgressWindow::handle_delete_entry_response()
{
	if (delete_entry_request)
	{
		const std::optional<bool> success = delete_entry_request->is_delete_success();

		delete_entry_request->deleteLater();
		delete_entry_request = nullptr;

		if (success)
		{
			if (*success)
			{
				entries_deleted++;
				handle_status_message("Entry deleted");
			}
			else
			{
				entries_already_deleted++;
				handle_status_message("Entry was already deleted");
			}
		}

		progress.advance_entry_done();
		send_next_entry_request();
	}
}

QString DatastoreBulkDeleteProgressWindow::get_summary() const
{
	QString result = QString{ "%1 entries deleted" }.arg(entries_deleted);
	if (entries_already_deleted > 0)
	{
		result = result + QString{ ", %1 entries already deleted" }.arg(entries_already_deleted);
	}
	return result;
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

		get_entry_details_request = new GetStandardDatastoreEntryDetailsRequest{ this, api_key, universe_id, entry.get_datastore_name(), entry.get_scope(), entry.get_key() };
		get_entry_details_request->set_http_429_count(http_429_count);
		connect(get_entry_details_request, &GetStandardDatastoreEntryDetailsRequest::received_http_429, this, &DatastoreBulkDownloadProgressWindow::handle_received_http_429);
		connect(get_entry_details_request, &GetStandardDatastoreEntryDetailsRequest::status_error, this, &DatastoreBulkDownloadProgressWindow::handle_status_message);
		if (UserSettings::get()->get_less_verbose_bulk_operations() == false)
		{
			connect(get_entry_details_request, &GetStandardDatastoreEntryDetailsRequest::status_info, this, &DatastoreBulkDownloadProgressWindow::handle_status_message);
		}
		connect(get_entry_details_request, &GetStandardDatastoreEntryDetailsRequest::request_complete, this, &DatastoreBulkDownloadProgressWindow::handle_entry_response);
		get_entry_details_request->send_request();

		handle_status_message(QString{ "Downloading '%1'..." }.arg(entry.get_key()));
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
		const std::optional<DatastoreEntryWithDetails> opt_details = get_entry_details_request->get_details();
		if (opt_details)
		{
			writer->write(*opt_details);
		}
		progress.advance_entry_done();
		get_entry_details_request->deleteLater();
		get_entry_details_request = nullptr;
		send_next_entry_request();
	}
}

DatastoreBulkUndeleteProgressWindow::DatastoreBulkUndeleteProgressWindow(
	QWidget* parent,
	const QString& api_key,
	long long universe_id,
	const QString& scope,
	const QString& key_prefix,
	std::vector<QString> datastore_names,
	std::optional<QDateTime> undelete_after) :
	DatastoreBulkOperationProgressWindow{ parent, api_key, universe_id, scope, key_prefix, datastore_names },
	undelete_after{ undelete_after }
{
	setWindowTitle("Undelete Progress");
}

QString DatastoreBulkUndeleteProgressWindow::progress_label_done() const
{
	return "Undelete complete";
}

QString DatastoreBulkUndeleteProgressWindow::progress_label_working(const size_t total) const
{
	return QString{ "Undeleting entry %1/%2..." }.arg(progress.get_current_entry_index() + 1).arg(total);
}

void DatastoreBulkUndeleteProgressWindow::send_next_entry_request()
{
	if (pending_entries.size() > 0)
	{
		StandardDatastoreEntry entry = pending_entries.back();
		pending_entries.pop_back();

		get_versions_request = new GetStandardDatastoreEntryVersionsRequest{ this, api_key, universe_id, entry.get_datastore_name(), entry.get_scope(), entry.get_key() };
		get_versions_request->set_http_429_count(http_429_count);
		connect(get_versions_request, &GetStandardDatastoreEntryVersionsRequest::received_http_429, this, &DatastoreBulkUndeleteProgressWindow::handle_received_http_429);
		connect(get_versions_request, &GetStandardDatastoreEntryVersionsRequest::status_error, this, &DatastoreBulkUndeleteProgressWindow::handle_status_message);
		if (UserSettings::get()->get_less_verbose_bulk_operations() == false)
		{
			connect(get_versions_request, &GetStandardDatastoreEntryVersionsRequest::status_info, this, &DatastoreBulkUndeleteProgressWindow::handle_status_message);
		}
		connect(get_versions_request, &GetStandardDatastoreEntryVersionsRequest::request_complete, this, &DatastoreBulkUndeleteProgressWindow::handle_get_versions_response);
		get_versions_request->send_request();

		handle_status_message(QString{ "Undeleting '%1'..." }.arg(entry.get_key()));
	}
	else
	{
		close_button->setText("Close");
		handle_status_message("Undelete complete");
		QString summary = QString{ "%1 entries restored, %2 already existed, %3 could not be restored" }.arg(entries_restored).arg(entries_not_deleted).arg(entries_no_old_version);
		if (entries_not_in_time_range > 0)
		{
			summary = summary + QString{ ", %1 not in selected time range" }.arg(entries_not_in_time_range);
		}
		if (entries_errored > 0)
		{
			summary = summary + QString{ ", %1 errors" }.arg(entries_errored);
		}
		handle_status_message(summary);
	}
}

void DatastoreBulkUndeleteProgressWindow::handle_get_versions_response()
{
	if (get_versions_request)
	{
		std::vector<StandardDatastoreEntryVersion> versions = get_versions_request->get_versions();
		const QString datastore_name = get_versions_request->get_datastore_name();
		const QString scope = get_versions_request->get_scope();
		const QString key_name = get_versions_request->get_key_name();
		get_versions_request->deleteLater();
		get_versions_request = nullptr;

		std::sort(versions.begin(), versions.end(),
			[](const StandardDatastoreEntryVersion& a, const StandardDatastoreEntryVersion& b)
			{
				return b.get_version() < a.get_version();
			}
		);

		if (versions.size() == 0)
		{
			handle_status_message("No versions found, skipping");
			entries_errored++;
			progress.advance_entry_done();
			send_next_entry_request();
			return;
		}

		if (versions.front().get_deleted() == false)
		{
			handle_status_message("Not deleted, skipping");
			entries_not_deleted++;
			progress.advance_entry_done();
			send_next_entry_request();
			return;
		}

		if (undelete_after)
		{
			const std::optional<QDateTime> opt_front_date = RobloxTime::parse_version_date(versions.front().get_created_time());
			if (opt_front_date)
			{
				if (*opt_front_date < *undelete_after)
				{
					handle_status_message("Deleted outside of selected time range, skipping");
					entries_not_in_time_range++;
					progress.advance_entry_done();
					send_next_entry_request();
					return;
				}
				else
				{
					// Advance
				}
			}
			else
			{
				handle_status_message("Failed to parse version timestamp, skipping");
				entries_errored++;
				progress.advance_entry_done();
				send_next_entry_request();
				return;
			}
		}

		std::optional<StandardDatastoreEntryVersion> target_version;
		for (const StandardDatastoreEntryVersion& this_version : versions)
		{
			if (this_version.get_deleted() == false)
			{
				target_version = this_version;
				break;
			}
		}

		if (target_version.has_value() == false)
		{
			handle_status_message("No old version available, skipping");
			entries_no_old_version++;
			progress.advance_entry_done();
			send_next_entry_request();
			return;
		}

		get_entry_at_version_request = new GetStandardDatastoreEntryAtVersionRequest{ this, api_key, universe_id, datastore_name, scope, key_name, target_version->get_version() };
		get_entry_at_version_request->set_http_429_count(http_429_count);
		connect(get_entry_at_version_request, &GetStandardDatastoreEntryVersionsRequest::received_http_429, this, &DatastoreBulkUndeleteProgressWindow::handle_received_http_429);
		connect(get_entry_at_version_request, &GetStandardDatastoreEntryVersionsRequest::status_error, this, &DatastoreBulkUndeleteProgressWindow::handle_status_message);
		if (UserSettings::get()->get_less_verbose_bulk_operations() == false)
		{
			connect(get_entry_at_version_request, &GetStandardDatastoreEntryVersionsRequest::status_info, this, &DatastoreBulkUndeleteProgressWindow::handle_status_message);
		}
		connect(get_entry_at_version_request, &GetStandardDatastoreEntryVersionsRequest::request_complete, this, &DatastoreBulkUndeleteProgressWindow::handle_get_entry_version_response);
		get_entry_at_version_request->send_request();
	}
}

void DatastoreBulkUndeleteProgressWindow::handle_get_entry_version_response()
{
	if (get_entry_at_version_request)
	{
		const std::optional<DatastoreEntryWithDetails> opt_details = get_entry_at_version_request->get_details();
		get_entry_at_version_request->deleteLater();
		get_entry_at_version_request = nullptr;

		if (opt_details.has_value() == false)
		{
			handle_status_message("Failed to fetch version, skipping");
			entries_errored++;
			progress.advance_entry_done();
			send_next_entry_request();
			return;
		}

		const QString datastore_name = opt_details->get_datastore_name();
		const QString scope = opt_details->get_scope();
		const QString key_name = opt_details->get_key_name();
		const std::optional<QString> userids = opt_details->get_userids();
		const std::optional<QString> attributes = opt_details->get_attributes();
		const QString body = opt_details->get_data_raw();

		post_entry_request = new PostStandardDatastoreEntryRequest{ this, api_key, universe_id, datastore_name, scope, key_name, userids, attributes, body };
		post_entry_request->set_http_429_count(http_429_count);
		connect(post_entry_request, &PostStandardDatastoreEntryRequest::received_http_429, this, &DatastoreBulkUndeleteProgressWindow::handle_received_http_429);
		connect(post_entry_request, &PostStandardDatastoreEntryRequest::status_error, this, &DatastoreBulkUndeleteProgressWindow::handle_status_message);
		if (UserSettings::get()->get_less_verbose_bulk_operations() == false)
		{
			connect(post_entry_request, &PostStandardDatastoreEntryRequest::status_info, this, &DatastoreBulkUndeleteProgressWindow::handle_status_message);
		}
		connect(post_entry_request, &PostStandardDatastoreEntryRequest::request_complete, this, &DatastoreBulkUndeleteProgressWindow::handle_post_entry_response);
		post_entry_request->send_request();
	}
}

void DatastoreBulkUndeleteProgressWindow::handle_post_entry_response()
{
	if (post_entry_request)
	{
		const bool success = post_entry_request->get_success();
		post_entry_request->deleteLater();
		post_entry_request = nullptr;

		if (success)
		{
			handle_status_message("Restore complete");
			entries_restored++;
		}
		else
		{
			handle_status_message("Restore failed");
			entries_errored++;
		}
		progress.advance_entry_done();
		send_next_entry_request();
	}
}
