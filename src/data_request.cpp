#include "data_request.h"

#include <Qt>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaEnum>
#include <QNetworkReply>
#include <QTimer>
#include <QUuid>
#include <QVariant>

#include <memory>

#include "http_req_builder.h"
#include "http_wrangler.h"
#include "model_api_opencloud.h"
#include "roblox_time.h"
#include "util_enum.h"

DataRequestBody::DataRequestBody()
{
	md5 = QCryptographicHash::hash("", QCryptographicHash::Algorithm::Md5).toBase64();
}

DataRequestBody::DataRequestBody(const QString& data) : data{ data }
{
	md5 = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Algorithm::Md5).toBase64();
}

DataRequestBody::operator bool() const
{
	return data.has_value();
}

void DataRequest::send_request(const std::optional<QString>& cursor)
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
	pending_reply = HttpWrangler::get()->send(request_type, *pending_request, req_body.get_data());

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
	QString http_status = pending_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
	QString reply_body = pending_reply->readAll();
	QList<QNetworkReply::RawHeaderPair> headers = pending_reply->rawHeaderPairs();
	RobloxTime::update_time_from_headers(headers);

	// Prevent timeout from firing
	pending_reply->disconnect(this);

	pending_reply->deleteLater();
	pending_reply = nullptr;

	if (http_status == "200") // OK -- NOLINTNEXTLINE(bugprone-branch-clone)
	{
		handle_http_200(reply_body, headers);
	}
	else if (http_status == "204") // No content, body will be empty
	{
		handle_http_200(reply_body, headers);
	}
	else if (http_status == "404") // Not found
	{
		handle_http_404(reply_body, headers);
	}
	else if (http_status == "429") // Too many requests
	{
		emit status_info(QString{ "Received HTTP 429, briefly pausing..." });
		emit received_http_429();

		QTimer* timer = new QTimer(this);
		timer->setSingleShot(true);
		connect(timer, &QTimer::timeout, this, &DataRequest::resend);
		timer->start(get_next_429_delay());
	}
	else if (http_status == "500") // Internal server error
	{
		emit status_info(QString{ "Received HTTP 500 Internal Server Error, retrying..." });
		resend();
	}
	else if (http_status == "502") // Bad gateway
	{
		emit status_info(QString{ "Received HTTP 502 Bad Gateway, retrying..." });
		resend();
	}
	else if (http_status == "504") // Gateway timeout
	{
		emit status_info(QString{ "Received HTTP 504 Gateway Timeout, retrying..." });
		resend();
	}
	else if (http_status == "")
	{
		QMetaEnum meta_enum = QMetaEnum::fromType<QNetworkReply::NetworkError>();
		do_error( QString{ "Network error %1, aborting" }.arg( meta_enum.valueToKey( static_cast<int>(error) ) ) );
	}
	else
	{
		do_error(QString{ "Received HTTP %1, aborting" }.arg(http_status));
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
		pending_reply = HttpWrangler::get()->send(request_type, *pending_request, req_body.get_data());
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
	emit success();
}

MemoryStoreSortedMapGetListRequest::MemoryStoreSortedMapGetListRequest(const QString& api_key, long long universe_id, const QString& map_name, bool ascending)
	: DataRequest{ api_key }, universe_id{ universe_id }, map_name{ map_name }, ascending{ ascending }
{

}

QString MemoryStoreSortedMapGetListRequest::get_title_string() const
{
	return "Fetching memory store sorted map entries...";
}

void MemoryStoreSortedMapGetListRequest::set_result_limit(const size_t limit)
{
	result_limit = limit;
}

QNetworkRequest MemoryStoreSortedMapGetListRequest::build_request(std::optional<QString> cursor) const
{
	return HttpRequestBuilder::memory_store_v2_sorted_map_get_list(api_key, universe_id, map_name, ascending, cursor);
}

void MemoryStoreSortedMapGetListRequest::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	if (const std::optional<GetMemoryStoreSortedMapItemListResponse> response = GetMemoryStoreSortedMapItemListResponse::from_json(universe_id, map_name, body))
	{
		for (const MemoryStoreSortedMapItem& this_entry : response->get_items())
		{
			if (result_limit && items.size() >= *result_limit)
			{
				// Limit has been hit
				break;
			}
			items.push_back(this_entry);
		}

		const bool limit_reached = result_limit && items.size() >= *result_limit;
		const std::optional<QString> token{ response->get_next_page_token() };
		if (token && token->size() > 0 && !limit_reached)
		{
			send_request(token);
		}
		else
		{
			do_success();
		}
	}
	else
	{
		status_error("Received invalid response with HTTP 200");
	}
}

