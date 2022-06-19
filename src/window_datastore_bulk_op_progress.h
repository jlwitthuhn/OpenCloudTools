#pragma once

#include <cstddef>

#include <memory>
#include <optional>
#include <vector>

#include <QObject>
#include <QString>
#include <QWidget>

#include "api_response.h"
#include "sqlite_wrapper.h"
#include "util_enum.h"

class QLabel;
class QProgressBar;
class QPushButton;
class QTextEdit;

class DeleteStandardDatastoreEntryRequest;
class GetStandardDatastoreEntriesRequest;
class GetStandardDatastoreEntryRequest;

class DatastoreBulkOperationProgressWindow : public QWidget
{
	Q_OBJECT
protected:
	DatastoreBulkOperationProgressWindow(QWidget* parent, const QString& api_key, long long universe_id, const QString& scope, const QString& key_prefix, std::vector<QString> datastore_names);

	virtual QString progress_label_done() const = 0;
	virtual QString progress_label_working(size_t total) const = 0;

	virtual void send_next_entry_request() = 0;
	virtual void handle_entry_response() = 0;

	void update_ui();

	void send_next_enumerate_keys_request();

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
	QString scope;
	QString key_prefix;

	size_t http_429_count = 0;

	DownloadProgress progress;
	std::vector<QString> datastore_names;

	std::vector<StandardDatastoreEntry> pending_entries;

	GetStandardDatastoreEntriesRequest* get_entries_request = nullptr;

	QLabel* progress_label = nullptr;
	QProgressBar* progress_bar = nullptr;

	QTextEdit* text_box = nullptr;

	QPushButton* close_button = nullptr;
};

class DatastoreBulkDeleteProgressWindow: public DatastoreBulkOperationProgressWindow
{
	Q_OBJECT
public:
	DatastoreBulkDeleteProgressWindow(QWidget* parent, const QString& api_key, long long universe_id, const QString& scope, const QString& key_prefix, std::vector<QString> datastore_names);

private:
	virtual QString progress_label_done() const override;
	virtual QString progress_label_working(size_t total) const override;

	virtual void send_next_entry_request() override;
	virtual void handle_entry_response() override;

	DeleteStandardDatastoreEntryRequest* delete_entry_request = nullptr;
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
	virtual void handle_entry_response() override;

	std::unique_ptr<SqliteDatastoreWriter> writer;

	GetStandardDatastoreEntryRequest* get_entry_details_request = nullptr;
};
