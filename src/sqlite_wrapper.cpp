#include "sqlite_wrapper.h"

#include <array>
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

	// Table to track which enumerated entries no longer exist
	sqlite3_exec(db_handle, "DROP TABLE IF EXISTS datastore_deleted;", nullptr, nullptr, nullptr);
	sqlite3_exec(db_handle, "CREATE TABLE datastore_deleted (id INTEGER PRIMARY KEY, universe_id INTEGER NOT NULL, datastore_name TEXT NOT NULL, scope TEXT NOT NULL, key_name TEXT NOT NULL)", nullptr, nullptr, nullptr);

	// Table to track status of enumarating all keys
	sqlite3_exec(db_handle, "DROP TABLE IF EXISTS datastore_enumerate;", nullptr, nullptr, nullptr);
	sqlite3_exec(db_handle, "CREATE TABLE datastore_enumerate (universe_id INTEGER NOT NULL, datastore_name TEXT NOT NULL, next_cursor TEXT, PRIMARY KEY (universe_id, datastore_name))", nullptr, nullptr, nullptr);

	// Table to track status of enumaration metadata (scope and prefix)
	sqlite3_exec(db_handle, "DROP TABLE IF EXISTS datastore_enumerate_meta;", nullptr, nullptr, nullptr);
	sqlite3_exec(db_handle, "CREATE TABLE datastore_enumerate_meta (universe_id INTEGER NOT NULL, key TEXT NOT NULL, value TEXT NOT NULL, PRIMARY KEY (universe_id, key))", nullptr, nullptr, nullptr);

	// Table to enumerate what entries remain to be downloaded
	sqlite3_exec(db_handle, "DROP TABLE IF EXISTS datastore_pending;", nullptr, nullptr, nullptr);
	sqlite3_exec(db_handle, "CREATE TABLE datastore_pending (id INTEGER PRIMARY KEY, universe_id INTEGER NOT NULL, datastore_name TEXT NOT NULL, scope TEXT NOT NULL, key_name TEXT NOT NULL)", nullptr, nullptr, nullptr);

	return std::make_unique<SqliteDatastoreWrapper>(db_handle);
}

