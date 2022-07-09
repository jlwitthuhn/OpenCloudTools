#include "data_request.h"

#include <algorithm>

#include <QByteArray>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTimer>
#include <QVariant>

#include "api_response.h"
#include "http_req_builder.h"
#include "http_wrangler.h"
#include "roblox_time.h"
#include "util_enum.h"

void DataRequest::send_request(std::optional<QString> cursor)
{
	if (pending_request)
	{
		pending_request = std::nullopt;
	}
	if (pending_reply)
	{
		pending_reply->deleteLater();
		pending_reply = nullptr;
	}

	pending_request = build_request(cursor);
	pending_reply = HttpWrangler::send(request_type, *pending_request, post_body);

	connect(pending_reply, &QNetworkReply::finished, this, &DataRequest::handle_reply_ready);

	emit status_info(get_send_message());
}

DataRequest::DataRequest(QObject* const parent, const QString& api_key) : QObject{ parent }, api_key{ api_key }
{

}

void DataRequest::handle_http_404(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	emit status_error(QString{ "Received HTTP 404, aborting" });
	emit status_error(body);
}

QString DataRequest::get_send_message() const
{
	return QString{ "Sending request..." };
}

void DataRequest::handle_reply_ready()
{
	QString status = pending_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
	QString reply_body = pending_reply->readAll();
	QList<QNetworkReply::RawHeaderPair> headers = pending_reply->rawHeaderPairs();
	RobloxTime::update_time_from_headers(headers);

	pending_reply->deleteLater();
	pending_reply = nullptr;

	if (status == "200") // OK
	{
		handle_http_200(reply_body, headers);
	}
	else if (status == "204") // No content, body will be empty
	{
		handle_http_200(reply_body, headers);
	}
	else if (status == "404") // Not found
	{
		handle_http_404(reply_body, headers);
	}
	else if (status == "429") // Too many requests
	{
		emit status_info(QString{ "Received HTTP 429, briefly pausing..." });
		emit received_http_429();

		QTimer* timer = new QTimer(this);
		timer->setSingleShot(true);
		connect(timer, &QTimer::timeout, this, &DataRequest::resend);
		timer->start(get_next_429_delay());
	}
	else if (status == "500") // Internal server error
	{
		emit status_info(QString{ "Received HTTP 500 Internal Server Error, retrying..." });
		resend();
	}
	else if (status == "502") // Bad gateway
	{
		emit status_info(QString{ "Received HTTP 502 Bad Gateway, retrying..." });
		resend();
	}
	else if (status == "504") // Gateway timeout
	{
		emit status_info(QString{ "Received HTTP 504 Gateway Timeout, retrying..." });
		resend();
	}
	else
	{
		emit status_error(QString{ "Received HTTP %1, aborting" }.arg(status));
		emit status_error(reply_body);
	}
}

void DataRequest::resend()
{
	if (pending_request)
	{
		emit status_info("Resending...");
		pending_reply = HttpWrangler::send(request_type, *pending_request, post_body);
		connect(pending_reply, &QNetworkReply::finished, this, &DataRequest::handle_reply_ready);
	}
}

int DataRequest::get_next_429_delay()
{
	http_429_count = http_429_count + 1;
	if (http_429_count < 2)
	{
		return 2500;
	}
	else if (http_429_count < 3)
	{
		return 5000;
	}
	else
	{
		return 10000;
	}
}

DeleteStandardDatastoreEntryRequest::DeleteStandardDatastoreEntryRequest(QObject* parent, const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name) :
	DataRequest{ parent, api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, key_name{ key_name }
{
	request_type = HttpRequestType::Delete;
}

QString DeleteStandardDatastoreEntryRequest::get_title_string() const
{
	return "Deleting entry...";
}

QNetworkRequest DeleteStandardDatastoreEntryRequest::build_request(std::optional<QString>)
{
	return HttpRequestBuilder::delete_standard_datastore_entry(api_key, universe_id, datastore_name, scope, key_name);
}

void DeleteStandardDatastoreEntryRequest::handle_http_200(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	delete_success = true;
	emit status_info(QString{ "Complete" });
	emit request_complete();
}

void DeleteStandardDatastoreEntryRequest::handle_http_404(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	delete_success = false;
	emit status_info(QString{ "Entry already deleted" });
	emit request_complete();
}

QString DeleteStandardDatastoreEntryRequest::get_send_message() const
{
	return QString{ "Deleting entry '%1'..." }.arg(key_name);
}

GetStandardDatastoresDataRequest::GetStandardDatastoresDataRequest(QObject* parent, const QString& api_key, const long long universe_id) : DataRequest{ parent, api_key }, universe_id{ universe_id }
{

}

QString GetStandardDatastoresDataRequest::get_title_string() const
{
	return "Fetching datastores...";
}

QNetworkRequest GetStandardDatastoresDataRequest::build_request(std::optional<QString> cursor)
{
	return HttpRequestBuilder::get_standard_datastores(api_key, universe_id, cursor);
}

void GetStandardDatastoresDataRequest::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	std::optional<GetStandardDatastoresResponse> response = GetStandardDatastoresResponse::fromJson(body);
	if (response)
	{
		for (QString this_name : response->get_datastores_vec())
		{
			datastore_names.push_back(this_name);
		}

		emit status_info(QString{ "Received %1 datastore name(s), %2 total" }.arg(QString::number(response->get_datastores_vec().size()), QString::number(datastore_names.size())));

		std::optional<QString> cursor{ response->get_cursor() };
		if (cursor && cursor->size() > 0)
		{
			send_request(cursor);
		}
		else
		{
			emit status_info(QString{ "Complete" });
			emit request_complete();
		}
	}
	else
	{
		emit status_info(QString{ "Complete" });
		emit request_complete();
	}
}

