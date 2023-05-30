#pragma once

#include <QString>

#include "util_enum.h"

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

class DatastoreEntryWithDetails
{
public:
	DatastoreEntryWithDetails(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const std::optional<QString>& userids, const std::optional<QString>& attributes, const QString& data);

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
