#pragma once

#include <cstddef>

#include <optional>
#include <vector>

#include <QList>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>

#include "api_response.h"
#include "util_enum.h"

class DataRequest : public QObject
{
	Q_OBJECT

public:
	virtual void send_request(std::optional<QString> cursor = std::nullopt);

	virtual QString get_title_string() const = 0;

	void set_http_429_count(size_t new_count) { http_429_count = new_count; }

signals:
	void request_complete();
	void status_error(QString message);
	void status_info(QString message);
	void received_http_429();

protected:
	DataRequest(QObject* parent, const QString& api_key);

	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) = 0;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) = 0;

	virtual void handle_http_404(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{});

	virtual QString get_send_message() const;

	void handle_reply_ready();
	void resend();

	int get_next_429_delay();

	QString api_key;

	std::optional<QNetworkRequest> pending_request;
	QNetworkReply* pending_reply = nullptr;

	HttpRequestType request_type = HttpRequestType::Get;
	std::optional<QString> post_body;

	size_t http_429_count = 0;
};

class DeleteStandardDatastoreEntryRequest : public DataRequest
{
public:
	DeleteStandardDatastoreEntryRequest(QObject* parent, const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name);

	virtual QString get_title_string() const override;

	const std::optional<bool>& is_delete_success() const { return delete_success; }

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual void handle_http_404(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString key_name;

	std::optional<bool> delete_success;
};

class GetStandardDatastoresDataRequest : public DataRequest
{
public:
	GetStandardDatastoresDataRequest(QObject* parent, const QString& api_key, long long universe_id);

	virtual QString get_title_string() const override;

	const std::vector<QString>& get_datastore_names() const { return datastore_names; }

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;

	long long universe_id;

	std::vector<QString> datastore_names;
};

class GetStandardDatastoreEntriesRequest : public DataRequest
{
public:
	GetStandardDatastoreEntriesRequest(QObject* parent, const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString prefix);

	virtual QString get_title_string() const override;

	void set_result_limit(size_t limit);

	const std::vector<StandardDatastoreEntry>& get_datastore_entries() const { return datastore_entries; }

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString prefix;

	std::optional<size_t> result_limit;

	std::vector<StandardDatastoreEntry> datastore_entries;
};

class GetStandardDatastoreEntryDetailsRequest : public DataRequest
{
public:
	GetStandardDatastoreEntryDetailsRequest(QObject* parent, const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name);

	virtual QString get_title_string() const override;

	const std::optional<DatastoreEntryWithDetails>& get_details() const { return details; }

protected:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual void handle_http_404(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString key_name;

	std::optional<DatastoreEntryWithDetails> details;
};

class GetStandardDatastoreEntryAtVersionRequest : public GetStandardDatastoreEntryDetailsRequest
{
public:
	GetStandardDatastoreEntryAtVersionRequest(QObject* parent, const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name, QString version);

protected:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) override;

	QString version;
};

class GetStandardDatastoreEntryVersionsRequest : public DataRequest
{
public:
	GetStandardDatastoreEntryVersionsRequest(QObject* parent, const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name);

	virtual QString get_title_string() const override;

	const QString& get_datastore_name() const { return datastore_name; }
	const QString& get_scope() const { return scope; }
	const QString& get_key_name() const { return key_name; }

	const std::vector<StandardDatastoreEntryVersion>& get_versions() const { return versions; }

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString key_name;

	std::vector<StandardDatastoreEntryVersion> versions;
};

class PostMessagingServiceMessageRequest : public DataRequest
{
public:
	PostMessagingServiceMessageRequest(QObject* parent, const QString& api_key, long long universe_id, QString topic, QString unencoded_message);

	virtual QString get_title_string() const override;

	bool get_success() const { return success; }

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString topic;

	bool success = false;
};

class PostStandardDatastoreEntryRequest : public DataRequest
{
public:
	PostStandardDatastoreEntryRequest(QObject* parent, const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name, std::optional<QString> userids, std::optional<QString> attributes, QString body);

	virtual QString get_title_string() const override;

	bool get_success() const { return success; }

	const QString& get_datastore_name() const { return datastore_name; }
	const QString& get_scope() const { return scope; }
	const QString& get_key_name() const { return key_name; }

private:
	virtual QNetworkRequest build_request(std::optional<QString> cursor = std::nullopt) override;
	virtual void handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers = QList<QNetworkReply::RawHeaderPair>{}) override;
	virtual QString get_send_message() const override;

	long long universe_id;
	QString datastore_name;
	QString scope;
	QString key_name;
	std::optional<QString> userids;
	std::optional<QString> attributes;
	QString body_md5;

	bool success = false;
};
