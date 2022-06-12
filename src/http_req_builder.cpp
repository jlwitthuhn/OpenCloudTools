#include "http_req_builder.h"

#include <string>

#include <QNetworkRequest>
#include <QUrl>

QNetworkRequest HttpRequestBuilder::delete_standard_datastore_entry(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name)
{
	QString url = datastore_base_url(universe_id) + "/standard-datastores/datastore/entries/entry";
	url = url + "?datastoreName=" + QUrl::toPercentEncoding(datastore_name);
	url = url + "&scope=" + QUrl::toPercentEncoding(scope);
	url = url + "&entryKey=" + QUrl::toPercentEncoding(key_name);

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::get_standard_datastores(const QString& api_key, const long long universe_id, std::optional<QString> cursor)
{
	QString url = datastore_base_url(universe_id) + "/standard-datastores?limit=50";
	if (cursor)
	{
		url = url + "&cursor=" + QUrl::toPercentEncoding(*cursor);
	}
	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::get_standard_datastore_entries(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& prefix, std::optional<QString> cursor)
{
	QString url = datastore_base_url(universe_id) + "/standard-datastores/datastore/entries?limit=100";
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

QNetworkRequest HttpRequestBuilder::get_standard_datastore_entry_details(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name)
{
	QString url = datastore_base_url(universe_id) + "/standard-datastores/datastore/entries/entry";
	url = url + "?datastoreName=" + QUrl::toPercentEncoding(datastore_name);
	url = url + "&scope=" + QUrl::toPercentEncoding(scope);
	url = url + "&entryKey=" + QUrl::toPercentEncoding(key_name);

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::get_standard_datastore_entry_version_details(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version)
{
	QString url = datastore_base_url(universe_id) + "/standard-datastores/datastore/entries/entry/versions/version";
	url = url + "?datastoreName=" + QUrl::toPercentEncoding(datastore_name);
	url = url + "&scope=" + QUrl::toPercentEncoding(scope);
	url = url + "&entryKey=" + QUrl::toPercentEncoding(key_name);
	url = url + "&versionId=" + QUrl::toPercentEncoding(version);

	QNetworkRequest req{ url };
	req.setRawHeader("x-api-key", api_key.toStdString().c_str());
	return req;
}

QNetworkRequest HttpRequestBuilder::get_standard_datastore_entry_versions(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, std::optional<QString> cursor)
{
	QString url = datastore_base_url(universe_id) + "/standard-datastores/datastore/entries/entry/versions?sortOrder=Descending";
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

QNetworkRequest HttpRequestBuilder::post_standard_datastore_entry(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& body_md5, const std::optional<QString>& userids, const std::optional<QString>& attributes)
{
	QString url = datastore_base_url(universe_id) + "/standard-datastores/datastore/entries/entry";
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

QString HttpRequestBuilder::datastore_base_url(const long long universe_id)
{
	return QString{ "https://apis.roblox.com/datastores/v1/universes/" } + QString::number(universe_id);
}
