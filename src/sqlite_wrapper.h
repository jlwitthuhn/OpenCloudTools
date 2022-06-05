#pragma once

#include <memory>
#include <string>

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
