#pragma once

#include <optional>
#include <vector>

#include <QString>

#include "model_common.h"

class GetOrderedDatastoreEntryDetailsResponse
{
public:
	static std::optional<GetOrderedDatastoreEntryDetailsResponse> from_json(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& json);

	OrderedDatastoreEntryFull get_details() const { return details; }

private:
	GetOrderedDatastoreEntryDetailsResponse(const QString& path, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_id, long long value) :
		details{ path, universe_id, datastore_name, scope, key_id, value } {}

	OrderedDatastoreEntryFull details;
};

class GetOrderedDatastoreEntryListResponse
{
public:
	static std::optional<GetOrderedDatastoreEntryListResponse> from_json(long long universe_id, const QString& datastore_name, const QString& scope, const QString& json);

	const std::vector<OrderedDatastoreEntryFull>& get_entries() const { return entries; }
	const std::optional<QString> get_next_page_token() const { return next_page_token; }

private:
	GetOrderedDatastoreEntryListResponse(const std::vector<OrderedDatastoreEntryFull>& entries, const std::optional<QString>& next_page_token) :
		entries{ entries }, next_page_token{ next_page_token } {}

	std::vector<OrderedDatastoreEntryFull> entries;
	std::optional<QString> next_page_token;
};

class GetStandardDatastoreListResponse
{
public:
	static std::optional<GetStandardDatastoreListResponse> from_json(const QString& json);

	const std::vector<QString>& get_datastores_vec() const { return datastores_vec; }
	const std::optional<QString>& get_cursor() const { return cursor; }

private:
	GetStandardDatastoreListResponse(const std::vector<QString>& datastores_vec, const std::optional<QString>& cursor) : datastores_vec{ datastores_vec }, cursor{ cursor } {}

	std::vector<QString> datastores_vec;
	std::optional<QString> cursor;
};

class GetStandardDatastoreEntryListResponse
{
public:
	static std::optional<GetStandardDatastoreEntryListResponse> from_json(const QString& json, long long universe_id, const QString& datastore_name);

	const std::vector<StandardDatastoreEntryName>& get_entries() const { return entries; }
	const std::optional<QString>& get_cursor() const { return cursor; }

private:
	GetStandardDatastoreEntryListResponse(const std::vector<StandardDatastoreEntryName>& entries, const std::optional<QString>& cursor) : entries{ entries }, cursor{ cursor } {}

	std::vector<StandardDatastoreEntryName> entries;
	std::optional<QString> cursor;
};

class GetStandardDatastoreEntryDetailsResponse
{
public:
	static std::optional<GetStandardDatastoreEntryDetailsResponse> from(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const std::optional<QString>& userids, const std::optional<QString>& attributes, const QString& body);

	StandardDatastoreEntryFull get_details() const { return details; }

private:
	GetStandardDatastoreEntryDetailsResponse(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const std::optional<QString>& userids, const std::optional<QString>& attributes, const QString& data);

	StandardDatastoreEntryFull details;
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