GetStandardDatastoreEntriesRequest::GetStandardDatastoreEntriesRequest(QObject* parent, const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString prefix) :
	DataRequest{ parent, api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, prefix{ prefix }
{

}

QString GetStandardDatastoreEntriesRequest::get_title_string() const
{
	return "Fetching datastore entries...";
}

void GetStandardDatastoreEntriesRequest::set_result_limit(const size_t limit)
{
	result_limit = limit;
}

QNetworkRequest GetStandardDatastoreEntriesRequest::build_request(std::optional<QString> cursor)
{
	return HttpRequestBuilder::get_standard_datastore_entries(api_key, universe_id, datastore_name, scope, prefix, cursor);
}

void GetStandardDatastoreEntriesRequest::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	std::optional<GetStandardDatastoreEntriesResponse> response = GetStandardDatastoreEntriesResponse::fromJson(body, universe_id, datastore_name);
	if (response)
	{
		for (StandardDatastoreEntry this_entry : response->get_entries())
		{
			datastore_entries.push_back(this_entry);
		}

		emit status_info(QString{ "Received %1 entries, %2 total" }.arg(QString::number(response->get_entries().size()), QString::number(datastore_entries.size())));

		const bool limit_reached = result_limit && datastore_entries.size() >= *result_limit;

		std::optional<QString> cursor{ response->get_cursor() };
		if (cursor && cursor->size() > 0 && limit_reached == false)
		{
			send_request(cursor);
		}
		else
		{
			emit status_info(QString{ "Complete" });
			emit request_complete();
		}
	}
	else
	{
		emit status_info(QString{ "Complete" });
		emit request_complete();
	}
}

GetStandardDatastoreEntryDetailsRequest::GetStandardDatastoreEntryDetailsRequest(QObject* parent, const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name) :
	DataRequest{ parent, api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, key_name{ key_name }
{

}

QString GetStandardDatastoreEntryDetailsRequest::get_title_string() const
{
	return "Fetching datastore entry details...";
}

QNetworkRequest GetStandardDatastoreEntryDetailsRequest::build_request(std::optional<QString>)
{
	return HttpRequestBuilder::get_standard_datastore_entry_details(api_key, universe_id, datastore_name, scope, key_name);
}

void GetStandardDatastoreEntryDetailsRequest::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers)
{
	std::optional<QString> version;
	std::optional<QString> userids;
	std::optional<QString> attributes;
	for (QNetworkReply::RawHeaderPair this_header : headers)
	{
		QString header_key{ this_header.first };
		if (header_key == "roblox-entry-version")
		{
			version = QString{ this_header.second };
		}
		if (header_key == "roblox-entry-userids")
		{
			userids = QString{ this_header.second };
		}
		if (header_key == "roblox-entry-attributes")
		{
			attributes = QString{ this_header.second };
		}
	}

	if (!version)
	{
		emit status_error("Error, response did not contain version");
		return;
	}

	std::optional<GetStandardDatastoreEntryDetailsResponse> response = GetStandardDatastoreEntryDetailsResponse::from(universe_id, datastore_name, scope, key_name, *version, userids, attributes, body);
	if (response)
	{
		details = response->get_details();
	}
	else
	{
		emit status_error(QString{ "Received invalid response, aborting" });
		return;
	}

	emit status_info(QString{ "Complete" });
	emit request_complete();
}

void GetStandardDatastoreEntryDetailsRequest::handle_http_404(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	details = std::nullopt;

	emit status_info(QString{ "Complete" });
	emit request_complete();
}

QString GetStandardDatastoreEntryDetailsRequest::get_send_message() const
{
	return QString{ "Fetching entry '%1'..." }.arg(key_name);
}

GetStandardDatastoreEntryAtVersionRequest::GetStandardDatastoreEntryAtVersionRequest(QObject* parent, const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name, QString version) :
	GetStandardDatastoreEntryDetailsRequest{ parent, api_key, universe_id, datastore_name, scope, key_name }, version{ version }
{

}

