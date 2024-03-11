#include "http_req_builder.h"

#include <QByteArray>
#include <QNetworkRequest>
#include <QUrl>

QNetworkRequest HttpRequestBuilder::delete_standard_datastore_entry(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name)
{
	QString url = standard_datastore_base_url(universe_id) + "/standard-datastores/datastore/entries/entry";
	url = url + "?datastoreName=" + QUrl::toPercentEncoding(datastore_name);
	url = url + "&scope=" + QUrl::toPercentEncoding(scope);
	url = url + "&entryKey=" + QUrl::toPercentEncoding(key_name);

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::get_ordered_datastore_entry_details(const QString& api_key, const long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name)
{
	QString url = ordered_datastore_base_url(universe_id);
	url = url + "/orderedDataStores/" + QUrl::toPercentEncoding(datastore_name);
	url = url + "/scopes/" + QUrl::toPercentEncoding(scope);
	url = url + "/entries/" + QUrl::toPercentEncoding(key_name);

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::get_ordered_datastore_entry_list(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const bool ascending, std::optional<QString> cursor)
{
	QString url = ordered_datastore_base_url(universe_id) +
		"/orderedDataStores/" + QUrl::toPercentEncoding(datastore_name) +
		"/scopes/" + QUrl::toPercentEncoding(scope) +
		"/entries?max_page_size=100&order_by=" + QString{ ascending ? "asc" : "desc" };
	if (cursor)
	{
		url = url + "&page_token=" + QUrl::toPercentEncoding(*cursor);
	}

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::get_standard_datastore_entry_details(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name)
{
	QString url = standard_datastore_base_url(universe_id) + "/standard-datastores/datastore/entries/entry";
	url = url + "?datastoreName=" + QUrl::toPercentEncoding(datastore_name);
	url = url + "&scope=" + QUrl::toPercentEncoding(scope);
	url = url + "&entryKey=" + QUrl::toPercentEncoding(key_name);

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::get_standard_datastore_entry_list(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& prefix, std::optional<QString> cursor)
{
	QString url = standard_datastore_base_url(universe_id) + "/standard-datastores/datastore/entries?limit=100";
	url = url + "&datastoreName=" + QUrl::toPercentEncoding(datastore_name);
	if (scope.size() > 0)
	{
		url = url + "&scope=" + QUrl::toPercentEncoding(scope);
	}
	else
	{
		url = url + "&AllScopes=true";
	}
	if (prefix.size() > 0)
	{
		url = url + "&prefix=" + QUrl::toPercentEncoding(prefix);
	}
	if (cursor)
	{
		url = url + "&cursor=" + QUrl::toPercentEncoding(*cursor);
	}

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::get_standard_datastore_entry_version_details(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version)
{
	QString url = standard_datastore_base_url(universe_id) + "/standard-datastores/datastore/entries/entry/versions/version";
	url = url + "?datastoreName=" + QUrl::toPercentEncoding(datastore_name);
	url = url + "&scope=" + QUrl::toPercentEncoding(scope);
	url = url + "&entryKey=" + QUrl::toPercentEncoding(key_name);
	url = url + "&versionId=" + QUrl::toPercentEncoding(version);

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::get_standard_datastore_entry_version_list(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, std::optional<QString> cursor)
{
	QString url = standard_datastore_base_url(universe_id) + "/standard-datastores/datastore/entries/entry/versions?sortOrder=Descending";
	url = url + "&datastoreName=" + QUrl::toPercentEncoding(datastore_name);
	url = url + "&scope=" + QUrl::toPercentEncoding(scope);
	url = url + "&entryKey=" + QUrl::toPercentEncoding(key_name);
	if (cursor)
	{
		url = url + "&cursor=" + QUrl::toPercentEncoding(*cursor);
	}

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::get_standard_datastore_list(const QString& api_key, const long long universe_id, std::optional<QString> cursor)
{
	QString url = standard_datastore_base_url(universe_id) + "/standard-datastores?limit=50";
	if (cursor)
	{
		url = url + "&cursor=" + QUrl::toPercentEncoding(*cursor);
	}
	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::get_universe_details(const QString& api_key, const long long universe_id)
{
	QString url = universe_base_url(universe_id);
	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::post_messaging_service_message(const QString api_key, long long universe_id, const QString& topic)
{
	const QString url = messaging_base_url(universe_id) + "/topics/" + QUrl::toPercentEncoding(topic);

	QNetworkRequest req{ url };
	req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::post_standard_datastore_entry(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& body_md5, const std::optional<QString>& userids, const std::optional<QString>& attributes)
{
	QString url = standard_datastore_base_url(universe_id) + "/standard-datastores/datastore/entries/entry";
	url = url + "?datastoreName=" + QUrl::toPercentEncoding(datastore_name);
	url = url + "&scope=" + QUrl::toPercentEncoding(scope);
	url = url + "&entryKey=" + QUrl::toPercentEncoding(key_name);

	QNetworkRequest req{ url };
	req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	req.setRawHeader("content-md5", body_md5.toStdString().c_str());
	if (userids)
	{
		req.setRawHeader("roblox-entry-userids", userids->toStdString().c_str());
	}
	if (attributes)
	{
		req.setRawHeader("roblox-entry-attributes", attributes->toStdString().c_str());
	}
	return req;
}

QString HttpRequestBuilder::messaging_base_url(const long long universe_id)
{
	return QString{ "https://apis.roblox.com/messaging-service/v1/universes/" } + QString::number(universe_id);
}

QString HttpRequestBuilder::ordered_datastore_base_url(const long long universe_id)
{
	return QString{ "https://apis.roblox.com/ordered-data-stores/v1/universes/" } + QString::number(universe_id);
}

QString HttpRequestBuilder::standard_datastore_base_url(const long long universe_id)
{
	return QString{ "https://apis.roblox.com/datastores/v1/universes/" } + QString::number(universe_id);
}

QString HttpRequestBuilder::universe_base_url(const long long universe_id)
{
	return QString{ "https://apis.roblox.com/cloud/v2/universes/" } + QString::number(universe_id);
}