std::unique_ptr<SqliteDatastoreWrapper> SqliteDatastoreWrapper::open_from_path(const std::string& file_path)
{
	sqlite3* db_handle = nullptr;
	if (sqlite3_open(file_path.c_str(), &db_handle) != SQLITE_OK)
	{
		return nullptr;
	}

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

bool SqliteDatastoreWrapper::is_correct_schema()
{
	if (db_handle != nullptr)
	{
		{
			sqlite3_stmt* stmt = nullptr;
			const std::string sql = "PRAGMA table_info(datastore);";
			sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

			std::array<bool, 13> column_valid;
			column_valid.fill(false);
			while (true)
			{
				const int sqlite_result = sqlite3_step(stmt);
				if (sqlite_result == SQLITE_ROW)
				{
					if (sqlite3_column_type(stmt, 0) != SQLITE_INTEGER)
					{
						return false;
					}
					if (sqlite3_column_type(stmt, 1) != SQLITE_TEXT)
					{
						return false;
					}
					if (sqlite3_column_type(stmt, 2) != SQLITE_TEXT)
					{
						return false;
					}
					if (sqlite3_column_type(stmt, 3) != SQLITE_INTEGER)
					{
						return false;
					}
					if (sqlite3_column_type(stmt, 5) != SQLITE_INTEGER)
					{
						return false;
					}

					switch (sqlite3_column_int64(stmt, 0))
					{
					case 0:
						column_valid[0] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "id" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "INTEGER";
						break;
					case 1:
						column_valid[1] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "universe_id" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "INTEGER";
						break;
					case 2:
						column_valid[2] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "datastore_name" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					case 3:
						column_valid[3] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "scope" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					case 4:
						column_valid[4] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "key_name" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					case 5:
						column_valid[5] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "version" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					case 6:
						column_valid[6] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "data_type" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					case 7:
						column_valid[7] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "data_raw" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					case 8:
						column_valid[8] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "data_str" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					case 9:
						column_valid[9] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "data_num" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "REAL";
						break;
					case 10:
						column_valid[10] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "data_bool" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "INTEGER";
						break;
					case 11:
						column_valid[11] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "userids" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					case 12:
						column_valid[12] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "attributes" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					default:
						return false;
					}
				}
				else
				{
					break;
				}
			}

			for (const bool this_valid : column_valid)
			{
				if (this_valid == false)
				{
					return false;
				}
			}

			sqlite3_finalize(stmt);
		}

		{
			sqlite3_stmt* stmt = nullptr;
			const std::string sql = "PRAGMA table_info(datastore_deleted);";
			sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

			std::array<bool, 5> column_valid;
			column_valid.fill(false);
			while (true)
			{
				const int sqlite_result = sqlite3_step(stmt);
				if (sqlite_result == SQLITE_ROW)
				{
					switch (sqlite3_column_int64(stmt, 0))
					{
					case 0:
						column_valid[0] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "id" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "INTEGER";
						break;
					case 1:
						column_valid[1] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "universe_id" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "INTEGER";
						break;
					case 2:
						column_valid[2] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "datastore_name" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					case 3:
						column_valid[3] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "scope" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					case 4:
						column_valid[4] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "key_name" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					default:
						return false;
					}
				}
				else
				{
					break;
				}
			}

			for (const bool this_valid : column_valid)
			{
				if (this_valid == false)
				{
					return false;
				}
			}

			sqlite3_finalize(stmt);
		}

		{
			sqlite3_stmt* stmt = nullptr;
			const std::string sql = "PRAGMA table_info(datastore_pending);";
			sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

			std::array<bool, 5> column_valid;
			column_valid.fill(false);
			while (true)
			{
				const int sqlite_result = sqlite3_step(stmt);
				if (sqlite_result == SQLITE_ROW)
				{
					switch (sqlite3_column_int64(stmt, 0))
					{
					case 0:
						column_valid[0] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "id" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "INTEGER";
						break;
					case 1:
						column_valid[1] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "universe_id" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "INTEGER";
						break;
					case 2:
						column_valid[2] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "datastore_name" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					case 3:
						column_valid[3] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "scope" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					case 4:
						column_valid[4] =
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) } == "key_name" &&
							std::string{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) } == "TEXT";
						break;
					default:
						return false;
					}
				}
				else
				{
					break;
				}
			}

			for (const bool this_valid : column_valid)
			{
				if (this_valid == false)
				{
					return false;
				}
			}

			sqlite3_finalize(stmt);
		}

		return true;
	}

	return false;
}

bool SqliteDatastoreWrapper::is_resumable(const long long universe_id)
{
	if (db_handle != nullptr)
	{
		bool has_valid_cursors = false;
		{
			sqlite3_stmt* stmt = nullptr;
			const std::string sql = "SELECT COUNT(*) FROM datastore_enumerate WHERE universe_id = ?010 AND next_cursor IS NOT NULL;";
			sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
			if (stmt != nullptr)
			{
				sqlite3_bind_int64(stmt, 10, universe_id);

				const int sqlite_result = sqlite3_step(stmt);
				if (sqlite_result == SQLITE_ROW)
				{
					has_valid_cursors = sqlite3_column_int64(stmt, 0) <= 1;
				}
				sqlite3_finalize(stmt);
			}
		}

		bool has_entries_pending = false;
		{
			sqlite3_stmt* stmt = nullptr;
			const std::string sql = "SELECT COUNT(*) FROM datastore_pending WHERE universe_id = ?010;";
			sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
			if (stmt != nullptr)
			{
				sqlite3_bind_int64(stmt, 10, universe_id);

				const int sqlite_result = sqlite3_step(stmt);
				if (sqlite_result == SQLITE_ROW)
				{
					has_entries_pending = sqlite3_column_int64(stmt, 0) > 0;
				}
				sqlite3_finalize(stmt);
			}
		}

		bool has_enumeration_pending = false;
		{
			sqlite3_stmt* stmt = nullptr;
			const std::string sql = "SELECT COUNT(*) FROM datastore_enumerate WHERE universe_id = ?010;";
			sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
			if (stmt != nullptr)
			{
				sqlite3_bind_int64(stmt, 10, universe_id);

				const int sqlite_result = sqlite3_step(stmt);
				if (sqlite_result == SQLITE_ROW)
				{
					has_enumeration_pending = sqlite3_column_int64(stmt, 0) > 0;
				}
				sqlite3_finalize(stmt);
			}
		}

		return has_valid_cursors && (has_entries_pending || has_enumeration_pending);
	}

	return false;
}

