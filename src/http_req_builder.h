#pragma once

#include <optional>

#include <QString>

class QNetworkRequest;

class HttpRequestBuilder
{
public:
	static QNetworkRequest delete_standard_datastore_entry(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name);

	static QNetworkRequest get_standard_datastores(const QString& api_key, long long universe_id, std::optional<QString> cursor = std::nullopt);
	static QNetworkRequest get_standard_datastore_entries(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& prefix, std::optional<QString> cursor = std::nullopt);
	static QNetworkRequest get_standard_datastore_entry_details(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name);
	static QNetworkRequest get_standard_datastore_entry_version_details(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version);
	static QNetworkRequest get_standard_datastore_entry_versions(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, std::optional<QString> cursor = std::nullopt);

	static QNetworkRequest post_messaging_service_message(const QString api_key, long long universe_id, const QString& topic);
	static QNetworkRequest post_standard_datastore_entry(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& body_md5, const std::optional<QString>& userids, const std::optional<QString>& attributes);

private:
	static QString datastore_base_url(long long universe_id);
	static QString messaging_base_url(long long universe_id);
};
