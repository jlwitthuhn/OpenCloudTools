#pragma once

#include <cstddef>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QWidget>

#include "model_common.h"
#include "sqlite_wrapper.h"

class QLabel;
class QProgressBar;
class QPushButton;


class StandardDatastoreEntryDeleteRequest;
class StandardDatastoreEntryGetDetailsRequest;
class StandardDatastoreEntryGetListRequest;
class StandardDatastoreEntryGetVersionRequest;
class StandardDatastoreEntryGetVersionListRequest;
class StandardDatastoreEntryPostSetRequest;
class TextLogWidget;

class DatastoreBulkOperationProgressWindow : public QWidget
{
	Q_OBJECT

public:
	void start();

protected:
	DatastoreBulkOperationProgressWindow(QWidget* parent, const QString& api_key, long long universe_id, const QString& find_scope, const QString& find_key_prefix, std::vector<QString> datastore_names);

	virtual QString progress_label_done() const = 0;
	virtual QString progress_label_working(size_t total) const = 0;

	virtual void send_next_entry_request() = 0;

	virtual bool is_retryable() const;
	virtual bool do_retry();

	void update_ui();

	void send_next_enumerate_keys_request();

	void handle_clicked_retry();
	void handle_error_message(QString message);
	void handle_status_message(QString message);
	void handle_enumerate_keys_success();
	void handle_received_http_429();

	class DownloadProgress
	{
	public:
		DownloadProgress(size_t datastore_total);

		bool is_enumerating() const;
		bool is_done() const;

		size_t get_current_datastore_index() const;
		size_t get_current_entry_index() const;
		size_t get_progress() const;

		void advance_datastore_done();
		void advance_entry_done();

		void set_entry_total(size_t total);
		std::optional<size_t> get_entry_total() const;

		static constexpr size_t MAXIMUM = 10000;

	private:
		size_t datastore_done = 0;
		size_t datastore_total = 1;
		size_t entry_done = 0;
		std::optional<size_t> entry_total = std::nullopt;
	};

	QString api_key;
	long long universe_id;
	QString find_scope;
	QString find_key_prefix;

	std::optional<QString> initial_cursor;

	size_t http_429_count = 0;

	DownloadProgress progress;
	std::vector<QString> datastore_names;

	std::vector<StandardDatastoreEntryName> pending_entries;

	std::shared_ptr<StandardDatastoreEntryGetListRequest> enumerate_entries_request;

	QLabel* progress_label = nullptr;
	QProgressBar* progress_bar = nullptr;

	TextLogWidget* text_log = nullptr;

	QPushButton* retry_button = nullptr;
	QPushButton* close_button = nullptr;

	std::shared_ptr<std::function<void(long long, const std::string&, const std::string&)>> datastore_enumerate_step_callback;
	std::shared_ptr<std::function<void(long long, const std::string&)>> datastore_enumerate_done_callback;
	std::shared_ptr<std::function<void(const StandardDatastoreEntryName&)>> entry_found_callback;
};

class DatastoreBulkDeleteProgressWindow: public DatastoreBulkOperationProgressWindow
{
	Q_OBJECT
public:
	DatastoreBulkDeleteProgressWindow(
		QWidget* parent,
		const QString& api_key,
		long long universe_id,
		const QString& scope,
		const QString& key_prefix,
		std::vector<QString> datastore_names,
		bool confirm_count_before_delete,
		bool rewrite_before_delete,
		bool hide_datastores_when_done
	);

private:
	virtual QString progress_label_done() const override;
	virtual QString progress_label_working(size_t total) const override;

	virtual void send_next_entry_request() override;
	void handle_get_entry_response();
	void handle_post_entry_response();
	void handle_delete_entry_response();

	QString get_summary() const;

	bool confirm_count_before_delete;
	bool rewrite_before_delete;
	bool hide_datastores_when_done;
	bool first_delete_request_sent = false;

	size_t entries_deleted = 0;
	size_t entries_already_deleted = 0;

	std::shared_ptr<StandardDatastoreEntryGetDetailsRequest> get_entry_request;
	std::shared_ptr<StandardDatastoreEntryPostSetRequest> post_entry_request;
	std::shared_ptr<StandardDatastoreEntryDeleteRequest> delete_entry_request;
};

class DatastoreBulkDownloadProgressWindow : public DatastoreBulkOperationProgressWindow
{
	Q_OBJECT
public:
	DatastoreBulkDownloadProgressWindow(QWidget* parent, const QString& api_key, long long universe_id, const QString& scope, const QString& key_prefix, std::vector<QString> datastore_names, std::unique_ptr<SqliteDatastoreWrapper> db_wrapper);
	DatastoreBulkDownloadProgressWindow(QWidget* parent, const QString& api_key, long long universe_id, std::unique_ptr<SqliteDatastoreWrapper> db_wrapper);

private:
	virtual QString progress_label_done() const override;
	virtual QString progress_label_working(size_t total) const override;

	virtual void send_next_entry_request() override;

	virtual bool is_retryable() const override;
	virtual bool do_retry() override;

	void common_init();
	void handle_entry_response();

	std::unique_ptr<SqliteDatastoreWrapper> db_wrapper;

	std::shared_ptr<StandardDatastoreEntryGetDetailsRequest> get_entry_details_request;
};

class DatastoreBulkUndeleteProgressWindow : public DatastoreBulkOperationProgressWindow
{
	Q_OBJECT
public:
	DatastoreBulkUndeleteProgressWindow(QWidget* parent, const QString& api_key, long long universe_id, const QString& scope, const QString& key_prefix, std::vector<QString> datastore_names, std::optional<QDateTime> undelete_after);

private:
	virtual QString progress_label_done() const override;
	virtual QString progress_label_working(size_t total) const override;

	virtual void send_next_entry_request() override;
	void handle_get_versions_response();
	void handle_get_entry_version_response();
	void handle_post_entry_response();

	std::optional<QDateTime> undelete_after;

	std::shared_ptr<StandardDatastoreEntryGetVersionListRequest> get_version_list_request;
	std::shared_ptr<StandardDatastoreEntryGetVersionRequest> get_version_request;
	std::shared_ptr<StandardDatastoreEntryPostSetRequest> post_entry_request;

	size_t entries_restored = 0;
	size_t entries_not_deleted = 0;
	size_t entries_no_old_version = 0;
	size_t entries_not_in_time_range = 0;
	size_t entries_errored = 0;
};
