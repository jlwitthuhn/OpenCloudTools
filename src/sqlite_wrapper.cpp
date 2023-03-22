#include "sqlite_wrapper.h"

#include <optional>

#include <QString>

#include <sqlite3.h>

#include "api_response.h"
#include "util_enum.h"

std::unique_ptr<SqliteDatastoreWrapper> SqliteDatastoreWrapper::new_from_path(const std::string& file_path)
{
	sqlite3* db_handle = nullptr;
	if (sqlite3_open(file_path.c_str(), &db_handle) != SQLITE_OK)
	{
		return nullptr;
	}

	// Table to store raw datastore data
	sqlite3_exec(db_handle, "DROP TABLE IF EXISTS datastore;", nullptr, nullptr, nullptr);
	sqlite3_exec(db_handle, "CREATE TABLE datastore (id INTEGER PRIMARY KEY, universe_id INTEGER NOT NULL, datastore_name TEXT NOT NULL, scope TEXT NOT NULL, key_name TEXT NOT NULL, version TEXT NOT NULL, data_type TEXT NOT NULL, data_raw TEXT NOT NULL, data_str TEXT, data_num REAL, data_bool INTEGER, userids TEXT, attributes TEXT)", nullptr, nullptr, nullptr);

	// Table to enumerate what entries remain to be downloaded
	sqlite3_exec(db_handle, "DROP TABLE IF EXISTS datastore_pending;", nullptr, nullptr, nullptr);
	sqlite3_exec(db_handle, "CREATE TABLE datastore_pending (id INTEGER PRIMARY KEY, universe_id INTEGER NOT NULL, datastore_name TEXT NOT NULL, scope TEXT NOT NULL, key_name TEXT NOT NULL)", nullptr, nullptr, nullptr);

	return std::make_unique<SqliteDatastoreWrapper>(db_handle);
}

SqliteDatastoreWrapper::SqliteDatastoreWrapper(sqlite3* db_handle) : db_handle{ db_handle }
{

}

SqliteDatastoreWrapper::~SqliteDatastoreWrapper()
{
	if (db_handle != nullptr)
	{
		sqlite3_close(db_handle);
		db_handle = nullptr;
	}
}

void SqliteDatastoreWrapper::write_details(const DatastoreEntryWithDetails& details)
{
	if (db_handle != nullptr)
	{
		sqlite3_stmt* stmt = nullptr;
		const std::string sql = "INSERT INTO datastore (universe_id, datastore_name, scope, key_name, version, data_type, data_raw, data_str, data_num, data_bool, userids, attributes) VALUES (?010, ?020, ?030, ?040, ?050, ?060, ?070, ?080, ?090, ?095, ?100, ?110);";
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
			if (details.get_entry_type() == DatastoreEntryType::Bool)
			{
				const int value = details.get_data_raw() == "true" ? 1 : 0;
				sqlite3_bind_int(stmt, 95, value);
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

void SqliteDatastoreWrapper::write_pending(const StandardDatastoreEntry& entry)
{
	if (db_handle != nullptr)
	{
		sqlite3_stmt* stmt = nullptr;
		const std::string sql = "INSERT INTO datastore_pending (universe_id, datastore_name, scope, key_name) VALUES (?010, ?020, ?030, ?040);";
		sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
		if (stmt != nullptr)
		{
			sqlite3_bind_int64(stmt, 10, entry.get_universe_id());
			sqlite3_bind_text(stmt, 20, entry.get_datastore_name().toStdString().c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 30, entry.get_scope().toStdString().c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 40, entry.get_key().toStdString().c_str(), -1, SQLITE_TRANSIENT);

			sqlite3_step(stmt);

			sqlite3_finalize(stmt);
		}
	}
}

void SqliteDatastoreWrapper::delete_pending(const DatastoreEntryWithDetails& details)
{
	const StandardDatastoreEntry entry{ details.get_universe_id(), details.get_datastore_name(), details.get_key_name(), details.get_scope() };
	delete_pending(entry);
}

void SqliteDatastoreWrapper::delete_pending(const StandardDatastoreEntry& entry)
{
	if (db_handle != nullptr)
	{
		sqlite3_stmt* stmt = nullptr;
		const std::string sql = "DELETE FROM datastore_pending WHERE universe_id = ?010 AND datastore_name = ?020 AND scope = ?030 AND key_name = ?040;";
		sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
		if (stmt != nullptr)
		{
			sqlite3_bind_int64(stmt, 10, entry.get_universe_id());
			sqlite3_bind_text(stmt, 20, entry.get_datastore_name().toStdString().c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 30, entry.get_scope().toStdString().c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 40, entry.get_key().toStdString().c_str(), -1, SQLITE_TRANSIENT);

			sqlite3_step(stmt);

			sqlite3_finalize(stmt);
		}
	}
}

std::optional<std::vector<DatastoreEntryWithDetails>> SqliteDatastoreReader::read_all(const std::string& file_path)
{
	sqlite3* db_handle = nullptr;
	if (sqlite3_open(file_path.c_str(), &db_handle) != SQLITE_OK)
	{
		return std::nullopt;
	}

	std::vector<DatastoreEntryWithDetails> result;
	{
		const std::string sql = "SELECT universe_id, datastore_name, scope, key_name, version, data_type, data_raw, data_str, data_num, data_bool, userids, attributes FROM datastore;";

		sqlite3_stmt* stmt = nullptr;
		sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

		while (true)
		{
			const int sqlite_result = sqlite3_step(stmt);
			if (sqlite_result == SQLITE_ROW)
			{
				std::optional<long long> opt_universe_id;
				if (sqlite3_column_type(stmt, 0) == SQLITE_INTEGER)
				{
					opt_universe_id = sqlite3_column_int64(stmt, 0);
				}

				std::optional<QString> opt_datastore_name;
				if (sqlite3_column_type(stmt, 1) == SQLITE_TEXT)
				{
					opt_datastore_name = QString{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) };
				}

				std::optional<QString> opt_scope;
				if (sqlite3_column_type(stmt, 2) == SQLITE_TEXT)
				{
					opt_scope = QString{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) };
				}

				std::optional<QString> opt_key_name;
				if (sqlite3_column_type(stmt, 3) == SQLITE_TEXT)
				{
					opt_key_name = QString{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) };
				}

				std::optional<QString> opt_version;
				if (sqlite3_column_type(stmt, 4) == SQLITE_TEXT)
				{
					opt_version = QString{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) };
				}

				// Slot 5 is data_type

				std::optional<QString> opt_data_raw;
				if (sqlite3_column_type(stmt, 6) == SQLITE_TEXT)
				{
					opt_data_raw = QString{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) };
				}

				std::optional<QString> opt_userids;
				if (sqlite3_column_type(stmt, 10) == SQLITE_TEXT)
				{
					opt_userids = QString{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)) };
				}

				std::optional<QString> opt_attributes;
				if (sqlite3_column_type(stmt, 11) == SQLITE_TEXT)
				{
					opt_attributes = QString{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11)) };
				}

				if (opt_universe_id && opt_datastore_name && opt_scope && opt_key_name && opt_version && opt_data_raw)
				{
					result.push_back(DatastoreEntryWithDetails{
						*opt_universe_id, *opt_datastore_name, *opt_scope, *opt_key_name, *opt_version, opt_userids, opt_attributes, *opt_data_raw
					});
				}
			}
			else if (sqlite_result == SQLITE_DONE)
			{
				return result;
			}
			else
			{
				return std::nullopt;
			}
		}
	}
}
