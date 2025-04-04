#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

struct sqlite3;

class StandardDatastoreEntryFull;
class StandardDatastoreEntryName;

class SqliteDatastoreWrapper
{
public:
	static std::unique_ptr<SqliteDatastoreWrapper> new_from_path(const std::string& file_path);
	static std::unique_ptr<SqliteDatastoreWrapper> open_from_path(const std::string& file_path);

	SqliteDatastoreWrapper(sqlite3* db_handle);
	~SqliteDatastoreWrapper();

	bool is_correct_schema();
	bool is_resumable(long long universe_id);

	void write_deleted(const StandardDatastoreEntryName& entry);
	void write_details(const StandardDatastoreEntryFull& details);
	void write_enumeration(long long universe_id, const std::string& datastore_name, const std::optional<std::string>& cursor = std::nullopt);
	void write_enumeration_metadata(long long universe_id, const std::string& scope, const std::string& key_prefix);
	void write_pending(const StandardDatastoreEntryName& entry);

	void delete_enumeration(long long universe_id, const std::string& datastore_name);
	void delete_pending(const StandardDatastoreEntryFull& entry);
	void delete_pending(const StandardDatastoreEntryName& entry);

	std::optional<std::string> get_enumerating_cursor(long long universe_id);
	std::optional<std::string> get_enumerating_datastore(long long universe_id);

	std::optional<std::string> get_enumeration_search_key_prefix(long long universe_id);
	std::optional<std::string> get_enumeration_search_scope(long long universe_id);

	std::vector<std::string> get_pending_datastores(long long universe_id);
	std::vector<StandardDatastoreEntryName> get_pending_entries(long long universe_id);

private:
	sqlite3* db_handle = nullptr;
};

class SqliteDatastoreReader
{
public:
	static std::optional<std::vector<StandardDatastoreEntryFull>> read_all(const std::string& file_path);
};
