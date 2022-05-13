#pragma once

#include <optional>
#include <vector>

#include <QString>

#include "util_enum.h"

class GetStandardDatastoresResponse
{
public:
	static std::optional<GetStandardDatastoresResponse> fromJson(const QString& json);

	const std::vector<QString>& get_datastores_vec() const { return datastores_vec; }
	const std::optional<QString>& get_cursor() const { return cursor; }

private:
	GetStandardDatastoresResponse(const std::vector<QString>& datastores_vec, const std::optional<QString>& cursor) : datastores_vec{ datastores_vec }, cursor{ cursor } {}

	std::vector<QString> datastores_vec;
	std::optional<QString> cursor;
};

class StandardDatastoreEntry
{
public:
	StandardDatastoreEntry(long long universe_id, const QString& datastore_name, const QString& key, const QString& scope) : universe_id{ universe_id }, datastore_name{ datastore_name }, key{ key }, scope{ scope } {}

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

class GetStandardDatastoreEntriesResponse
{
public:
	static std::optional<GetStandardDatastoreEntriesResponse> fromJson(const QString& json, long long universe_id, const QString& datastore_name);

	const std::vector<StandardDatastoreEntry>& get_entries() const { return entries; }
	const std::optional<QString>& get_cursor() const { return cursor; }

private:
	GetStandardDatastoreEntriesResponse(const std::vector<StandardDatastoreEntry>& entries, const std::optional<QString>& cursor) : entries{ entries }, cursor{ cursor } {}

	std::vector<StandardDatastoreEntry> entries;
	std::optional<QString> cursor;
};

class DatastoreEntryWithDetails
{
public:
	DatastoreEntryWithDetails(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const QString& userids, const std::optional<QString>& attributes, const QString& data);

	long long get_universe_id() const { return universe_id; }
	const QString& get_datastore_name() const { return datastore_name; }
	const QString& get_scope() const { return scope; }
	const QString& get_key_name() const { return key_name; }
	const QString& get_version() const { return version; }
	const QString& get_userids() const { return userids; }
	const std::optional<QString>& get_attributes() const { return attributes; }
	const DatastoreEntryType get_entry_type() const { return entry_type; }
	const QString& get_data_decoded() const { return data_decoded; }
	const QString& get_data_raw() const { return data_raw; }

private:
	long long universe_id;
	QString datastore_name;
	QString scope;
	QString key_name;
	QString version;
	QString userids;
	std::optional<QString> attributes;
	DatastoreEntryType entry_type;
	QString data_decoded;
	QString data_raw;
};

class GetStandardDatastoreEntryDetailsResponse
{
public:
	static std::optional<GetStandardDatastoreEntryDetailsResponse> from(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const QString& userids, const std::optional<QString>& attributes, const QString& body);

	DatastoreEntryWithDetails get_details() const { return details; }

private:
	GetStandardDatastoreEntryDetailsResponse(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const QString& userids, const std::optional<QString>& attributes, const QString& data);

	DatastoreEntryWithDetails details;
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

class GetStandardDatastoreEntryVersionsResponse
{
public:
	static std::optional<GetStandardDatastoreEntryVersionsResponse> from(const QString& json);

	const std::vector<StandardDatastoreEntryVersion>& get_versions() const { return versions; }
	const std::optional<QString>& get_cursor() const { return cursor; }

private:
	GetStandardDatastoreEntryVersionsResponse(const std::vector<StandardDatastoreEntryVersion>& versions, const std::optional<QString>& cursor) : versions{ versions }, cursor{ cursor } {}

	std::vector<StandardDatastoreEntryVersion> versions;
	std::optional<QString> cursor;
};
