#include "sqlite_wrapper.h"

#include <optional>

#include <QString>

#include <sqlite3.h>

#include "api_response.h"
#include "util_enum.h"

std::unique_ptr<SqliteDatastoreWriter> SqliteDatastoreWriter::from_path(const std::string& file_path)
{
	sqlite3* db_handle = nullptr;
	if (sqlite3_open(file_path.c_str(), &db_handle) != SQLITE_OK)
	{
		return nullptr;
	}

	sqlite3_exec(db_handle, "DROP TABLE IF EXISTS datastore;", nullptr, nullptr, nullptr);
	sqlite3_exec(db_handle, "CREATE TABLE datastore (id INTEGER PRIMARY KEY, universe_id INTEGER NOT NULL, datastore_name TEXT NOT NULL, scope TEXT NOT NULL, key_name TEXT NOT NULL, version TEXT NOT NULL, data_type TEXT NOT NULL, data_raw TEXT NOT NULL, data_str TEXT, data_num REAL, userids TEXT, attributes TEXT)", nullptr, nullptr, nullptr);

	return std::make_unique<SqliteDatastoreWriter>(db_handle);
}

SqliteDatastoreWriter::SqliteDatastoreWriter(sqlite3* db_handle) : db_handle{ db_handle }
{

}

SqliteDatastoreWriter::~SqliteDatastoreWriter()
{
	if (db_handle != nullptr)
	{
		sqlite3_close(db_handle);
		db_handle = nullptr;
	}
}

void SqliteDatastoreWriter::write(const DatastoreEntryWithDetails& details)
{
	if (db_handle != nullptr)
	{
		sqlite3_stmt* stmt = nullptr;
		std::string sql = "INSERT INTO datastore (universe_id, datastore_name, scope, key_name, version, data_type, data_raw, data_str, data_num, userids, attributes) VALUES (?010, ?020, ?030, ?040, ?050, ?060, ?070, ?080, ?090, ?100, ?110);";
		sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
		if (stmt != nullptr)
		{
			sqlite3_bind_int64(stmt, 10, details.get_universe_id());
			sqlite3_bind_text(stmt, 20, details.get_datastore_name().toStdString().c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 30, details.get_scope().toStdString().c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 40, details.get_key_name().toStdString().c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 50, details.get_version().toStdString().c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 60, get_enum_string(details.get_entry_type()).toStdString().c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 70, details.get_data_raw().toStdString().c_str(), -1, SQLITE_TRANSIENT);
			if (details.get_entry_type() == DatastoreEntryType::String)
			{
				sqlite3_bind_text(stmt, 80, details.get_data_decoded().toStdString().c_str(), -1, SQLITE_TRANSIENT);
			}
			bool is_double = false;
			const double as_double = details.get_data_raw().toDouble(&is_double);
			if (is_double)
			{
				sqlite3_bind_double(stmt, 90, as_double);
			}
			if (details.get_userids())
			{
				sqlite3_bind_text(stmt, 100, details.get_userids()->toStdString().c_str(), -1, SQLITE_TRANSIENT);
			}
			if (details.get_attributes())
			{
				sqlite3_bind_text(stmt, 110, details.get_attributes()->toStdString().c_str(), -1, SQLITE_TRANSIENT);
			}

			sqlite3_step(stmt);

			sqlite3_finalize(stmt);
		}
	}
}