void SqliteDatastoreWrapper::write_deleted(const StandardDatastoreEntry& entry)
{
	if (db_handle != nullptr)
	{
		sqlite3_stmt* stmt = nullptr;
		const std::string sql = "INSERT INTO datastore_deleted (universe_id, datastore_name, scope, key_name) VALUES (?010, ?020, ?030, ?040);";
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

void SqliteDatastoreWrapper::write_enumeration(long long universe_id, const std::string& datastore_name, const std::optional<std::string> cursor)
{
	if (db_handle != nullptr)
	{
		if (cursor)
		{
			sqlite3_stmt* stmt = nullptr;
			const std::string sql = "UPDATE datastore_enumerate SET next_cursor = ?030 WHERE universe_id = ?010 AND datastore_name = ?020;";
			sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
			if (stmt != nullptr)
			{
				sqlite3_bind_int64(stmt, 10, universe_id);
				sqlite3_bind_text(stmt, 20, datastore_name.c_str(), -1, SQLITE_TRANSIENT);

				sqlite3_bind_text(stmt, 30, cursor->c_str(), -1, SQLITE_TRANSIENT);

				sqlite3_step(stmt);

				sqlite3_finalize(stmt);
			}
		}
		else
		{
			sqlite3_stmt* stmt = nullptr;
			const std::string sql = "INSERT INTO datastore_enumerate (universe_id, datastore_name) VALUES (?010, ?020);";
			sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
			if (stmt != nullptr)
			{
				sqlite3_bind_int64(stmt, 10, universe_id);
				sqlite3_bind_text(stmt, 20, datastore_name.c_str(), -1, SQLITE_TRANSIENT);

				sqlite3_step(stmt);

				sqlite3_finalize(stmt);
			}
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

void SqliteDatastoreWrapper::delete_enumeration(const long long universe_id, const std::string& datastore_name)
{
	if (db_handle != nullptr)
	{
		sqlite3_stmt* stmt = nullptr;
		const std::string sql = "DELETE FROM datastore_enumerate WHERE universe_id = ?010 AND datastore_name = ?020;";
		sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
		if (stmt != nullptr)
		{
			sqlite3_bind_int64(stmt, 10, universe_id);
			sqlite3_bind_text(stmt, 20, datastore_name.c_str(), -1, SQLITE_TRANSIENT);

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

std::optional<std::string> SqliteDatastoreWrapper::get_enumarating_cursor(const long long universe_id)
{
	std::optional<std::string> result;

	if (db_handle != nullptr)
	{
		sqlite3_stmt* stmt = nullptr;
		const std::string sql = "SELECT next_cursor FROM datastore_enumerate WHERE universe_id = ?010 AND next_cursor IS NOT NULL;";
		sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
		if (stmt != nullptr)
		{
			sqlite3_bind_int64(stmt, 10, universe_id);

			const int sqlite_result = sqlite3_step(stmt);
			if (sqlite_result == SQLITE_ROW)
			{
				result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
			}

			sqlite3_finalize(stmt);
		}
	}

	return result;
}

std::optional<std::string> SqliteDatastoreWrapper::get_enumarating_datastore(const long long universe_id)
{
	std::optional<std::string> result;

	if (db_handle != nullptr)
	{
		sqlite3_stmt* stmt = nullptr;
		const std::string sql = "SELECT datastore_name FROM datastore_enumerate WHERE universe_id = ?010 AND next_cursor IS NOT NULL;";
		sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
		if (stmt != nullptr)
		{
			sqlite3_bind_int64(stmt, 10, universe_id);

			const int sqlite_result = sqlite3_step(stmt);
			if (sqlite_result == SQLITE_ROW)
			{
				result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
			}

			sqlite3_finalize(stmt);
		}
	}

	return result;
}

std::vector<std::string> SqliteDatastoreWrapper::get_pending_datastores(const long long universe_id)
{
	std::vector<std::string> result;

	if (db_handle != nullptr)
	{
		{
			sqlite3_stmt* stmt = nullptr;
			const std::string sql = "SELECT count(*) FROM datastore_enumerate WHERE universe_id = ?010 AND next_cursor IS NULL;";
			sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
			if (stmt != nullptr)
			{
				sqlite3_bind_int64(stmt, 10, universe_id);

				const int sqlite_result = sqlite3_step(stmt);
				if (sqlite_result == SQLITE_ROW)
				{
					result.reserve(static_cast<size_t>(sqlite3_column_int64(stmt, 0)));
				}

				sqlite3_finalize(stmt);
			}
		}

		{
			sqlite3_stmt* stmt = nullptr;
			const std::string sql = "SELECT datastore_name FROM datastore_enumerate WHERE universe_id = ?010 AND next_cursor IS NULL;";
			sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
			sqlite3_bind_int64(stmt, 10, universe_id);
			while (stmt != nullptr)
			{
				const int sqlite_result = sqlite3_step(stmt);
				if (sqlite_result == SQLITE_ROW)
				{
					const std::string this_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
					result.push_back(this_name);
				}
				else
				{
					sqlite3_finalize(stmt);
					stmt = nullptr;
				}
			}
		}
	}

	return result;
}

std::vector<StandardDatastoreEntry> SqliteDatastoreWrapper::get_pending_entries(const long long universe_id)
{
	std::vector<StandardDatastoreEntry> result;

	if (db_handle != nullptr)
	{
		{
			sqlite3_stmt* stmt = nullptr;
			const std::string sql = "SELECT count(*) FROM datastore_pending WHERE universe_id = ?010;";
			sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
			if (stmt != nullptr)
			{
				sqlite3_bind_int64(stmt, 10, universe_id);

				const int sqlite_result = sqlite3_step(stmt);
				if (sqlite_result == SQLITE_ROW)
				{
					result.reserve(static_cast<size_t>(sqlite3_column_int64(stmt, 0)));
				}

				sqlite3_finalize(stmt);
			}
		}

		{
			sqlite3_stmt* stmt = nullptr;
			const std::string sql = "SELECT universe_id, datastore_name, scope, key_name FROM datastore_pending WHERE universe_id = ?010;";
			sqlite3_prepare_v2(db_handle, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
			sqlite3_bind_int64(stmt, 10, universe_id);
			while (stmt != nullptr)
			{
				const int sqlite_result = sqlite3_step(stmt);
				if (sqlite_result == SQLITE_ROW)
				{
					const long long this_universe_id = sqlite3_column_int64(stmt, 0);
					const QString this_datastore_name = QString{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) };
					const QString this_scope = QString{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) };
					const QString this_key_name = QString{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) };

					const StandardDatastoreEntry this_entry{ this_universe_id, this_datastore_name, this_key_name, this_scope };
					result.push_back(this_entry);
				}
				else
				{
					sqlite3_finalize(stmt);
					stmt = nullptr;
				}
			}
		}
	}

	return result;
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
				sqlite3_finalize(stmt);
				return result;
			}
			else
			{
				sqlite3_finalize(stmt);
				return std::nullopt;
			}
		}
	}
}