MessagingServicePostMessageV2Request::MessagingServicePostMessageV2Request(const QString& api_key, long long universe_id, const QString& topic, const QString& unencoded_message)
	: DataRequest{ api_key }, universe_id{ universe_id }, topic{ topic }
{
	request_type = HttpRequestType::Post;

	QJsonObject send_object;
	send_object.insert("topic", topic);
	send_object.insert("message", unencoded_message);

	const QJsonDocument doc{ send_object };
	req_body = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString MessagingServicePostMessageV2Request::get_title_string() const
{
	return "Setting message (v2)...";
}

QNetworkRequest MessagingServicePostMessageV2Request::build_request(std::optional<QString>) const
{
	return HttpRequestBuilder::messaging_service_v2_post_message(api_key, universe_id);
}

void MessagingServicePostMessageV2Request::handle_http_200(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	do_success("Message sent");
}

QString MessagingServicePostMessageV2Request::get_send_message() const
{
	return QString{ "Sending message to '%1'..." }.arg(topic);
}

OrderedDatastoreEntryDeleteV2Request::OrderedDatastoreEntryDeleteV2Request(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id) :
	DataRequest{ api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, entry_id{ entry_id }
{
	request_type = HttpRequestType::Delete;
}

QString OrderedDatastoreEntryDeleteV2Request::get_title_string() const
{
	return "Deleting ordered datastore entry (v2)...";
}

QNetworkRequest OrderedDatastoreEntryDeleteV2Request::build_request(std::optional<QString>) const
{
	return HttpRequestBuilder::ordered_datastore_v2_entry_delete(api_key, universe_id, datastore_name, scope, entry_id);
}

void OrderedDatastoreEntryDeleteV2Request::handle_http_200(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	do_success();
}

QString OrderedDatastoreEntryDeleteV2Request::get_send_message() const
{
	return QString{ "Deleting '%1'..." }.arg(entry_id);
}

OrderedDatastoreEntryGetDetailsV2Request::OrderedDatastoreEntryGetDetailsV2Request(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id) :
	DataRequest{ api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, entry_id{ entry_id }
{

}

QString OrderedDatastoreEntryGetDetailsV2Request::get_title_string() const
{
	return "Fetching ordered datastore entry details (v2)...";
}

std::optional<OrderedDatastoreEntryFull> OrderedDatastoreEntryGetDetailsV2Request::get_details() const
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

QNetworkRequest OrderedDatastoreEntryGetDetailsV2Request::build_request(std::optional<QString>) const
{
	return HttpRequestBuilder::ordered_datastore_v2_entry_get_details(api_key, universe_id, datastore_name, scope, entry_id);
}

void OrderedDatastoreEntryGetDetailsV2Request::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	std::optional<GetOrderedDatastoreEntryDetailsV2Response> response = GetOrderedDatastoreEntryDetailsV2Response::from_json(universe_id, datastore_name, scope, entry_id, body);
	if (response)
	{
		details = response->get_details();
	}
	else
	{
		do_error("Received invalid response, aborting");
		return;
	}

	do_success();
}

QString OrderedDatastoreEntryGetDetailsV2Request::get_send_message() const
{
	return QString{ "Fetching information for key '%1'..." }.arg(entry_id);
}

OrderedDatastoreEntryGetListV2Request::OrderedDatastoreEntryGetListV2Request(const QString& api_key, const long long universe_id, const QString& datastore_name, const QString& scope, const bool ascending) :
	DataRequest{ api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, ascending{ ascending }
{

}

QString OrderedDatastoreEntryGetListV2Request::get_title_string() const
{
	return "Fetching ordered datastore entries (v2)...";
}

void OrderedDatastoreEntryGetListV2Request::set_result_limit(const size_t limit)
{
	result_limit = limit;
}

QNetworkRequest OrderedDatastoreEntryGetListV2Request::build_request(std::optional<QString> cursor) const
{
	return HttpRequestBuilder::ordered_datastore_v2_entry_get_list(api_key, universe_id, datastore_name, scope, ascending, cursor);
}

void OrderedDatastoreEntryGetListV2Request::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	if (const std::optional<GetOrderedDatastoreEntryListV2Response> response = GetOrderedDatastoreEntryListV2Response::from_json(universe_id, datastore_name, scope, body))
	{
		for (const OrderedDatastoreEntryFull& this_entry : response->get_entries())
		{
			if (result_limit && entries.size() >= *result_limit)
			{
				// Limit has been hit
				break;
			}
			entries.push_back(this_entry);
		}

		const bool limit_reached = result_limit && entries.size() >= *result_limit;
		const std::optional<QString> token{ response->get_next_page_token() };
		if (token && token->size() > 0 && !limit_reached)
		{
			send_request(token);
		}
		else
		{
			do_success();
		}
	}
	else
	{
		status_error("Received invalid response with HTTP 200");
	}
}

OrderedDatastoreEntryPostCreateV2Request::OrderedDatastoreEntryPostCreateV2Request(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id, const long long value)
	: DataRequest{ api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, entry_id{ entry_id }, value{ value }
{
	request_type = HttpRequestType::Post;
	QJsonObject body_json_obj;
	body_json_obj.insert("value", QJsonValue::fromVariant(value));
	const QJsonDocument body_json_doc{ body_json_obj };
	req_body = QString::fromUtf8(body_json_doc.toJson(QJsonDocument::Compact));
}

QString OrderedDatastoreEntryPostCreateV2Request::get_title_string() const
{
	return "Creating entry (v2)...";
}

QNetworkRequest OrderedDatastoreEntryPostCreateV2Request::build_request(std::optional<QString>) const
{
	return HttpRequestBuilder::ordered_datastore_v2_entry_post_create(api_key, universe_id, datastore_name, scope, entry_id, req_body.get_md5());
}

void OrderedDatastoreEntryPostCreateV2Request::handle_http_200(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	do_success();
}

QString OrderedDatastoreEntryPostCreateV2Request::get_send_message() const
{
	return QString{ "Creating entry '%1' with value %2..." }.arg(entry_id).arg(value);
}

OrderedDatastoreEntryPatchUpdateV2Request::OrderedDatastoreEntryPatchUpdateV2Request(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id, const long long new_value)
	: DataRequest{ api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, entry_id{ entry_id }, new_value{ new_value }
{
	request_type = HttpRequestType::Patch;
	QJsonObject body_json_obj;
	body_json_obj.insert("value", QJsonValue::fromVariant(new_value));
	const QJsonDocument body_json_doc{ body_json_obj };
	req_body = QString::fromUtf8(body_json_doc.toJson(QJsonDocument::Compact));
}

QString OrderedDatastoreEntryPatchUpdateV2Request::get_title_string() const
{
	return "Updating entry (v2)...";
}

QNetworkRequest OrderedDatastoreEntryPatchUpdateV2Request::build_request(std::optional<QString>) const
{
	return HttpRequestBuilder::ordered_datastore_v2_entry_patch_update(api_key, universe_id, datastore_name, scope, entry_id, req_body.get_md5());
}

void OrderedDatastoreEntryPatchUpdateV2Request::handle_http_200(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	do_success();
}

QString OrderedDatastoreEntryPatchUpdateV2Request::get_send_message() const
{
	return QString{ "Setting '%1' to %2..." }.arg(entry_id).arg(new_value);
}

OrderedDatastorePostIncrementV2Request::OrderedDatastorePostIncrementV2Request(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id, const long long increment_by)
	: DataRequest{ api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, entry_id{ entry_id }, increment_by{ increment_by }
{
	request_type = HttpRequestType::Post;
	QJsonObject body_json_obj;
	body_json_obj.insert("amount", QJsonValue::fromVariant(increment_by));
	const QJsonDocument body_json_doc{ body_json_obj };
	req_body = QString::fromUtf8(body_json_doc.toJson(QJsonDocument::Compact));
}

QString OrderedDatastorePostIncrementV2Request::get_title_string() const
{
	return "Incrementing entry (v2)...";
}

QNetworkRequest OrderedDatastorePostIncrementV2Request::build_request(std::optional<QString>) const
{
	return HttpRequestBuilder::ordered_datastore_v2_entry_post_increment(api_key, universe_id, datastore_name, scope, entry_id, req_body.get_md5());
}

void OrderedDatastorePostIncrementV2Request::handle_http_200(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	do_success();
}

QString OrderedDatastorePostIncrementV2Request::get_send_message() const
{
	return QString{ "Incrementing '%1' by %2..." }.arg(entry_id).arg(increment_by);
}

StandardDatastoreEntryDeleteRequest::StandardDatastoreEntryDeleteRequest(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name) :
	DataRequest{ api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, key_name{ key_name }
{
	request_type = HttpRequestType::Delete;
}

QString StandardDatastoreEntryDeleteRequest::get_title_string() const
{
	return "Deleting entry...";
}

std::optional<bool> StandardDatastoreEntryDeleteRequest::is_delete_success() const
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

QNetworkRequest StandardDatastoreEntryDeleteRequest::build_request(std::optional<QString>) const
{
	return HttpRequestBuilder::standard_datastore_entry_delete(api_key, universe_id, datastore_name, scope, key_name);
}

void StandardDatastoreEntryDeleteRequest::handle_http_200(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	delete_success = true;
	do_success();
}

void StandardDatastoreEntryDeleteRequest::handle_http_404(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	delete_success = false;
	do_success("Entry already deleted");
}

QString StandardDatastoreEntryDeleteRequest::get_send_message() const
{
	return QString{ "Deleting entry '%1'..." }.arg(key_name);
}

StandardDatastoreEntryGetDetailsRequest::StandardDatastoreEntryGetDetailsRequest(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name) :
	DataRequest{ api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, key_name{ key_name }
{

}

QString StandardDatastoreEntryGetDetailsRequest::get_title_string() const
{
	return "Fetching datastore entry details...";
}

std::optional<StandardDatastoreEntryFull> StandardDatastoreEntryGetDetailsRequest::get_details() const
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

QNetworkRequest StandardDatastoreEntryGetDetailsRequest::build_request(std::optional<QString>) const
{
	return HttpRequestBuilder::standard_datastore_entry_get_details(api_key, universe_id, datastore_name, scope, key_name);
}

void StandardDatastoreEntryGetDetailsRequest::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>& headers)
{
	std::optional<QString> version;
	std::optional<QString> userids;
	std::optional<QString> attributes;
	for (const QNetworkReply::RawHeaderPair& this_header : headers)
	{
		const QString header_key{ this_header.first };
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

	do_success();
}

void StandardDatastoreEntryGetDetailsRequest::handle_http_404(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	details = std::nullopt;

	do_success();
}

QString StandardDatastoreEntryGetDetailsRequest::get_send_message() const
{
	return QString{ "Fetching entry '%1'..." }.arg(key_name);
}

StandardDatastoreEntryGetListRequest::StandardDatastoreEntryGetListRequest(
    const QString& api_key,
    const long long universe_id,
    const QString& datastore_name,
    const QString& scope,
    const QString& prefix,
    const std::optional<QString>& initial_cursor
    ) :
	DataRequest{ api_key },
    universe_id{ universe_id },
    datastore_name{ datastore_name },
    scope{ scope },
    prefix{ prefix },
    initial_cursor{ initial_cursor }
{

}

QString StandardDatastoreEntryGetListRequest::get_title_string() const
{
	return "Fetching datastore entries...";
}

void StandardDatastoreEntryGetListRequest::set_result_limit(const size_t limit)
{
	result_limit = limit;
}

QNetworkRequest StandardDatastoreEntryGetListRequest::build_request(std::optional<QString> cursor) const
{
	QNetworkRequest request;
	if (cursor)
	{
		request = HttpRequestBuilder::standard_datastore_entry_get_list(api_key, universe_id, datastore_name, scope, prefix, cursor);
	}
	else
	{
		request = HttpRequestBuilder::standard_datastore_entry_get_list(api_key, universe_id, datastore_name, scope, prefix, initial_cursor);
	}
	return request;
}

void StandardDatastoreEntryGetListRequest::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	std::optional<GetStandardDatastoreEntryListResponse> response = GetStandardDatastoreEntryListResponse::from_json(body, universe_id, datastore_name);
	if (response)
	{
		for (const StandardDatastoreEntryName& this_entry : response->get_entries())
		{
			if (result_limit && datastore_entries.size() >= *result_limit)
			{
				// Limit has been hit
				break;
			}
			datastore_entries.push_back(this_entry);
			emit entry_found(this_entry);
		}

		emit status_info(QString{ "Received %1 entries, %2 total" }.arg(QString::number(response->get_entries().size()), QString::number(datastore_entries.size())));

		const bool limit_reached = result_limit && datastore_entries.size() >= *result_limit;

		std::optional<QString> cursor{ response->get_cursor() };
		if (cursor && cursor->size() > 0 && !limit_reached)
		{
			emit enumerate_step(universe_id, datastore_name.toStdString(), cursor->toStdString());
			send_request(cursor);
		}
		else
		{
			emit enumerate_done(universe_id, datastore_name.toStdString());
			do_success();
		}
	}
	else
	{
		emit enumerate_done(universe_id, datastore_name.toStdString());
		do_success();
	}
}

StandardDatastoreEntryGetVersionRequest::StandardDatastoreEntryGetVersionRequest(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version) :
	StandardDatastoreEntryGetDetailsRequest{ api_key, universe_id, datastore_name, scope, key_name }, version{ version }
{

}

QNetworkRequest StandardDatastoreEntryGetVersionRequest::build_request(std::optional<QString>) const
{
	return HttpRequestBuilder::standard_datastore_entry_version_get_details(api_key, universe_id, datastore_name, scope, key_name, version);
}

StandardDatastoreEntryGetVersionListRequest::StandardDatastoreEntryGetVersionListRequest(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name) :
	DataRequest{ api_key }, universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, key_name{ key_name }
{

}

QString StandardDatastoreEntryGetVersionListRequest::get_title_string() const
{
	return "Fetching datastore entry versions...";
}

QNetworkRequest StandardDatastoreEntryGetVersionListRequest::build_request(std::optional<QString> cursor) const
{
	return HttpRequestBuilder::standard_datastore_entry_version_get_list(api_key, universe_id, datastore_name, scope, key_name, cursor);
}

void StandardDatastoreEntryGetVersionListRequest::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	std::optional<GetStandardDatastoreEntryVersionListResponse> response = GetStandardDatastoreEntryVersionListResponse::from(body);
	if (response)
	{
		for (const StandardDatastoreEntryVersion& this_version : response->get_versions())
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
			do_success();
		}
	}
	else
	{
		do_success();
	}
}

QString StandardDatastoreEntryGetVersionListRequest::get_send_message() const
{
	return QString{ "Fetching versions for '%1'..." }.arg(key_name);
}

StandardDatastoreEntryPostSetRequest::StandardDatastoreEntryPostSetRequest(
    const QString& api_key,
    const long long universe_id,
    const QString& datastore_name,
    const QString& scope,
    const QString& key_name,
    const std::optional<QString>& userids,
    const std::optional<QString>& attributes,
    const QString& body
    ) :
    DataRequest{ api_key },
    universe_id{ universe_id },
    datastore_name{ datastore_name },
    scope{ scope },
    key_name{ key_name },
    userids{ userids },
    attributes{ attributes }
{
	request_type = HttpRequestType::Post;
	req_body = body;
}

QString StandardDatastoreEntryPostSetRequest::get_title_string() const
{
	return "Setting entry...";
}

QNetworkRequest StandardDatastoreEntryPostSetRequest::build_request(std::optional<QString>) const
{
	return HttpRequestBuilder::standard_datastore_entry_post(api_key, universe_id, datastore_name, scope, key_name, req_body.get_md5(), userids, attributes);
}

void StandardDatastoreEntryPostSetRequest::handle_http_200(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	do_success();
}

QString StandardDatastoreEntryPostSetRequest::get_send_message() const
{
	return QString{ "Setting key '%1'..." }.arg(key_name);
}

StandardDatastoreGetListRequest::StandardDatastoreGetListRequest(const QString& api_key, const long long universe_id) : DataRequest{ api_key }, universe_id{ universe_id }
{

}

QString StandardDatastoreGetListRequest::get_title_string() const
{
	return "Fetching datastores...";
}

QNetworkRequest StandardDatastoreGetListRequest::build_request(std::optional<QString> cursor) const
{
	return HttpRequestBuilder::standard_datastore_get_list(api_key, universe_id, cursor);
}

void StandardDatastoreGetListRequest::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	std::optional<GetStandardDatastoreListResponse> response = GetStandardDatastoreListResponse::from_json(body);
	if (response)
	{
		for (const QString& this_name : response->get_datastores_vec())
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
			do_success();
		}
	}
	else
	{
		do_success();
	}
}

StandardDatastorePostSnapshotRequest::StandardDatastorePostSnapshotRequest(const QString& api_key, long long universe_id)
	: DataRequest{ api_key }, universe_id{ universe_id }
{
	request_type = HttpRequestType::Post;
	req_body = DataRequestBody{ "{}" };
}

QString StandardDatastorePostSnapshotRequest::get_title_string() const
{
	return "Snapshotting datastores...";
}

QNetworkRequest StandardDatastorePostSnapshotRequest::build_request(std::optional<QString>) const
{
	return HttpRequestBuilder::standard_datastore_v2_snapshot(api_key, universe_id);
}

void StandardDatastorePostSnapshotRequest::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	const std::optional<PostStandardDatastoreSnapshotResponseV2> response = PostStandardDatastoreSnapshotResponseV2::from(body);
	if (response)
	{
		new_snapshot_taken = response->get_new_snapshot_taken();
		latest_snapshot_time = response->get_latest_snapshot_time();
	}
	do_success();
}

UniverseGetDetailsRequest::UniverseGetDetailsRequest(const QString& api_key, const long long universe_id)
	: DataRequest{ api_key }, universe_id{ universe_id }
{

}

QString UniverseGetDetailsRequest::get_title_string() const
{
	return "Fetching universe details...";
}

QNetworkRequest UniverseGetDetailsRequest::build_request(std::optional<QString>) const
{
	return HttpRequestBuilder::universe_v2_get_details(api_key, universe_id);
}

void UniverseGetDetailsRequest::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	const std::optional<GetUniverseDetailsResponse> response = GetUniverseDetailsResponse::from(body);
	if (response)
	{
		display_name = response->get_display_name();
	}
	do_success();
}

