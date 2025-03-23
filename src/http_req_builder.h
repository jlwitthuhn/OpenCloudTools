#pragma once

#include <optional>

#include <QString>

class QNetworkRequest;

class HttpRequestBuilder
{
public:
	static QNetworkRequest memory_store_v2_sorted_map_get_list(const QString& api_key, long long universe_id, const QString& map_name, bool ascending, std::optional<QString> cursor = std::nullopt);

	static QNetworkRequest messaging_service_v2_post_message(const QString api_key, long long universe_id);

	static QNetworkRequest ordered_datastore_v2_entry_delete(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id);
	static QNetworkRequest ordered_datastore_v2_entry_get_details(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name);
	static QNetworkRequest ordered_datastore_v2_entry_get_list(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, bool ascending, std::optional<QString> cursor = std::nullopt);
	static QNetworkRequest ordered_datastore_v2_entry_patch_update(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id, const QString& body_md5);
	static QNetworkRequest ordered_datastore_v2_entry_post_create(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id, const QString& body_md5);
	static QNetworkRequest ordered_datastore_v2_entry_post_increment(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& entry_id, const QString& body_md5);

	static QNetworkRequest standard_datastore_get_list(const QString& api_key, long long universe_id, std::optional<QString> cursor = std::nullopt);
	static QNetworkRequest standard_datastore_v2_snapshot(const QString& api_key, long long universe_id);

	static QNetworkRequest standard_datastore_entry_delete(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name);
	static QNetworkRequest standard_datastore_entry_get_details(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name);
	static QNetworkRequest standard_datastore_entry_get_list(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& prefix, std::optional<QString> cursor = std::nullopt);
	static QNetworkRequest standard_datastore_entry_post(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& body_md5, const std::optional<QString>& userids, const std::optional<QString>& attributes);

	static QNetworkRequest standard_datastore_entry_version_get_details(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version);
	static QNetworkRequest standard_datastore_entry_version_get_list(const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, std::optional<QString> cursor = std::nullopt);

	static QNetworkRequest universe_v2_get_details(const QString& api_key, long long universe_id);

private:
	static QString base_url_memory_store_v2(long long universe_id);
	static QString base_url_ordered_datastore_v2(long long universe_id);
	static QString base_url_standard_datastore(long long universe_id);
	static QString base_url_standard_datastore_v2(long long universe_id);
	static QString base_url_universe_v2(long long universe_id);
};
