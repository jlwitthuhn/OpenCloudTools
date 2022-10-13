#pragma once

#include <cstddef>

#include <memory>
#include <optional>
#include <vector>

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QWidget>

#include "api_response.h"
#include "sqlite_wrapper.h"

class QLabel;
class QProgressBar;
class QPushButton;
class QTextEdit;

class DeleteStandardDatastoreEntryRequest;
class GetStandardDatastoreEntriesRequest;
class GetStandardDatastoreEntryAtVersionRequest;
class GetStandardDatastoreEntryDetailsRequest;
class GetStandardDatastoreEntryVersionsRequest;
class PostStandardDatastoreEntryRequest;

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

	void update_ui();

	void send_next_enumerate_keys_request();

	void handle_error_message(QString message);
	void handle_status_message(QString message);
	void handle_enumerate_keys_response();
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

	size_t http_429_count = 0;

	DownloadProgress progress;
	std::vector<QString> datastore_names;

	std::vector<StandardDatastoreEntry> pending_entries;

	std::shared_ptr<GetStandardDatastoreEntriesRequest> get_entries_request;

	QLabel* progress_label = nullptr;
	QProgressBar* progress_bar = nullptr;

	QTextEdit* text_box = nullptr;

	QPushButton* close_button = nullptr;
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

	std::shared_ptr<GetStandardDatastoreEntryDetailsRequest> get_entry_request;
	std::shared_ptr<PostStandardDatastoreEntryRequest> post_entry_request;
	std::shared_ptr<DeleteStandardDatastoreEntryRequest> delete_entry_request;
};

class DatastoreBulkDownloadProgressWindow : public DatastoreBulkOperationProgressWindow
{
	Q_OBJECT
public:
	DatastoreBulkDownloadProgressWindow(QWidget* parent, const QString& api_key, long long universe_id, const QString& scope, const QString& key_prefix, std::vector<QString> datastore_names, std::unique_ptr<SqliteDatastoreWriter> writer);

private:
	virtual QString progress_label_done() const override;
	virtual QString progress_label_working(size_t total) const override;

	virtual void send_next_entry_request() override;
	void handle_entry_response();

	std::unique_ptr<SqliteDatastoreWriter> writer;

	std::shared_ptr<GetStandardDatastoreEntryDetailsRequest> get_entry_details_request;
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

	std::shared_ptr<GetStandardDatastoreEntryVersionsRequest> get_versions_request;
	std::shared_ptr<GetStandardDatastoreEntryAtVersionRequest> get_entry_at_version_request;
	std::shared_ptr<PostStandardDatastoreEntryRequest> post_entry_request;

	size_t entries_restored = 0;
	size_t entries_not_deleted = 0;
	size_t entries_no_old_version = 0;
	size_t entries_not_in_time_range = 0;
	size_t entries_errored = 0;
};
