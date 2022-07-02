#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "api_response.h"

struct sqlite3;

class DatastoreEntryWithDetails;

class SqliteDatastoreWriter
{
public:
	static std::unique_ptr<SqliteDatastoreWriter> from_path(const std::string& file_path);

	SqliteDatastoreWriter(sqlite3* db_handle);
	~SqliteDatastoreWriter();

	void write(const DatastoreEntryWithDetails& details);

private:
	sqlite3* db_handle = nullptr;
};

class SqliteDatastoreReader
{
public:
	static std::optional<std::vector<DatastoreEntryWithDetails>> read_all(const std::string& file_path);
};
