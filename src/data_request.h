#pragma once

#include <cstddef>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <QList>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>

#include "model_common.h"
#include "util_enum.h"

class QTimer;

enum class DataRequestStatus
{
	ReadyToBegin,
	Waiting,
	Success,
	Error,
};

class DataRequest : public QObject
{
	Q_OBJECT

public:
	DataRequestStatus req_status() const { return status; }
	bool req_success() const { return status == DataRequestStatus::Success; }

	void send_request(std::optional<QString> cursor = std::nullopt);
	void force_retry();

	virtual QString get_title_string() const = 0;

	void set_http_429_count(size_t new_count) { http_429_count = new_count; }

signals:
	void success();
	void status_error(QString message);
	void status_info(QString message);
	void received_http_429();

protected:
	DataRequest(const QString& api_key);

	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const = 0;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) = 0;

	virtual void handle_http_404(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{});

	virtual QString get_send_message() const;

	void handle_reply_ready();
	void handle_timeout();
	void resend();

	void timeout_begin();
	void timeout_end();

	int get_next_429_delay();

	void do_error(const QString& message);
	void do_success(const QString& message = "Success");

	DataRequestStatus status;

	QString api_key;

	std::optional<QString> pending_request_cursor;
	std::optional<QNetworkRequest> pending_request;
	QNetworkReply* pending_reply = nullptr;

	QTimer* request_timeout = nullptr;

	HttpRequestType request_type = HttpRequestType::Get;
	std::optional<QString> body_data;

	size_t http_429_count = 0;
};

class MessagingServicePostMessageRequest : public DataRequest
{
public:
	MessagingServicePostMessageRequest(const QString& api_key, long long universe_id, QString topic, QString unencoded_message);

	virtual QString get_title_string() const override;

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString topic;
};

class OrderedDatastoreEntryDeleteRequest : public DataRequest
{
public:
	OrderedDatastoreEntryDeleteRequest(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id);

	virtual QString get_title_string() const override;

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString entry_id;

	QString body_md5;
};

class OrderedDatastoreEntryGetDetailsRequest : public DataRequest
{
public:
	OrderedDatastoreEntryGetDetailsRequest(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name);

	virtual QString get_title_string() const override;

	std::optional<OrderedDatastoreEntryFull> get_details() const;

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString key_name;

	std::optional<OrderedDatastoreEntryFull> details;
};

class OrderedDatastoreEntryGetListRequest : public DataRequest
{
public:
	OrderedDatastoreEntryGetListRequest(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, bool ascending);

	virtual QString get_title_string() const override;

	void set_result_limit(size_t limit);

	const std::vector<OrderedDatastoreEntryFull>& get_entries() const { return entries; }

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	bool ascending;

	std::optional<size_t> result_limit;

	std::vector<OrderedDatastoreEntryFull> entries;
};

class OrderedDatastoreEntryPatchUpdateRequest : public DataRequest
{
public:
	OrderedDatastoreEntryPatchUpdateRequest(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id, long long new_value);

	virtual QString get_title_string() const override;

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual void handle_http_404(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString entry_id;
	long long new_value;

	QString body_md5;
};

class OrderedDatastoreEntryPostCreateRequest : public DataRequest
{
public:
	OrderedDatastoreEntryPostCreateRequest(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id, long long value);

	virtual QString get_title_string() const override;

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString entry_id;
	long long value;

	QString body_md5;
};

class OrderedDatastorePostIncrementRequest : public DataRequest
{
public:
	OrderedDatastorePostIncrementRequest(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id, long long increment_by);

	virtual QString get_title_string() const override;

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString entry_id;
	long long increment_by;

	QString body_md5;
};

class StandardDatastoreEntryDeleteRequest : public DataRequest
{
public:
	StandardDatastoreEntryDeleteRequest(const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name);

	virtual QString get_title_string() const override;