UserRestrictionGetListV2Request::UserRestrictionGetListV2Request(const QString& api_key, const long long universe_id)
	: DataRequest{ api_key }, universe_id{ universe_id }
{

}

QString UserRestrictionGetListV2Request::get_title_string() const
{
	return "Fetching user restrictions...";
}

QNetworkRequest UserRestrictionGetListV2Request::build_request(const std::optional<QString> cursor) const
{
	return HttpRequestBuilder::use_restrictions_v2_list(api_key, universe_id, cursor);
}

void UserRestrictionGetListV2Request::handle_http_200(const QString& body, const QList<QNetworkReply::RawHeaderPair>&)
{
	const std::optional<GetUserRestrictionListV2Response> response = GetUserRestrictionListV2Response::from(body);
	if (!response)
	{
		status_error("Received invalid response with HTTP 200");
		return;
	}

	for (const BanListUserRestriction& this_restriction : response->get_restrictions())
	{
		if (result_limit && restrictions.size() >= *result_limit)
		{
			// Limit has been hit
			break;
		}
		restrictions.push_back(this_restriction);
	}

	const bool limit_reached = result_limit && restrictions.size() >= *result_limit;
	const std::optional<QString> token{ response->get_next_page_token() };
	if (token && token->size() > 0 && !limit_reached)
	{
		send_request(token);
	}
	else
	{
		do_success();
	}

	do_success();
}

UserRestrictionPatchUpdateV2Request::UserRestrictionPatchUpdateV2Request(const QString& api_key, const QString& path, const BanListGameJoinRestrictionUpdate& restriction_update) :
	DataRequest{ api_key },
	path{ path },
	restriction_update{ restriction_update }
{
	request_type = HttpRequestType::Patch;
	req_body = restriction_update.to_json();
}

QString UserRestrictionPatchUpdateV2Request::get_title_string() const
{
	return "Updating ban details...";
}

QNetworkRequest UserRestrictionPatchUpdateV2Request::build_request(const std::optional<QString>) const
{
	const QString uuid = QUuid::createUuid().toString();
	const QString now_str = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
	QString path_with_params = path;
	path_with_params = path_with_params + "?idempotencyKey.key=" + uuid;
	path_with_params = path_with_params + "&idempotencyKey.firstSent=" + now_str;
	return HttpRequestBuilder::resource_v2(api_key, path_with_params);
}

void UserRestrictionPatchUpdateV2Request::handle_http_200(const QString&, const QList<QNetworkReply::RawHeaderPair>&)
{
	do_success();
}

QString UserRestrictionPatchUpdateV2Request::get_send_message() const
{
	return QString{ "Updating ban %1" }.arg(path);
}
