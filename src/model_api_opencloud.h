#pragma once

#include <cstddef>

#include <optional>
#include <vector>

#include <QString>

#include "model_common.h"
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

class GetStandardDatastoreEntryDetailsResponse
{
public:
	static std::optional<GetStandardDatastoreEntryDetailsResponse> from(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const std::optional<QString>& userids, const std::optional<QString>& attributes, const QString& body);

	DatastoreEntryWithDetails get_details() const { return details; }

private:
	GetStandardDatastoreEntryDetailsResponse(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const std::optional<QString>& userids, const std::optional<QString>& attributes, const QString& data);

	DatastoreEntryWithDetails details;
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
