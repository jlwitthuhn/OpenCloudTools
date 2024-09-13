#include "window_datastore_bulk_op_progress.h"

#include <algorithm>
#include <utility>

#include <Qt>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

#include "assert.h"
#include "data_request.h"
#include "profile.h"
#include "roblox_time.h"
#include "widget_text_log.h"

void DatastoreBulkOperationProgressWindow::start()
{
	send_next_enumerate_keys_request();
}

DatastoreBulkOperationProgressWindow::DatastoreBulkOperationProgressWindow(QWidget* parent, const QString& api_key, const long long universe_id, const QString& find_scope, const QString& find_key_prefix, std::vector<QString> datastore_names) :
	QWidget{ parent, Qt::Window },
	api_key{ api_key },
	universe_id{ universe_id },
	find_scope{ find_scope },
	find_key_prefix{ find_key_prefix },
	progress{ datastore_names.size() },
	datastore_names{ std::move(datastore_names) }
{
	setAttribute(Qt::WA_DeleteOnClose);
	setMinimumHeight(380);

	OCTASSERT(parent != nullptr);
	setWindowModality(Qt::WindowModality::ApplicationModal);

	progress_label = new QLabel{ "", this };
	progress_bar = new QProgressBar{ this };
	progress_bar->setMinimumWidth(360);
	progress_bar->setTextVisible(false);
	progress_bar->setMaximum(DownloadProgress::MAXIMUM);

	text_log = new TextLogWidget{ this };

	retry_button = new QPushButton{ "Retry", this };
	retry_button->setEnabled(false);
	connect(retry_button, &QPushButton::clicked, this, &DatastoreBulkOperationProgressWindow::handle_clicked_retry);

	close_button = new QPushButton{ "Stop", this };
	connect(close_button, &QPushButton::clicked, this, &DatastoreBulkOperationProgressWindow::close);

	QVBoxLayout* const layout = new QVBoxLayout{ this };
	layout->addWidget(progress_label);
	layout->addWidget(progress_bar);
	layout->addWidget(text_log);
	layout->addWidget(retry_button);
	layout->addWidget(close_button);

	update_ui();
}

bool DatastoreBulkOperationProgressWindow::is_retryable() const
{
	return enumerate_entries_request && enumerate_entries_request->req_status() == DataRequestStatus::Error;
}

bool DatastoreBulkOperationProgressWindow::do_retry()
{
	if (DatastoreBulkOperationProgressWindow::is_retryable())
	{
		enumerate_entries_request->force_retry();
		return true;
	}
	else
	{
		return false;
	}
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
		const size_t total_found = enumerate_entries_request ? enumerate_entries_request->get_datastore_entries().size() + pending_entries.size() : pending_entries.size();
		progress_label->setText(QString{ "Enumerating entries, found %1..." }.arg(total_found));
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

		enumerate_entries_request = std::make_shared<StandardDatastoreEntryGetListRequest>(api_key, universe_id, this_datastore_name, find_scope, find_key_prefix, initial_cursor);
		initial_cursor = std::nullopt;
		enumerate_entries_request->set_http_429_count(http_429_count);
		enumerate_entries_request->set_enumerate_step_callback(datastore_enumerate_step_callback);
		enumerate_entries_request->set_enumerate_done_callback(datastore_enumerate_done_callback);
		enumerate_entries_request->set_entry_found_callback(entry_found_callback);
		connect(enumerate_entries_request.get(), &StandardDatastoreEntryGetListRequest::received_http_429, this, &DatastoreBulkOperationProgressWindow::handle_received_http_429);
		connect(enumerate_entries_request.get(), &StandardDatastoreEntryGetListRequest::status_error, this, &DatastoreBulkOperationProgressWindow::handle_error_message);
		connect(enumerate_entries_request.get(), &StandardDatastoreEntryGetListRequest::status_error, this, &DatastoreBulkOperationProgressWindow::handle_error_message);
		if (UserProfile::get().get_less_verbose_bulk_operations() == false)
		{
			connect(enumerate_entries_request.get(), &StandardDatastoreEntryGetListRequest::status_info, this, &DatastoreBulkOperationProgressWindow::handle_status_message);
		}
		connect(enumerate_entries_request.get(), &StandardDatastoreEntryGetListRequest::success, this, &DatastoreBulkOperationProgressWindow::handle_enumerate_keys_success);
		enumerate_entries_request->send_request();

		handle_status_message(QString{ "Enumerating entries for '%1'..." }.arg(this_datastore_name));
	}
	else
	{
		send_next_entry_request();
	}
}

