#include "data_request.h"

#include <algorithm>

#include <QByteArray>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaEnum>
#include <QNetworkReply>
#include <QTimer>
#include <QVariant>

#include "http_req_builder.h"
#include "http_wrangler.h"
#include "model_api_opencloud.h"
#include "roblox_time.h"
#include "util_enum.h"

void DataRequest::send_request(std::optional<QString> cursor)
{
	if (status != DataRequestStatus::ReadyToBegin && status != DataRequestStatus::Waiting)
	{
		emit status_error("Failed to send request, aborting");
		return;
	}

	if (pending_reply)
	{
		pending_reply->deleteLater();
		pending_reply = nullptr;
	}
	if (pending_request)
	{
		pending_request = std::nullopt;
	}

	pending_request_cursor = cursor;
	pending_request = build_request(cursor);
	pending_reply = HttpWrangler::send(request_type, *pending_request, post_body);

	connect(pending_reply, &QNetworkReply::finished, this, &DataRequest::handle_reply_ready);

	timeout_begin();

	status = DataRequestStatus::Waiting;

	emit status_info(get_send_message());
}

void DataRequest::force_retry()
{
	if (status == DataRequestStatus::Error && pending_request)
	{
		status = DataRequestStatus::Waiting;
		send_request(pending_request_cursor);
	}
}

DataRequest::DataRequest(const QString& api_key) : QObject{ nullptr }, status{ DataRequestStatus::ReadyToBegin }, api_key { api_key }
{

}

void DataRequest::handle_http_404(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	do_error("Received HTTP 404, aborting");
	emit status_info(body);
}

QString DataRequest::get_send_message() const
{
	return QString{ "Sending request..." };
}

void DataRequest::handle_reply_ready()
{
	timeout_end();

	if (status != DataRequestStatus::Waiting)
	{
		do_error("Received reply at unexpected time, aborting");
		return;
	}

	QNetworkReply::NetworkError error = pending_reply->error();
	QString status = pending_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
	QString reply_body = pending_reply->readAll();
	QList<QNetworkReply::RawHeaderPair> headers = pending_reply->rawHeaderPairs();
	RobloxTime::update_time_from_headers(headers);

	// Prevent timeout from firing
	pending_reply->disconnect(this);

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
	else if (status == "")
	{
		QMetaEnum meta_enum = QMetaEnum::fromType<QNetworkReply::NetworkError>();
		do_error( QString{ "Network error %1, aborting" }.arg( meta_enum.valueToKey( static_cast<int>(error) ) ) );
	}
	else
	{
		do_error(QString{ "Received HTTP %1, aborting" }.arg(status));
		if (reply_body != "")
		{
			emit status_info(reply_body);
		}
	}
}

void DataRequest::handle_timeout()
{
	timeout_end();

	pending_reply->disconnect(this);
	pending_reply->deleteLater();
	pending_reply = nullptr;

	emit status_info(QString{ "Request timed out, retrying..." });
	resend();
}

void DataRequest::resend()
{
	if (pending_request)
	{
		emit status_info("Resending...");
		pending_reply = HttpWrangler::send(request_type, *pending_request, post_body);
		connect(pending_reply, &QNetworkReply::finished, this, &DataRequest::handle_reply_ready);
		timeout_begin();
	}
}

void DataRequest::timeout_begin()
{
	timeout_end();

	request_timeout = new QTimer{ this };
	request_timeout->setSingleShot(true);
	request_timeout->start(20000);
	connect(request_timeout, &QTimer::timeout, this, &DataRequest::handle_timeout);
}

