#include "api_response.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "util_json_string.h"
#include "util_validator.h"

std::optional<GetStandardDatastoresResponse> GetStandardDatastoresResponse::fromJson(const QString& json)
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
		return GetStandardDatastoresResponse{ datastores_vec, cursor };
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<GetStandardDatastoreEntriesResponse> GetStandardDatastoreEntriesResponse::fromJson(const QString& json, const long long universe_id, const QString& datastore_name)
{
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
	QJsonObject root = doc.object();

	std::vector<StandardDatastoreEntry> entries_vec;
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
						entries_vec.push_back(StandardDatastoreEntry{ universe_id, datastore_name, *key_opt, *scope_opt });
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
		return GetStandardDatastoreEntriesResponse{ entries_vec, cursor };
	}
	else
	{
		return std::nullopt;
	}
}

DatastoreEntryWithDetails::DatastoreEntryWithDetails(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const QString& userids, const std::optional<QString>& attributes, const QString& data) :
	universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, key_name{ key_name }, version{ version }, userids{ userids }, attributes{ attributes }, data_raw{ data }
{
	data_decoded = data_raw;
	if (DataValidator::is_json(data))
	{
		entry_type = DatastoreEntryType::Json;
	}
	else if (std::optional<QString> decoded_string = decode_json_string(data))
	{
		data_decoded = *decoded_string;
		entry_type = DatastoreEntryType::String;
	}
	else if (DataValidator::is_number(data))
	{
		entry_type = DatastoreEntryType::Number;
	}
	else if (DataValidator::is_bool(data))
	{
		entry_type = DatastoreEntryType::Bool;
	}
	else
	{
		entry_type = DatastoreEntryType::Error;
	}
}

std::optional<GetStandardDatastoreEntryDetailsResponse> GetStandardDatastoreEntryDetailsResponse::from(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const QString& userids, const std::optional<QString>& attributes, const QString& body)
{
	return GetStandardDatastoreEntryDetailsResponse{ universe_id, datastore_name, scope, key_name, version, userids, attributes, body };
}

GetStandardDatastoreEntryDetailsResponse::GetStandardDatastoreEntryDetailsResponse(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const QString& userids, const std::optional<QString>& attributes, const QString& data) :
	details{ universe_id, datastore_name, scope, key_name, version, userids, attributes, data}
{

}

std::optional<GetStandardDatastoreEntryVersionsResponse> GetStandardDatastoreEntryVersionsResponse::from(const QString& json)
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
		return GetStandardDatastoreEntryVersionsResponse{ versions_vec, cursor };
	}
	else
	{
		return std::nullopt;
	}
}
