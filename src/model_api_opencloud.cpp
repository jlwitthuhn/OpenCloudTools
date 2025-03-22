#include "model_api_opencloud.h"

#include <cstddef>

#include <QtGlobal>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "assert.h"
#include "util_json.h"

static std::optional<bool> extract_bool(const QJsonObject& obj, const QString& name)
{
	const QJsonObject::const_iterator iter = obj.find(name);
	if (iter == obj.end())
	{

	}
	if (iter->isBool() == false)
	{
		return std::nullopt;
	}
	return iter->toBool();
}

static std::optional<JsonValue> extract_json(QJsonObject& obj, const QString& name)
{
	const QJsonObject::iterator iter = obj.find(name);
	if (iter != obj.end())
	{
		return JsonValue::from_qjson(*iter);
	}
	return std::nullopt;
}

static std::optional<double> extract_number(const QJsonObject& obj, const QString& name)
{
	const QJsonObject::const_iterator iter = obj.find(name);
	if (iter != obj.end())
	{
		if (iter->isDouble())
		{
			return iter->toDouble();
		}
	}
	return std::nullopt;
}

static std::optional<QString> extract_string(const QJsonObject& obj, const QString& name)
{
	const QJsonObject::const_iterator iter = obj.find(name);
	if (iter != obj.end())
	{
		if (iter->isString())
		{
			return iter->toString();
		}
	}
	return std::nullopt;
}

std::optional<GetMemoryStoreSortedMapItemListResponse> GetMemoryStoreSortedMapItemListResponse::from_json(const long long universe_id, const QString& map_name, const QString& json)
{
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
	QJsonObject root = doc.object();

	std::vector<MemoryStoreSortedMapItem> items;
	QJsonObject::iterator items_it = root.find("items");
	if (items_it != root.end())
	{
		QJsonValueRef items_value = items_it.value();
		if (items_value.isArray())
		{
			for (QJsonValueRef this_entry : items_value.toArray())
			{
				if (this_entry.isObject())
				{
					QJsonObject this_entry_obj = this_entry.toObject();

					const std::optional<QString> opt_path = extract_string(this_entry_obj, "path");
					const std::optional<JsonValue> opt_value = extract_json(this_entry_obj, "value");
					const std::optional<QString> opt_etag = extract_string(this_entry_obj, "etag");
					const std::optional<QString> opt_expire = extract_string(this_entry_obj, "expireTime");
					const std::optional<QString> opt_id = extract_string(this_entry_obj, "id");

					const std::optional<QString> opt_sort_string = extract_string(this_entry_obj, "stringSortKey");
					const std::optional<double> opt_sort_number = extract_number(this_entry_obj, "numericSortKey");

					if (opt_path && opt_value && opt_etag && opt_expire && opt_id)
					{
						items.push_back(MemoryStoreSortedMapItem{
							universe_id,
							map_name,
							*opt_path,
							*opt_value,
							*opt_etag,
							*opt_expire,
							*opt_id,
							opt_sort_string,
							opt_sort_number
						});
					}
				}
			}
		}

		const std::optional<QString> opt_next_page_token = extract_string(root, "nextPageToken");

		if (items.size() > 0 || opt_next_page_token)
		{
			return GetMemoryStoreSortedMapItemListResponse{ items, opt_next_page_token };
		}
	}

	return std::nullopt;
}