void DataRequest::timeout_end()
{
	if (request_timeout)
	{
		request_timeout->stop();
		request_timeout->disconnect(this);
		request_timeout->deleteLater();
		request_timeout = nullptr;
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

void DataRequest::do_error(const QString& message)
{
	status = DataRequestStatus::Error;
	emit status_error(message);
}

void DataRequest::do_success(const QString& message)
{
	status = DataRequestStatus::Success;
	emit status_info(message);
	emit request_success();
}

DeleteStandardDatastoreEntryRequest::DeleteStandardDatastoreEntryRequest(const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name) :
	DataRequest{ api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, key_name{ key_name }
{
	request_type = HttpRequestType::Delete;
}

QString DeleteStandardDatastoreEntryRequest::get_title_string() const
{
	return "Deleting entry...";
}

std::optional<bool> DeleteStandardDatastoreEntryRequest::is_delete_success() const
{
	if (status == DataRequestStatus::Success)
	{
		return delete_success;
	}
	else
	{
		return std::nullopt;
	}
}

QNetworkRequest DeleteStandardDatastoreEntryRequest::build_request(std::optional<QString>)
{
	return HttpRequestBuilder::delete_standard_datastore_entry(api_key, universe_id, datastore_name, scope, key_name);
}

void DeleteStandardDatastoreEntryRequest::handle_http_200(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	delete_success = true;
	do_success("Delete success");
}

void DeleteStandardDatastoreEntryRequest::handle_http_404(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	delete_success = false;
	do_success("Entry already deleted");
}

QString DeleteStandardDatastoreEntryRequest::get_send_message() const
{
	return QString{ "Deleting entry '%1'..." }.arg(key_name);
}

GetStandardDatastoresDataRequest::GetStandardDatastoresDataRequest(const QString& api_key, const long long universe_id) : DataRequest{ api_key }, universe_id{ universe_id }
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
			do_success("Complete");
		}
	}
	else
	{
		do_success("Complete");
	}
}

GetStandardDatastoreEntriesRequest::GetStandardDatastoreEntriesRequest(const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString prefix, std::optional<QString> initial_cursor) :
	DataRequest{ api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, prefix{ prefix }, initial_cursor{ initial_cursor }
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
	//assert(!(cursor && initial_cursor))
	QNetworkRequest request;
	if (cursor)
	{
		request = HttpRequestBuilder::get_standard_datastore_entries(api_key, universe_id, datastore_name, scope, prefix, cursor);
	}
	else
	{
		request = HttpRequestBuilder::get_standard_datastore_entries(api_key, universe_id, datastore_name, scope, prefix, initial_cursor);
		initial_cursor = std::nullopt;
	}
	return request;
}

void GetStandardDatastoreEntriesRequest::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	std::optional<GetStandardDatastoreEntriesResponse> response = GetStandardDatastoreEntriesResponse::fromJson(body, universe_id, datastore_name);
	if (response)
	{
		auto locked_entry_found_callback = entry_found_callback.lock();
		for (const StandardDatastoreEntryName& this_entry : response->get_entries())
		{
			datastore_entries.push_back(this_entry);
			if (locked_entry_found_callback)
			{
				locked_entry_found_callback->operator()(this_entry);
			}
		}

		emit status_info(QString{ "Received %1 entries, %2 total" }.arg(QString::number(response->get_entries().size()), QString::number(datastore_entries.size())));

		const bool limit_reached = result_limit && datastore_entries.size() >= *result_limit;

		std::optional<QString> cursor{ response->get_cursor() };
		if (cursor && cursor->size() > 0 && limit_reached == false)
		{
			if (auto locked_callback = enumerate_step_callback.lock())
			{
				locked_callback->operator()(universe_id, datastore_name.toStdString(), cursor->toStdString());
			}
			send_request(cursor);
		}
		else
		{
			if (auto locked_callback = enumerate_done_callback.lock())
			{
				locked_callback->operator()(universe_id, datastore_name.toStdString());
			}
			do_success("Complete");
		}
	}
	else
	{
		if (auto locked_callback = enumerate_done_callback.lock())
		{
			locked_callback->operator()(universe_id, datastore_name.toStdString());
		}
		do_success("Complete");
	}
}