void DatastoreBulkOperationProgressWindow::handle_clicked_retry()
{
	retry_button->setEnabled(false);
	do_retry();
}

void DatastoreBulkOperationProgressWindow::handle_error_message(const QString message)
{
	handle_status_message(message);
	update_ui();
	retry_button->setEnabled(is_retryable());
}

void DatastoreBulkOperationProgressWindow::handle_status_message(const QString message)
{
	text_log->append(message);
	update_ui();
}

void DatastoreBulkOperationProgressWindow::handle_enumerate_keys_success()
{
	if (enumerate_entries_request)
	{
		if (pending_entries.size() == 0)
		{
			// When no entries exist yet, move instead of appending
			pending_entries = std::move(enumerate_entries_request->get_datastore_entries_rvalue());
		}
		else
		{
			const std::vector<StandardDatastoreEntryName>& new_entries = enumerate_entries_request->get_datastore_entries();
			pending_entries.insert(pending_entries.end(), new_entries.begin(), new_entries.end());
		}
		progress.advance_datastore_done();
		progress.set_entry_total(pending_entries.size());

		enumerate_entries_request.reset();

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
		const double progress_fraction = static_cast<double>(entry_done) / static_cast<double>(*entry_total);
		return static_cast<size_t>(progress_fraction * MAXIMUM);
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
	const std::shared_ptr<UniverseProfile>& universe,
	const QString& scope,
	const QString& key_prefix,
	const std::vector<QString> datastore_names,
	const bool confirm_count_before_delete ,
	const bool rewrite_before_delete,
	const bool hide_datastores_when_done) :
	DatastoreBulkOperationProgressWindow{ parent, api_key, universe->get_universe_id(), scope, key_prefix, datastore_names},
	attached_universe{ universe },
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
		StandardDatastoreEntryName entry = pending_entries.back();
		pending_entries.pop_back();

		if (rewrite_before_delete)
		{
			get_entry_request = std::make_shared<StandardDatastoreEntryGetDetailsRequest>(api_key, universe_id, entry.get_datastore_name(), entry.get_scope(), entry.get_key());
			get_entry_request->set_http_429_count(http_429_count);
			connect(get_entry_request.get(), &StandardDatastoreEntryGetDetailsRequest::received_http_429, this, &DatastoreBulkDeleteProgressWindow::handle_received_http_429);
			connect(get_entry_request.get(), &StandardDatastoreEntryGetDetailsRequest::status_error, this, &DatastoreBulkDeleteProgressWindow::handle_error_message);
			if (UserProfile::get().get_less_verbose_bulk_operations() == false)
			{
				connect(get_entry_request.get(), &StandardDatastoreEntryGetDetailsRequest::status_info, this, &DatastoreBulkDeleteProgressWindow::handle_status_message);
			}
			connect(get_entry_request.get(), &StandardDatastoreEntryGetDetailsRequest::success, this, &DatastoreBulkDeleteProgressWindow::handle_get_entry_response);
			get_entry_request->send_request();

			handle_status_message( QString{ "Rewriting and deleting '%1'..." }.arg( entry.get_key() ) );
		}
		else
		{
			delete_entry_request = std::make_shared<StandardDatastoreEntryDeleteRequest>(api_key, universe_id, entry.get_datastore_name(), entry.get_scope(), entry.get_key());
			delete_entry_request->set_http_429_count(http_429_count);
			connect(delete_entry_request.get(), &StandardDatastoreEntryDeleteRequest::received_http_429, this, &DatastoreBulkDeleteProgressWindow::handle_received_http_429);
			connect(delete_entry_request.get(), &StandardDatastoreEntryDeleteRequest::status_error, this, &DatastoreBulkDeleteProgressWindow::handle_error_message);
			if (UserProfile::get().get_less_verbose_bulk_operations() == false)
			{
				connect(delete_entry_request.get(), &StandardDatastoreEntryDeleteRequest::status_info, this, &DatastoreBulkDeleteProgressWindow::handle_status_message);
			}
			connect(delete_entry_request.get(), &StandardDatastoreEntryDeleteRequest::success, this, &DatastoreBulkDeleteProgressWindow::handle_delete_entry_response);
			delete_entry_request->send_request();

			handle_status_message(QString{ "Deleting '%1'..." }.arg(entry.get_key()));
		}
		first_delete_request_sent = true;
	}
	else
	{
		if (const std::shared_ptr<UniverseProfile> universe = attached_universe.lock())
		{
			if (hide_datastores_when_done)
			{
				for (const QString& this_name : datastore_names)
				{
					universe->add_hidden_datastore(this_name);;
					handle_status_message(QString{ "Hid datastore: '%1'" }.arg(this_name));
				}
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
		const std::optional<StandardDatastoreEntryFull> opt_details = get_entry_request->get_details();

		get_entry_request.reset();

		if (opt_details)
		{
			const QString datastore_name = opt_details->get_datastore_name();
			const QString scope = opt_details->get_scope();
			const QString key_name = opt_details->get_key_name();
			const std::optional<QString> userids = opt_details->get_userids();
			const std::optional<QString> attributes = opt_details->get_attributes();
			const QString body = opt_details->get_data_raw();

			post_entry_request = std::make_shared<StandardDatastoreEntryPostSetRequest>(api_key, universe_id, datastore_name, scope, key_name, userids, attributes, body);
			post_entry_request->set_http_429_count(http_429_count);
			connect(post_entry_request.get(), &StandardDatastoreEntryPostSetRequest::received_http_429, this, &DatastoreBulkDeleteProgressWindow::handle_received_http_429);
			connect(post_entry_request.get(), &StandardDatastoreEntryPostSetRequest::status_error, this, &DatastoreBulkDeleteProgressWindow::handle_error_message);
			if (UserProfile::get().get_less_verbose_bulk_operations() == false)
			{
				connect(post_entry_request.get(), &StandardDatastoreEntryPostSetRequest::status_info, this, &DatastoreBulkDeleteProgressWindow::handle_status_message);
			}
			connect(post_entry_request.get(), &StandardDatastoreEntryPostSetRequest::success, this, &DatastoreBulkDeleteProgressWindow::handle_post_entry_response);
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

		post_entry_request.reset();

		delete_entry_request = std::make_shared<StandardDatastoreEntryDeleteRequest>(api_key, universe_id, datastore_name, scope, key_name);
		delete_entry_request->set_http_429_count(http_429_count);
		connect(delete_entry_request.get(), &StandardDatastoreEntryDeleteRequest::received_http_429, this, &DatastoreBulkDeleteProgressWindow::handle_received_http_429);
		connect(delete_entry_request.get(), &StandardDatastoreEntryDeleteRequest::status_error, this, &DatastoreBulkDeleteProgressWindow::handle_error_message);
		if (UserProfile::get().get_less_verbose_bulk_operations() == false)
		{
			connect(delete_entry_request.get(), &StandardDatastoreEntryDeleteRequest::status_info, this, &DatastoreBulkDeleteProgressWindow::handle_status_message);
		}
		connect(delete_entry_request.get(), &StandardDatastoreEntryDeleteRequest::success, this, &DatastoreBulkDeleteProgressWindow::handle_delete_entry_response);
		delete_entry_request->send_request();
	}
}

void DatastoreBulkDeleteProgressWindow::handle_delete_entry_response()
{
	if (delete_entry_request)
	{
		const std::optional<bool> success = delete_entry_request->is_delete_success();

		delete_entry_request.reset();

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
	std::unique_ptr<SqliteDatastoreWrapper> db_wrapper) :
	DatastoreBulkOperationProgressWindow{ parent, api_key, universe_id, scope, key_prefix, std::move(datastore_names) },
	db_wrapper{ std::move(db_wrapper) }
{
	this->db_wrapper->write_enumeration_metadata(universe_id, scope.toStdString(), key_prefix.toStdString());
	// Initialize all targeted datastore names in the sqlite db
	for (const QString& this_datastore : this->datastore_names)
	{
		this->db_wrapper->write_enumeration(universe_id, this_datastore.toStdString());
	}

	common_init();
}

DatastoreBulkDownloadProgressWindow::DatastoreBulkDownloadProgressWindow(
	QWidget* parent,
	const QString& api_key,
	long long universe_id,
	std::unique_ptr<SqliteDatastoreWrapper> db_wrapper) :
	DatastoreBulkOperationProgressWindow{ parent, api_key, universe_id, "", "", std::vector<QString>{} },
	db_wrapper{ std::move(db_wrapper) }
{
	pending_entries = this->db_wrapper->get_pending_entries(universe_id);

	if (const std::optional<std::string> opt_key_prefix = this->db_wrapper->get_enumeration_search_key_prefix(universe_id))
	{
		this->find_key_prefix = QString::fromStdString(*opt_key_prefix);
	}
	if (const std::optional<std::string> opt_scope = this->db_wrapper->get_enumeration_search_scope(universe_id))
	{
		this->find_scope = QString::fromStdString(*opt_scope);
	}

	datastore_names.clear();
	if (const std::optional<std::string> opt_name = this->db_wrapper->get_enumarating_datastore(universe_id))
	{
		datastore_names.push_back(QString::fromStdString(*opt_name));
	}
	if (const std::optional<std::string> opt_cursor = this->db_wrapper->get_enumarating_cursor(universe_id))
	{
		initial_cursor = QString::fromStdString(*opt_cursor);
	}
	for (const std::string& this_datastore_name : this->db_wrapper->get_pending_datastores(universe_id))
	{
		datastore_names.push_back(QString::fromStdString(this_datastore_name));
	}
	progress = DownloadProgress{ datastore_names.size() };

	if (datastore_names.size() == 0)
	{
		progress.set_entry_total(pending_entries.size());
	}

	common_init();
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
		StandardDatastoreEntryName entry = pending_entries.back();
		pending_entries.pop_back();

		get_entry_details_request = std::make_shared<StandardDatastoreEntryGetDetailsRequest>(api_key, universe_id, entry.get_datastore_name(), entry.get_scope(), entry.get_key());
		get_entry_details_request->set_http_429_count(http_429_count);
		connect(get_entry_details_request.get(), &StandardDatastoreEntryGetDetailsRequest::received_http_429, this, &DatastoreBulkDownloadProgressWindow::handle_received_http_429);
		connect(get_entry_details_request.get(), &StandardDatastoreEntryGetDetailsRequest::status_error, this, &DatastoreBulkDownloadProgressWindow::handle_error_message);
		if (UserProfile::get().get_less_verbose_bulk_operations() == false)
		{
			connect(get_entry_details_request.get(), &StandardDatastoreEntryGetDetailsRequest::status_info, this, &DatastoreBulkDownloadProgressWindow::handle_status_message);
		}
		connect(get_entry_details_request.get(), &StandardDatastoreEntryGetDetailsRequest::success, this, &DatastoreBulkDownloadProgressWindow::handle_entry_response);
		get_entry_details_request->send_request();

		handle_status_message(QString{ "Downloading '%1'..." }.arg(entry.get_key()));
	}
	else
	{
		close_button->setText("Close");
		handle_status_message("Download complete");
	}
}

bool DatastoreBulkDownloadProgressWindow::is_retryable() const
{
	return DatastoreBulkOperationProgressWindow::is_retryable() || (get_entry_details_request && get_entry_details_request->req_status() == DataRequestStatus::Error);
}

bool DatastoreBulkDownloadProgressWindow::do_retry()
{
	if (is_retryable())
	{
		if (DatastoreBulkOperationProgressWindow::do_retry())
		{
			return true;
		}
		else if (get_entry_details_request && get_entry_details_request->req_status() == DataRequestStatus::Error)
		{
			get_entry_details_request->force_retry();
			return true;
		}
	}
	return false;
}

void DatastoreBulkDownloadProgressWindow::common_init()
{
	setWindowTitle("Download Progress");

	std::unique_ptr<SqliteDatastoreWrapper>& db_ref = this->db_wrapper;

#pragma warning ( push )
#pragma warning ( disable: 4458 )
	datastore_enumerate_step_callback = std::make_shared<std::function<void(long long, const std::string&, const std::string&)>>(
		[&db_ref](const long long universe_id, const std::string& datastore_name, const std::string& cursor) {
			db_ref->write_enumeration(universe_id, datastore_name, cursor);
		}
	);

	datastore_enumerate_done_callback = std::make_shared<std::function<void(long long, const std::string&)>>(
		[&db_ref](const long long universe_id, const std::string& datastore_name) {
			db_ref->delete_enumeration(universe_id, datastore_name);
		}
	);
#pragma warning ( pop )

	entry_found_callback = std::make_shared<std::function<void(const StandardDatastoreEntryName&)>>(
		[&db_ref](const StandardDatastoreEntryName& entry) {
			db_ref->write_pending(entry);
		}
	);
}

void DatastoreBulkDownloadProgressWindow::handle_entry_response()
{
	if (get_entry_details_request)
	{
		const std::optional<StandardDatastoreEntryFull> opt_details = get_entry_details_request->get_details();
		if (opt_details)
		{
			db_wrapper->write_details(*opt_details);
			db_wrapper->delete_pending(*opt_details);
		}
		else
		{
			// Entry was deleted
			const StandardDatastoreEntryName entry(get_entry_details_request->get_universe_id(), get_entry_details_request->get_datastore_name(), get_entry_details_request->get_key_name(), get_entry_details_request->get_scope());
			db_wrapper->write_deleted(entry);
			db_wrapper->delete_pending(entry);
		}
		progress.advance_entry_done();
		get_entry_details_request.reset();
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
		StandardDatastoreEntryName entry = pending_entries.back();
		pending_entries.pop_back();

		get_version_list_request = std::make_shared<StandardDatastoreEntryGetVersionListRequest>(api_key, universe_id, entry.get_datastore_name(), entry.get_scope(), entry.get_key());
		get_version_list_request->set_http_429_count(http_429_count);
		connect(get_version_list_request.get(), &StandardDatastoreEntryGetVersionListRequest::received_http_429, this, &DatastoreBulkUndeleteProgressWindow::handle_received_http_429);
		connect(get_version_list_request.get(), &StandardDatastoreEntryGetVersionListRequest::status_error, this, &DatastoreBulkUndeleteProgressWindow::handle_error_message);
		if (UserProfile::get().get_less_verbose_bulk_operations() == false)
		{
			connect(get_version_list_request.get(), &StandardDatastoreEntryGetVersionListRequest::status_info, this, &DatastoreBulkUndeleteProgressWindow::handle_status_message);
		}
		connect(get_version_list_request.get(), &StandardDatastoreEntryGetVersionListRequest::success, this, &DatastoreBulkUndeleteProgressWindow::handle_get_versions_response);
		get_version_list_request->send_request();

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
	if (get_version_list_request)
	{
		std::vector<StandardDatastoreEntryVersion> versions = get_version_list_request->get_versions();
		const QString datastore_name = get_version_list_request->get_datastore_name();
		const QString scope = get_version_list_request->get_scope();
		const QString key_name = get_version_list_request->get_key_name();
		get_version_list_request.reset();

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

		get_version_request = std::make_shared<StandardDatastoreEntryGetVersionRequest>(api_key, universe_id, datastore_name, scope, key_name, target_version->get_version());
		get_version_request->set_http_429_count(http_429_count);
		connect(get_version_request.get(), &StandardDatastoreEntryGetVersionRequest::received_http_429, this, &DatastoreBulkUndeleteProgressWindow::handle_received_http_429);
		connect(get_version_request.get(), &StandardDatastoreEntryGetVersionRequest::status_error, this, &DatastoreBulkUndeleteProgressWindow::handle_error_message);
		if (UserProfile::get().get_less_verbose_bulk_operations() == false)
		{
			connect(get_version_request.get(), &StandardDatastoreEntryGetVersionRequest::status_info, this, &DatastoreBulkUndeleteProgressWindow::handle_status_message);
		}
		connect(get_version_request.get(), &StandardDatastoreEntryGetVersionRequest::success, this, &DatastoreBulkUndeleteProgressWindow::handle_get_entry_version_response);
		get_version_request->send_request();
	}
}

void DatastoreBulkUndeleteProgressWindow::handle_get_entry_version_response()
{
	if (get_version_request)
	{
		const std::optional<StandardDatastoreEntryFull> opt_details = get_version_request->get_details();
		get_version_request.reset();

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

		post_entry_request = std::make_shared<StandardDatastoreEntryPostSetRequest>(api_key, universe_id, datastore_name, scope, key_name, userids, attributes, body);
		post_entry_request->set_http_429_count(http_429_count);
		connect(post_entry_request.get(), &StandardDatastoreEntryPostSetRequest::received_http_429, this, &DatastoreBulkUndeleteProgressWindow::handle_received_http_429);
		connect(post_entry_request.get(), &StandardDatastoreEntryPostSetRequest::status_error, this, &DatastoreBulkUndeleteProgressWindow::handle_error_message);
		if (UserProfile::get().get_less_verbose_bulk_operations() == false)
		{
			connect(post_entry_request.get(), &StandardDatastoreEntryPostSetRequest::status_info, this, &DatastoreBulkUndeleteProgressWindow::handle_status_message);
		}
		connect(post_entry_request.get(), &StandardDatastoreEntryPostSetRequest::success, this, &DatastoreBulkUndeleteProgressWindow::handle_post_entry_response);
		post_entry_request->send_request();
	}
}

void DatastoreBulkUndeleteProgressWindow::handle_post_entry_response()
{
	if (post_entry_request)
	{
		const bool success = post_entry_request->req_success();
		post_entry_request.reset();

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