QNetworkRequest GetStandardDatastoreEntryAtVersionRequest::build_request(std::optional<QString>)
{
	return HttpRequestBuilder::get_standard_datastore_entry_version_details(api_key, universe_id, datastore_name, scope, key_name, version);
}

GetStandardDatastoreEntryVersionsRequest::GetStandardDatastoreEntryVersionsRequest(QObject* parent, const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name) :
	DataRequest{ parent, api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, key_name{ key_name }
{

}

QString GetStandardDatastoreEntryVersionsRequest::get_title_string() const
{
	return "Fetching datastore entry versions...";
}

QNetworkRequest GetStandardDatastoreEntryVersionsRequest::build_request(std::optional<QString> cursor)
{
	return HttpRequestBuilder::get_standard_datastore_entry_versions(api_key, universe_id, datastore_name, scope, key_name, cursor);
}

void GetStandardDatastoreEntryVersionsRequest::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	std::optional<GetStandardDatastoreEntryVersionsResponse> response = GetStandardDatastoreEntryVersionsResponse::from(body);
	if (response)
	{
		for (StandardDatastoreEntryVersion this_version : response->get_versions())
		{
			versions.push_back(this_version);
		}

		emit status_info(QString{ "Received %1 entries, %2 total" }.arg(QString::number(response->get_versions().size()), QString::number(versions.size())));

		std::optional<QString> cursor{ response->get_cursor() };
		if (cursor && cursor->size() > 0)
		{
			send_request(cursor);
		}
		else
		{
			emit status_info(QString{ "Complete" });
			emit request_complete();
		}
	}
	else
	{
		emit status_info(QString{ "Complete" });
		emit request_complete();
	}
}

QString GetStandardDatastoreEntryVersionsRequest::get_send_message() const
{
	return QString{ "Fetching versions for '%1'..." }.arg(key_name);
}

PostMessagingServiceMessageRequest::PostMessagingServiceMessageRequest(QObject* parent, const QString& api_key, long long universe_id, QString topic, DatastoreEntryType entry_type, QString unencoded_message)
	: DataRequest{ parent, api_key }, universe_id{ universe_id }, topic{ topic }
{
	request_type = HttpRequestType::Post;

	QJsonObject send_object;
	switch (entry_type)
	{
		case DatastoreEntryType::Json:
		{
			const QJsonDocument doc = QJsonDocument::fromJson(unencoded_message.toUtf8());
			if (doc.isNull())
			{
				// Error, probably should do something
				return;
			}
			send_object.insert("message", doc.object());
			break;
		}
		case DatastoreEntryType::String:
		{
			send_object.insert("message", unencoded_message);
			break;
		}
		case DatastoreEntryType::Number:
		{
			send_object.insert("message", unencoded_message.toDouble());
			break;
		}
		case DatastoreEntryType::Bool:
		{
			if (unencoded_message.toLower() == "true")
			{
				send_object.insert("message", true);
			}
			else if (unencoded_message.toLower() == "false")
			{
				send_object.insert("message", false);
			}
			break;
		}
		case DatastoreEntryType::Error:
		{
			// Do nothing
			break;
		}
	}

	const QJsonDocument doc{ send_object };
	post_body = doc.toJson(QJsonDocument::Compact);
}

QString PostMessagingServiceMessageRequest::get_title_string() const
{
	return "Setting message...";
}

QNetworkRequest PostMessagingServiceMessageRequest::build_request(std::optional<QString>)
{
	return HttpRequestBuilder::post_messaging_service_message(api_key, universe_id, topic);
}

void PostMessagingServiceMessageRequest::handle_http_200(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	success = true;
	emit status_info(QString{ "Sent" });
	emit request_complete();
}

QString PostMessagingServiceMessageRequest::get_send_message() const
{
	return QString{ "Sending message to '%1'..." }.arg(topic);
}

PostStandardDatastoreEntryRequest::PostStandardDatastoreEntryRequest(QObject* parent, const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name, std::optional<QString> userids, std::optional<QString> attributes, QString body)
	: DataRequest{ parent, api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, key_name{ key_name }, userids{ userids }, attributes{ attributes }
{
	request_type = HttpRequestType::Post;
	post_body = body;
	body_md5 = QCryptographicHash::hash(body.toUtf8(), QCryptographicHash::Algorithm::Md5).toBase64();
}

QString PostStandardDatastoreEntryRequest::get_title_string() const
{
	return "Setting entry...";
}

QNetworkRequest PostStandardDatastoreEntryRequest::build_request(std::optional<QString>)
{
	return HttpRequestBuilder::post_standard_datastore_entry(api_key, universe_id, datastore_name, scope, key_name, body_md5, userids, attributes);
}

void PostStandardDatastoreEntryRequest::handle_http_200(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	success = true;
	emit status_info(QString{ "Complete" });
	emit request_complete();
}

QString PostStandardDatastoreEntryRequest::get_send_message() const
{
	return QString{ "Setting key '%1'..." }.arg(key_name);
}