	std::optional<bool> is_delete_success() const;

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual void handle_http_404(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString key_name;

	bool delete_success = false;
};

class StandardDatastoreEntryGetDetailsRequest : public DataRequest
{
public:
	StandardDatastoreEntryGetDetailsRequest(const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name);

	virtual QString get_title_string() const override;

	std::optional<StandardDatastoreEntryFull> get_details() const;

	long long get_universe_id() const { return universe_id; }
	QString get_datastore_name() const { return datastore_name; }
	QString get_scope() const { return scope; }
	QString get_key_name() const { return key_name; }

protected:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual void handle_http_404(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString key_name;

	std::optional<StandardDatastoreEntryFull> details;
};

class StandardDatastoreEntryGetListRequest : public DataRequest
{
public:
	StandardDatastoreEntryGetListRequest(const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString prefix, std::optional<QString> initial_cursor = std::nullopt);

	virtual QString get_title_string() const override;

	void set_result_limit(size_t limit);

	void set_enumerate_step_callback(std::weak_ptr<std::function<void(long long, const std::string&, const std::string&)>> callback) { enumerate_step_callback = callback; }
	void set_enumerate_done_callback(std::weak_ptr<std::function<void(long long, const std::string&)>> callback) { enumerate_done_callback = callback; }
	void set_entry_found_callback(std::weak_ptr<std::function<void(const StandardDatastoreEntryName&)>> callback) { entry_found_callback = callback; }

	const std::vector<StandardDatastoreEntryName>& get_datastore_entries() const { return datastore_entries; }
	std::vector<StandardDatastoreEntryName>&& get_datastore_entries_rvalue() { return std::move(datastore_entries); }

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString prefix;

	std::optional<QString> initial_cursor;

	std::optional<size_t> result_limit;

	std::vector<StandardDatastoreEntryName> datastore_entries;

	std::weak_ptr<std::function<void(long long, const std::string&, const std::string&)>> enumerate_step_callback;
	std::weak_ptr<std::function<void(long long, const std::string&)>> enumerate_done_callback;
	std::weak_ptr<std::function<void(const StandardDatastoreEntryName&)>> entry_found_callback;
};

class StandardDatastoreEntryGetVersionListRequest : public DataRequest
{
public:
	StandardDatastoreEntryGetVersionListRequest(const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name);

	virtual QString get_title_string() const override;

	const QString& get_datastore_name() const { return datastore_name; }
	const QString& get_scope() const { return scope; }
	const QString& get_key_name() const { return key_name; }

	const std::vector<StandardDatastoreEntryVersion>& get_versions() { return versions; }

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString key_name;

	std::vector<StandardDatastoreEntryVersion> versions;
};

class StandardDatastoreEntryGetVersionRequest : public StandardDatastoreEntryGetDetailsRequest
{
public:
	StandardDatastoreEntryGetVersionRequest(const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name, QString version);

protected:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;

	QString version;
};

class StandardDatastoreEntryPostSetRequest : public DataRequest
{
public:
	StandardDatastoreEntryPostSetRequest(const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name, std::optional<QString> userids, std::optional<QString> attributes, QString body);

	virtual QString get_title_string() const override;

	const QString& get_datastore_name() const { return datastore_name; }
	const QString& get_scope() const { return scope; }
	const QString& get_key_name() const { return key_name; }

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString key_name;
	std::optional<QString> userids;
	std::optional<QString> attributes;

	QString body_md5;
};


class StandardDatastoreGetListRequest : public DataRequest
{
public:
	StandardDatastoreGetListRequest(const QString& api_key, long long universe_id);

	virtual QString get_title_string() const override;

	const std::vector<QString>& get_datastore_names() const { return datastore_names; }

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;

	long long universe_id;

	std::vector<QString> datastore_names;
};

class UniverseGetDetailsRequest : public DataRequest
{
public:
	UniverseGetDetailsRequest(const QString& api_key, long long universe_id);

	virtual QString get_title_string() const override;

	const std::optional<QString>& get_display_name() const { return display_name; }

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) const override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;

	long long universe_id;

	std::optional<QString> display_name;
};