std::optional<GetOrderedDatastoreEntryDetailsResponse> GetOrderedDatastoreEntryDetailsResponse::from_json(long long universe_id, const QString& datastore_name, const QString& scope, [[maybe_unused]] const QString& key_name, const QString& json)
{
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
	QJsonObject root = doc.object();

	std::optional<QString> path_opt;
	QJsonObject::iterator path_it = root.find("path");
	if (path_it != root.end())
	{
		if (path_it.value().isString())
		{
			path_opt = path_it.value().toString();
		}
	}

	std::optional<QString> id_opt;
	QJsonObject::iterator id_it = root.find("id");
	if (id_it != root.end())
	{
		if (id_it.value().isString())
		{
			id_opt = id_it.value().toString();
		}
	}

	std::optional<long long> value_opt;
	QJsonObject::iterator value_it = root.find("value");
	if (value_it != root.end())
	{
		if (value_it.value().isDouble())
		{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			value_opt = value_it.value().toInteger();
#else
			value_opt = static_cast<long long>(value_it.value().toDouble());
#endif
		}
		else if (value_it.value().isString())
		{
			value_opt = value_it.value().toString().toLongLong();
		}
	}

	if (path_opt && id_opt && value_opt)
	{
		OCTASSERT(*id_opt == key_name);
		return GetOrderedDatastoreEntryDetailsResponse{ *path_opt, universe_id, datastore_name, scope, *id_opt, *value_opt };
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<GetOrderedDatastoreEntryListV2Response> GetOrderedDatastoreEntryListV2Response::from_json(const long long universe_id, const QString& datastore_name, const QString& scope, const QString& json)
{
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
	QJsonObject root = doc.object();

	std::vector<OrderedDatastoreEntryFull> entries;
	QJsonObject::iterator entries_it = root.find("orderedDataStoreEntries");
	if (entries_it != root.end())
	{
		QJsonValueRef entries_value = entries_it.value();
		if (entries_value.isArray())
		{
			for (QJsonValueRef this_entry : entries_value.toArray())
			{
				if (this_entry.isObject())
				{
					QJsonObject this_entry_obj = this_entry.toObject();

					std::optional<QString> path_opt;
					QJsonObject::iterator path_it = this_entry_obj.find("path");
					if (path_it != this_entry_obj.end())
					{
						if (path_it.value().isString())
						{
							path_opt = path_it.value().toString();
						}
					}

					std::optional<QString> id_opt;
					QJsonObject::iterator id_it = this_entry_obj.find("id");
					if (id_it != this_entry_obj.end())
					{
						if (id_it.value().isString())
						{
							id_opt = id_it.value().toString();
						}
					}

					std::optional<long long> value_opt;
					QJsonObject::iterator value_it = this_entry_obj.find("value");
					if (value_it != this_entry_obj.end())
					{
						if (value_it.value().isDouble())
						{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
							value_opt = value_it.value().toInteger();
#else
							value_opt = static_cast<long long>(value_it.value().toDouble());
#endif
						}
						else if (value_it.value().isString())
						{
							value_opt = value_it.value().toString().toLongLong();
						}
					}

					if (path_opt && id_opt && value_opt)
					{
						entries.push_back(OrderedDatastoreEntryFull{ *path_opt, universe_id, datastore_name, scope, *id_opt, *value_opt });
					}
				}
			}
		}
	}

	std::optional<QString> page_token;
	QJsonObject::iterator page_token_it = root.find("nextPageToken");
	if (page_token_it != root.end())
	{
		QJsonValueRef page_token_value = page_token_it.value();
		if (page_token_value.isString())
		{
			page_token = page_token_value.toString();
		}
	}

	return GetOrderedDatastoreEntryListV2Response{ entries, page_token };
}

std::optional<GetStandardDatastoreListResponse> GetStandardDatastoreListResponse::from_json(const QString& json)
{
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
	QJsonObject root = doc.object();

	std::vector<QString> datastores_vec;
	QJsonObject::iterator datastores_it = root.find("datastores");
	if (datastores_it != root.end())
	{
		QJsonValueRef datastores_value = datastores_it.value();
		if (datastores_value.isArray())
		{
			for (QJsonValueRef this_val : datastores_value.toArray())
			{
				if (this_val.isObject())
				{
					QJsonObject this_obj = this_val.toObject();

					QJsonObject::iterator datastore_name_it = this_obj.find("name");
					if (datastore_name_it != this_obj.end())
					{
						if (datastore_name_it.value().isString())
						{
							datastores_vec.push_back(datastore_name_it.value().toString());
						}
					}
				}
			}
		}
	}

	std::optional<QString> cursor;
	QJsonObject::iterator cursor_it = root.find("nextPageCursor");
	if (cursor_it != root.end())
	{
		QJsonValueRef cursor_value = cursor_it.value();
		if (cursor_value.isString())
		{
			cursor = cursor_value.toString();
		}
	}

	if (datastores_vec.size() > 0)
	{
		return GetStandardDatastoreListResponse{ datastores_vec, cursor };
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<GetStandardDatastoreEntryListResponse> GetStandardDatastoreEntryListResponse::from_json(const QString& json, const long long universe_id, const QString& datastore_name)
{
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
	QJsonObject root = doc.object();

	std::vector<StandardDatastoreEntryName> entries_vec;
	QJsonObject::iterator keys_it = root.find("keys");
	if (keys_it != root.end())
	{
		QJsonValueRef keys_value = keys_it.value();
		if (keys_value.isArray())
		{
			for (QJsonValueRef this_val : keys_value.toArray())
			{
				if (this_val.isObject())
				{
					QJsonObject this_entry_obj = this_val.toObject();

					std::optional<QString> scope_opt;
					QJsonObject::iterator scope_it = this_entry_obj.find("scope");
					if (scope_it != this_entry_obj.end())
					{
						if (scope_it.value().isString())
						{
							scope_opt = scope_it.value().toString();
						}
					}

					std::optional<QString> key_opt;
					QJsonObject::iterator key_it = this_entry_obj.find("key");
					if (key_it != this_entry_obj.end())
					{
						if (key_it.value().isString())
						{
							key_opt = key_it.value().toString();
						}
					}

					if (scope_opt && key_opt)
					{
						entries_vec.push_back(StandardDatastoreEntryName{ universe_id, datastore_name, *key_opt, *scope_opt });
					}
				}
			}
		}
	}

	std::optional<QString> cursor;
	QJsonObject::iterator cursor_it = root.find("nextPageCursor");
	if (cursor_it != root.end())
	{
		QJsonValueRef cursor_value = cursor_it.value();
		if (cursor_value.isString())
		{
			cursor = cursor_value.toString();
		}
	}

	if (entries_vec.size() > 0)
	{
		return GetStandardDatastoreEntryListResponse{ entries_vec, cursor };
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<GetStandardDatastoreEntryDetailsResponse> GetStandardDatastoreEntryDetailsResponse::from(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const std::optional<QString>& userids, const std::optional<QString>& attributes, const QString& body)
{
	return GetStandardDatastoreEntryDetailsResponse{ universe_id, datastore_name, scope, key_name, version, userids, attributes, body };
}

GetStandardDatastoreEntryDetailsResponse::GetStandardDatastoreEntryDetailsResponse(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const std::optional<QString>& userids, const std::optional<QString>& attributes, const QString& data) :
	details{ universe_id, datastore_name, scope, key_name, version, userids, attributes, data}
{

}

std::optional<GetStandardDatastoreEntryVersionListResponse> GetStandardDatastoreEntryVersionListResponse::from(const QString& json)
{
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
	QJsonObject root = doc.object();

	std::vector<StandardDatastoreEntryVersion> versions_vec;
	QJsonObject::iterator versions_it = root.find("versions");
	if (versions_it != root.end())
	{
		QJsonValueRef versions_value = versions_it.value();
		if (versions_value.isArray())
		{
			for (QJsonValueRef this_val : versions_value.toArray())
			{
				if (this_val.isObject())
				{
					QJsonObject this_entry_obj = this_val.toObject();

					std::optional<QString> version_opt;
					{
						QJsonObject::iterator version_it = this_entry_obj.find("version");
						if (version_it != this_entry_obj.end())
						{
							if (version_it.value().isString())
							{
								version_opt = version_it.value().toString();
							}
						}
					}

					std::optional<bool> deleted_opt;
					{
						QJsonObject::iterator deleted_it = this_entry_obj.find("deleted");
						if (deleted_it != this_entry_obj.end())
						{
							if (deleted_it.value().isBool())
							{
								deleted_opt = deleted_it.value().toBool();
							}
						}
					}

					std::optional<size_t> content_length_opt;
					{
						QJsonObject::iterator content_length_it = this_entry_obj.find("contentLength");
						if (content_length_it != this_entry_obj.end())
						{
							if (content_length_it.value().isDouble())
							{
								content_length_opt = static_cast<size_t>(content_length_it.value().toDouble());
							}
						}
					}

					std::optional<QString> created_time_opt;
					{
						QJsonObject::iterator created_time_it = this_entry_obj.find("createdTime");
						if (created_time_it != this_entry_obj.end())
						{
							if (created_time_it.value().isString())
							{
								created_time_opt = created_time_it.value().toString();
							}
						}
					}

					std::optional<QString> object_created_time_opt;
					{
						QJsonObject::iterator object_created_time_it = this_entry_obj.find("objectCreatedTime");
						if (object_created_time_it != this_entry_obj.end())
						{
							if (object_created_time_it.value().isString())
							{
								object_created_time_opt = object_created_time_it.value().toString();
							}
						}
					}

					if (version_opt && deleted_opt && content_length_opt && created_time_opt && object_created_time_opt)
					{
						versions_vec.push_back(StandardDatastoreEntryVersion{ *version_opt, *deleted_opt, *content_length_opt, *created_time_opt, *object_created_time_opt });
					}
				}
			}
		}
	}

	std::optional<QString> cursor;
	QJsonObject::iterator cursor_it = root.find("nextPageCursor");
	if (cursor_it != root.end())
	{
		QJsonValueRef cursor_value = cursor_it.value();
		if (cursor_value.isString())
		{
			cursor = cursor_value.toString();
		}
	}

	if (versions_vec.size() > 0)
	{
		return GetStandardDatastoreEntryVersionListResponse{ versions_vec, cursor };
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<GetUniverseDetailsResponse> GetUniverseDetailsResponse::from(const QString& json)
{
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
	QJsonObject root = doc.object();

	QJsonObject::iterator display_name_it = root.find("displayName");
	std::optional<QString> display_name;
	if (display_name_it != root.end())
	{
		if (display_name_it->isString())
		{
			display_name = display_name_it->toString();
		}
	}

	if (display_name)
	{
		return GetUniverseDetailsResponse(*display_name);
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<PostStandardDatastoreSnapshotResponseV2> PostStandardDatastoreSnapshotResponseV2::from(const QString& json)
{
	const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
	const QJsonObject root = doc.object();

	const std::optional<bool> new_snapshot = extract_bool(root, "newSnapshotTaken");
	const std::optional<QString> latest_snapshot = extract_string(root, "latestSnapshotTime");

	if (!new_snapshot || !latest_snapshot)
	{
		return std::nullopt;
	}

	return PostStandardDatastoreSnapshotResponseV2{ *new_snapshot, *latest_snapshot };
}
