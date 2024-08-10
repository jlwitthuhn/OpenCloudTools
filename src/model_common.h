#pragma once

#include <cstddef>
#include <optional>

#include <QString>

#include "util_json.h"

enum class DatastoreEntryType;

class MemoryStoreSortedMapItem
{
public:
	MemoryStoreSortedMapItem(
		long long universe_id,
		const QString& map_name,
		const QString& path,
		const JsonValue& value,
		const QString& etag,
		const QString& expire_time,
		const QString& id,
		const std::optional<QString>& string_sort_key,
		const std::optional<double>& numeric_sort_key)
		:
		universe_id{ universe_id },
		map_name{ map_name },
		path{ path },
		value{ value },
		etag{ etag },
		expire_time{ expire_time },
		id{ id },
		string_sort_key{ string_sort_key },
		numeric_sort_key{ numeric_sort_key }
	{}

private:
	long long universe_id;
	QString map_name;
	QString path;
	JsonValue value;
	QString etag;
	QString expire_time;
	QString id;
	std::optional<QString> string_sort_key;
	std::optional<double> numeric_sort_key;
};

class OrderedDatastoreEntryFull
{
public:
	OrderedDatastoreEntryFull(const QString& path, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, long long value) :
		path{ path }, universe_id { universe_id }, datastore_name{ datastore_name }, scope{ scope }, key_name{ key_name }, value{ value } {}

	const QString& get_path() const { return path; }
	long long get_universe_id() const { return universe_id; }
	const QString& get_datastore_name() const { return datastore_name; }
	const QString& get_scope() const { return scope; }
	const QString& get_key_name() const { return key_name; }
	long long get_value() const { return value; }

private:
	QString path;
	long long universe_id;
	QString datastore_name;
	QString scope;
	QString key_name;
	long long value;
};

class StandardDatastoreEntryFull
{
public:
	StandardDatastoreEntryFull(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const std::optional<QString>& userids, const std::optional<QString>& attributes, const QString& data);

	long long get_universe_id() const { return universe_id; }
	const QString& get_datastore_name() const { return datastore_name; }
	const QString& get_scope() const { return scope; }
	const QString& get_key_name() const { return key_name; }
	const QString& get_version() const { return version; }
	const std::optional<QString>& get_userids() const { return userids; }
	const std::optional<QString>& get_attributes() const { return attributes; }
	DatastoreEntryType get_entry_type() const { return entry_type; }
	const QString& get_data_decoded() const { return data_decoded; }
	const QString& get_data_raw() const { return data_raw; }

private:
	long long universe_id;
	QString datastore_name;
	QString scope;
	QString key_name;
	QString version;
	std::optional<QString> userids;
	std::optional<QString> attributes;
	DatastoreEntryType entry_type;
	QString data_decoded;
	QString data_raw;
};

class StandardDatastoreEntryName
{
public:
	StandardDatastoreEntryName(long long universe_id, const QString& datastore_name, const QString& key, const QString& scope) :
		universe_id{ universe_id }, datastore_name{ datastore_name }, key{ key }, scope{ scope } {}

	long long get_universe_id() const { return universe_id; }
	const QString& get_datastore_name() const { return datastore_name; }
	const QString& get_key() const { return key; }
	const QString& get_scope() const { return scope; }

private:
	long long universe_id;
	QString datastore_name;
	QString key;
	QString scope;
};

class StandardDatastoreEntryVersion
{
public:
	StandardDatastoreEntryVersion(const QString& version, bool deleted, size_t content_length, const QString& created_time, const QString& object_created_time) :
		version{ version }, deleted{ deleted }, content_length{ content_length }, created_time{ created_time }, object_created_time{ object_created_time }
	{}

	const QString& get_version() const { return version; }
	bool get_deleted() const { return deleted; }
	size_t get_content_length() const { return content_length; }
	const QString& get_created_time() const { return created_time; }
	const QString& get_object_created_time() const { return object_created_time; }

private:
	QString version;
	bool deleted;
	size_t content_length;
	QString created_time;
	QString object_created_time;
};
