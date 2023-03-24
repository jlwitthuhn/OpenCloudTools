#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

struct sqlite3;

class DatastoreEntryWithDetails;
class StandardDatastoreEntry;

class SqliteDatastoreWrapper
{
public:
	static std::unique_ptr<SqliteDatastoreWrapper> new_from_path(const std::string& file_path);

	SqliteDatastoreWrapper(sqlite3* db_handle);
	~SqliteDatastoreWrapper();

	bool is_correct_schema();

	void write_deleted(const StandardDatastoreEntry& entry);
	void write_details(const DatastoreEntryWithDetails& details);
	void write_enumeration(long long universe_id, const std::string& datastore_name, const std::optional<std::string> cursor = std::nullopt);
	void write_pending(const StandardDatastoreEntry& entry);

	void delete_enumeration(long long universe_id, const std::string& datastore_name);
	void delete_pending(const DatastoreEntryWithDetails& entry);
	void delete_pending(const StandardDatastoreEntry& entry);

private:
	sqlite3* db_handle = nullptr;
};

class SqliteDatastoreReader
{
public:
	static std::optional<std::vector<DatastoreEntryWithDetails>> read_all(const std::string& file_path);
};
