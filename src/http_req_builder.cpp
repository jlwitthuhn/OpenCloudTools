#include "http_req_builder.h"

#include <string>

#include <QByteArray>
#include <QNetworkRequest>
#include <QUrl>

QNetworkRequest HttpRequestBuilder::memory_store_v2_sorted_map_get_list(const QString& api_key, const long long universe_id, const QString& map_name, bool ascending, const std::optional<QString>& cursor)
{
	QString url = base_url_memory_store_v2(universe_id);
	url = url + "/sorted-maps/" + QUrl::toPercentEncoding(map_name);
	url = url + "/items";
	url = url + "?maxPageSize=100";
	url = url + "&orderBy=" + (ascending ? "asc" : "desc");
	if (cursor)
	{
		url = url + "&pageToken=" + QUrl::toPercentEncoding(*cursor);
	}

	QNetworkRequest req{ url };
	req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::messaging_service_v2_post_message(const QString& api_key, long long universe_id)
{
	const QString url = base_url_universe_v2(universe_id) + ":publishMessage";

	QNetworkRequest req{ url };
	req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::ordered_datastore_v2_entry_delete(const QString& api_key, const long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id)
{
	QString url = base_url_ordered_datastore_v2(universe_id);
	url = url + "/" + QUrl::toPercentEncoding(datastore_name);
	url = url + "/scopes/" + QUrl::toPercentEncoding(scope);
	url = url + "/entries/" + QUrl::toPercentEncoding(entry_id);

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::ordered_datastore_v2_entry_get_details(const QString& api_key, const long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name)
{
	QString url = base_url_ordered_datastore_v2(universe_id);
	url = url + "/" + QUrl::toPercentEncoding(datastore_name);
	url = url + "/scopes/" + QUrl::toPercentEncoding(scope);
	url = url + "/entries/" + QUrl::toPercentEncoding(key_name);

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::ordered_datastore_v2_entry_get_list(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const bool ascending, std::optional<QString> cursor)
{
	QString url = base_url_ordered_datastore_v2(universe_id) +
		"/" + QUrl::toPercentEncoding(datastore_name) +
		"/scopes/" + QUrl::toPercentEncoding(scope) +
		"/entries?maxPageSize=100&orderBy=" + QString{ ascending ? "value" : "value%20desc" };
	if (cursor)
	{
		url = url + "&pageToken=" + QUrl::toPercentEncoding(*cursor);
	}

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::ordered_datastore_v2_entry_patch_update(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id, const QString& body_md5)
{
	QString url = base_url_ordered_datastore_v2(universe_id);
	url = url + "/" + QUrl::toPercentEncoding(datastore_name);
	url = url + "/scopes/" + QUrl::toPercentEncoding(scope);
	url = url + "/entries/" + QUrl::toPercentEncoding(entry_id);

	QNetworkRequest req{ url };
	req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
	req.setRawHeader("Accept", "*/*");
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	req.setRawHeader("content-md5", body_md5.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::ordered_datastore_v2_entry_post_create(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id, const QString& body_md5)
{
	QString url = base_url_ordered_datastore_v2(universe_id);
	url = url + "/" + QUrl::toPercentEncoding(datastore_name);
	url = url + "/scopes/" + QUrl::toPercentEncoding(scope);
	url = url + "/entries";
	url = url + "?id=" + QUrl::toPercentEncoding(entry_id);

	QNetworkRequest req{ url };
	req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	req.setRawHeader("content-md5", body_md5.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::ordered_datastore_v2_entry_post_increment(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id, const QString& body_md5)
{
	QString url = base_url_ordered_datastore_v2(universe_id);
	url = url + "/" + QUrl::toPercentEncoding(datastore_name);
	url = url + "/scopes/" + QUrl::toPercentEncoding(scope);
	url = url + "/entries/" + QUrl::toPercentEncoding(entry_id);
	url = url + ":increment";

	QNetworkRequest req{ url };
	req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	req.setRawHeader("content-md5", body_md5.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::resource_v2(const std::optional<QString>& api_key, const QString& path)
{
	const QString url = base_url_v2() + path;

	QNetworkRequest req{ url };
	req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
	if (api_key)
	{
		req.setRawHeader("x-api-key", api_key->toStdString().c_str());
	}
	return req;
}

QNetworkRequest HttpRequestBuilder::standard_datastore_get_list(const QString& api_key, const long long universe_id, std::optional<QString> cursor)
{
	QString url = base_url_standard_datastore(universe_id) + "/standard-datastores?limit=50";
	if (cursor)
	{
		url = url + "&cursor=" + QUrl::toPercentEncoding(*cursor);
	}
	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::standard_datastore_v2_snapshot(const QString& api_key, const long long universe_id)
{
	const QString url = base_url_standard_datastore_v2(universe_id) + ":snapshot";
	QNetworkRequest req{ url };
	req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::standard_datastore_entry_delete(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name)
{
	QString url = base_url_standard_datastore(universe_id) + "/standard-datastores/datastore/entries/entry";
	url = url + "?datastoreName=" + QUrl::toPercentEncoding(datastore_name);
	url = url + "&scope=" + QUrl::toPercentEncoding(scope);
	url = url + "&entryKey=" + QUrl::toPercentEncoding(key_name);

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::standard_datastore_entry_get_details(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name)
{
	QString url = base_url_standard_datastore(universe_id) + "/standard-datastores/datastore/entries/entry";
	url = url + "?datastoreName=" + QUrl::toPercentEncoding(datastore_name);
	url = url + "&scope=" + QUrl::toPercentEncoding(scope);
	url = url + "&entryKey=" + QUrl::toPercentEncoding(key_name);

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::standard_datastore_entry_get_list(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& prefix, std::optional<QString> cursor)
{
	QString url = base_url_standard_datastore(universe_id) + "/standard-datastores/datastore/entries?limit=100";
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

QNetworkRequest HttpRequestBuilder::standard_datastore_entry_post(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& body_md5, const std::optional<QString>& userids, const std::optional<QString>& attributes)
{
	QString url = base_url_standard_datastore(universe_id) + "/standard-datastores/datastore/entries/entry";
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

QNetworkRequest HttpRequestBuilder::standard_datastore_entry_version_get_details(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version)
{
	QString url = base_url_standard_datastore(universe_id) + "/standard-datastores/datastore/entries/entry/versions/version";
	url = url + "?datastoreName=" + QUrl::toPercentEncoding(datastore_name);
	url = url + "&scope=" + QUrl::toPercentEncoding(scope);
	url = url + "&entryKey=" + QUrl::toPercentEncoding(key_name);
	url = url + "&versionId=" + QUrl::toPercentEncoding(version);

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::standard_datastore_entry_version_get_list(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, std::optional<QString> cursor)
{
	QString url = base_url_standard_datastore(universe_id) + "/standard-datastores/datastore/entries/entry/versions?sortOrder=Descending";
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

QNetworkRequest HttpRequestBuilder::universe_v2_get_details(const QString& api_key, const long long universe_id)
{
	const QString url = base_url_universe_v2(universe_id);
	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::use_restrictions_v2_list(const QString& api_key, const long long universe_id, const std::optional<QString>& cursor)
{
	QString url = base_url_user_restrictions_v2(universe_id);
	url = url + "?maxPageSize=100";
	if (cursor)
	{
		url = url + "&pageToken=" + QUrl::toPercentEncoding(*cursor);
	}

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QString HttpRequestBuilder::base_url_v2()
{
	return "https://apis.roblox.com/cloud/v2/";
}

QString HttpRequestBuilder::base_url_universe_v2(const long long universe_id)
{
	return base_url_v2() + "universes/" + QString::number(universe_id);
}

QString HttpRequestBuilder::base_url_memory_store_v2(const long long universe_id)
{
	return base_url_universe_v2(universe_id) + "/memory-store";
}

QString HttpRequestBuilder::base_url_ordered_datastore_v2(const long long universe_id)
{
	return base_url_universe_v2(universe_id) + "/ordered-data-stores";
}

QString HttpRequestBuilder::base_url_standard_datastore(const long long universe_id)
{
	return QString{ "https://apis.roblox.com/datastores/v1/universes/" } + QString::number(universe_id);
}

QString HttpRequestBuilder::base_url_standard_datastore_v2(const long long universe_id)
{
	return base_url_universe_v2(universe_id) + "/data-stores";
}

QString HttpRequestBuilder::base_url_user_restrictions_v2(const long long universe_id)
{
	return base_url_universe_v2(universe_id) + "/user-restrictions";
}
