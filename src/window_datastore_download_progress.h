#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <QString>
#include <QWidget>

#include "api_response.h"
#include "data_request.h"
#include "sqlite_wrapper.h"

class QLabel;
class QProgressBar;
class QPushButton;
class QTextEdit;

class DownloadDatastoreProgressWindow : public QWidget
{
	Q_OBJECT
public:
	DownloadDatastoreProgressWindow(QWidget* parent, const QString& api_key, long long universe_id, std::vector<QString> datastore_names, std::unique_ptr<SqliteDatastoreWriter> writer);

private:
	void update_ui();

	void send_list_keys_request();
	void send_next_details_request();

	void handle_status_message(QString message);
	void handle_enumerate_entries_complete();
	void handle_get_entry_details_complete();

	class DownloadProgress
	{
	public:
		DownloadProgress(size_t datastore_total);

		bool is_done() const;

		size_t get_current_datastore_index() const;
		size_t get_overall_progress() const;
		std::optional<size_t> get_local_progress() const;

		void advance_datastore_done();
		void advance_entry_done();

		void clear_entry_done();
		void set_entry_total(size_t total);
		void clear_entry_total();

		static constexpr size_t MAXIMUM = 10000;

	private:
		size_t datastore_done = 0;
		size_t datastore_total = 1;
		size_t entry_done = 0;
		std::optional<size_t> entry_total = std::nullopt;
	};

	QString api_key;
	long long universe_id;

	DownloadProgress progress;
	std::unique_ptr<SqliteDatastoreWriter> writer;
	std::vector<QString> datastore_names;

	std::vector<StandardDatastoreEntry> pending_entries;

	GetStandardDatastoreEntriesRequest* get_entries_request = nullptr;
	GetStandardDatastoreEntryRequest* get_entry_details_request = nullptr;

	QLabel* label_overall = nullptr;
	QProgressBar* bar_overall = nullptr;

	QLabel* label_entry = nullptr;
	QProgressBar* bar_entry = nullptr;

	QTextEdit* text_box = nullptr;

	QPushButton* close_button = nullptr;
};
