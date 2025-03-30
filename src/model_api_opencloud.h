#pragma once

#include <optional>
#include <vector>

#include <QString>

#include "model_common.h"

class GetMemoryStoreSortedMapItemListResponse
{
public:
	static std::optional<GetMemoryStoreSortedMapItemListResponse> from_json(long long universe_id, const QString& datastore_name, const QString& json);

	const std::vector<MemoryStoreSortedMapItem>& get_items() const { return items; }
	const std::optional<QString> get_next_page_token() const { return next_page_token; }

private:
	GetMemoryStoreSortedMapItemListResponse(const std::vector<MemoryStoreSortedMapItem>& items, const std::optional<QString>& next_page_token) :
		items{ items }, next_page_token{ next_page_token } {}

	std::vector<MemoryStoreSortedMapItem> items;
	std::optional<QString> next_page_token;
};

class GetOrderedDatastoreEntryDetailsV2Response
{
public:
	static std::optional<GetOrderedDatastoreEntryDetailsV2Response> from_json(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& json);

	OrderedDatastoreEntryFull get_details() const { return details; }

private:
	GetOrderedDatastoreEntryDetailsV2Response(const QString& path, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_id, long long value) :
		details{ path, universe_id, datastore_name, scope, key_id, value } {}

	OrderedDatastoreEntryFull details;
};

class GetOrderedDatastoreEntryListV2Response
{
public:
	static std::optional<GetOrderedDatastoreEntryListV2Response> from_json(long long universe_id, const QString& datastore_name, const QString& scope, const QString& json);

	const std::vector<OrderedDatastoreEntryFull>& get_entries() const { return entries; }
	const std::optional<QString> get_next_page_token() const { return next_page_token; }

private:
	GetOrderedDatastoreEntryListV2Response(const std::vector<OrderedDatastoreEntryFull>& entries, const std::optional<QString>& next_page_token) :
		entries{ entries }, next_page_token{ next_page_token } {
	}

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

class GetStandardDatastoreEntryVersionListResponse
{
public:
	static std::optional<GetStandardDatastoreEntryVersionListResponse> from(const QString& json);

	const std::vector<StandardDatastoreEntryVersion>& get_versions() const { return versions; }
	const std::optional<QString>& get_cursor() const { return cursor; }

private:
	GetStandardDatastoreEntryVersionListResponse(const std::vector<StandardDatastoreEntryVersion>& versions, const std::optional<QString>& cursor) : versions{ versions }, cursor{ cursor } {}

	std::vector<StandardDatastoreEntryVersion> versions;
	std::optional<QString> cursor;
};

class GetUniverseDetailsResponse
{
public:
	static std::optional<GetUniverseDetailsResponse> from(const QString& json);

	const QString& get_display_name() const { return display_name; }

private:
	GetUniverseDetailsResponse(const QString& display_name) : display_name{ display_name } {}

	QString display_name;
};

class GetUserRestrictionListV2Response
{
public:
	static std::optional<GetUserRestrictionListV2Response> from(const QString& json);

	const std::vector<BanListUserRestriction>& get_restrictions() const { return restrictions; }
	const std::optional<QString>& get_next_page_token() const { return next_page_token; }

private:
	GetUserRestrictionListV2Response(const std::vector<BanListUserRestriction>& restrictions, const std::optional<QString>& next_page_token) : restrictions{ restrictions }, next_page_token{ next_page_token } {}

	std::vector<BanListUserRestriction> restrictions;
	std::optional<QString> next_page_token;
};

class PostStandardDatastoreSnapshotResponseV2
{
public:
	static std::optional<PostStandardDatastoreSnapshotResponseV2> from(const QString& json);

	bool get_new_snapshot_taken() const { return new_snapshot_taken; }
	const QString& get_latest_snapshot_time() const { return latest_snapshot_time; }

private:
	PostStandardDatastoreSnapshotResponseV2(bool new_snapshot_taken, const QString& last_snapshot_time)
		: new_snapshot_taken{ new_snapshot_taken }, latest_snapshot_time{ last_snapshot_time } {}

	bool new_snapshot_taken = false;
	QString latest_snapshot_time;
};