GetStandardDatastoreEntryDetailsRequest::GetStandardDatastoreEntryDetailsRequest(const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name) :
	DataRequest{ api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, key_name{ key_name }
{

}

QString GetStandardDatastoreEntryDetailsRequest::get_title_string() const
{
	return "Fetching datastore entry details...";
}

std::optional<StandardDatastoreEntryFull> GetStandardDatastoreEntryDetailsRequest::get_details() const
{
	if (status == DataRequestStatus::Success)
	{
		return details;
	}
	else
	{
		return std::nullopt;
	}
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
		do_error("Error, response did not contain version");
		return;
	}

	std::optional<GetStandardDatastoreEntryDetailsResponse> response = GetStandardDatastoreEntryDetailsResponse::from(universe_id, datastore_name, scope, key_name, *version, userids, attributes, body);
	if (response)
	{
		details = response->get_details();
	}
	else
	{
		do_error("Received invalid response, aborting");
		return;
	}

	do_success("Complete");
}

void GetStandardDatastoreEntryDetailsRequest::handle_http_404(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	details = std::nullopt;

	do_success("Complete");
}

QString GetStandardDatastoreEntryDetailsRequest::get_send_message() const
{
	return QString{ "Fetching entry '%1'..." }.arg(key_name);
}

GetStandardDatastoreEntryAtVersionRequest::GetStandardDatastoreEntryAtVersionRequest(const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name, QString version) :
	GetStandardDatastoreEntryDetailsRequest{ api_key, universe_id, datastore_name, scope, key_name }, version{ version }
{

}

QNetworkRequest GetStandardDatastoreEntryAtVersionRequest::build_request(std::optional<QString>)
{
	return HttpRequestBuilder::get_standard_datastore_entry_version_details(api_key, universe_id, datastore_name, scope, key_name, version);
}

GetStandardDatastoreEntryVersionsRequest::GetStandardDatastoreEntryVersionsRequest(const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name) :
	DataRequest{ api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, key_name{ key_name }
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
			do_success("Complete");
		}
	}
	else
	{
		do_success("Complete");
	}
}

QString GetStandardDatastoreEntryVersionsRequest::get_send_message() const
{
	return QString{ "Fetching versions for '%1'..." }.arg(key_name);
}

PostMessagingServiceMessageRequest::PostMessagingServiceMessageRequest(const QString& api_key, long long universe_id, QString topic, QString unencoded_message)
	: DataRequest{ api_key }, universe_id{ universe_id }, topic{ topic }
{
	request_type = HttpRequestType::Post;

	QJsonObject send_object;
	send_object.insert("message", unencoded_message);

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
	do_success("Sent");
}

QString PostMessagingServiceMessageRequest::get_send_message() const
{
	return QString{ "Sending message to '%1'..." }.arg(topic);
}

PostStandardDatastoreEntryRequest::PostStandardDatastoreEntryRequest(const QString& api_key, long long universe_id, QString datastore_name, QString scope, QString key_name, std::optional<QString> userids, std::optional<QString> attributes, QString body)
	: DataRequest{ api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, key_name{ key_name }, userids{ userids }, attributes{ attributes }
{
	request_type = HttpRequestType::Post;
	post_body = body;
	body_md5 = QCryptographicHash::hash(body.toUtf8(), QCryptographicHash::Algorithm::Md5).toBase64();
}

QString PostStandardDatastoreEntryRequest::get_title_string() const
{
	return "Setting entry...";
}

bool PostStandardDatastoreEntryRequest::get_success() const
{
	return status == DataRequestStatus::Success;
}

QNetworkRequest PostStandardDatastoreEntryRequest::build_request(std::optional<QString>)
{
	return HttpRequestBuilder::post_standard_datastore_entry(api_key, universe_id, datastore_name, scope, key_name, body_md5, userids, attributes);
}

void PostStandardDatastoreEntryRequest::handle_http_200(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	do_success("Complete");
}

QString PostStandardDatastoreEntryRequest::get_send_message() const
{
	return QString{ "Setting key '%1'..." }.arg(key_name);
}
