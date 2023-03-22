#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

struct sqlite3;

class DatastoreEntryWithDetails;

class SqliteDatastoreWrapper
{
public:
	static std::unique_ptr<SqliteDatastoreWrapper> new_from_path(const std::string& file_path);

	SqliteDatastoreWrapper(sqlite3* db_handle);
	~SqliteDatastoreWrapper();

	void write(const DatastoreEntryWithDetails& details);

private:
	sqlite3* db_handle = nullptr;
};

class SqliteDatastoreReader
{
public:
	static std::optional<std::vector<DatastoreEntryWithDetails>> read_all(const std::string& file_path);
};
