#include "model_qt.h"

#include <algorithm>

#include <QString>
#include <QVariant>

#include "util_json.h"

BanListQTableModel::BanListQTableModel(QObject* parent, const std::vector<BanListUserRestriction>& restrictions) :  QAbstractTableModel{ parent }, restrictions{ restrictions }
{

}

QVariant BanListQTableModel::data(const QModelIndex& index, const int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (index.row() < static_cast<int>(restrictions.size()))
		{
			if (index.column() == 0)
			{
				return restrictions.at(index.row()).get_user();
			}
			else if (index.column() == 1)
			{
				return restrictions.at(index.row()).get_game_join_restriction().get_active();
			}
			else if (index.column() == 2)
			{
				return restrictions.at(index.row()).get_game_join_restriction().get_start_time();
			}
			else if (index.column() == 3)
			{
				const std::optional<QString> opt_duration = restrictions.at(index.row()).get_game_join_restriction().get_duration();
				if (opt_duration)
				{
					return *opt_duration;
				}
				else
				{
					return "Permanent";
				}
			}
		}
	}
	return QVariant{};
}

int BanListQTableModel::columnCount(const QModelIndex&) const
{
	return 4;
}

int BanListQTableModel::rowCount(const QModelIndex&) const
{
	return static_cast<int>(restrictions.size());
}

QVariant BanListQTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section == 0)
		{
			return "User";
		}
		else if (section == 1)
		{
			return "Active";
		}
		else if (section == 2)
		{
			return "Start";
		}
		else if (section == 3)
		{
			return "Duration";
		}
	}
	return QVariant{};
}

MemoryStoreSortedMapQTableModel::MemoryStoreSortedMapQTableModel(QObject* parent, const std::vector<MemoryStoreSortedMapItem>& items) : QAbstractTableModel{ parent }, items{ items }
{

}

QVariant MemoryStoreSortedMapQTableModel::data(const QModelIndex& index, const int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (index.row() < static_cast<int>(items.size()))
		{
			if (index.column() == 0)
			{
				return items.at(index.row()).get_id();
			}
			else if (index.column() == 1)
			{
				return items.at(index.row()).get_display_string_sort_key();
			}
			else if (index.column() == 2)
			{
				return items.at(index.row()).get_display_numeric_sort_key();
			}
			else if (index.column() == 3)
			{
				return items.at(index.row()).get_value().get_short_display_string();
			}
			else if (index.column() == 4)
			{
				return items.at(index.row()).get_expire_time();
			}
		}
	}
	return QVariant{};
}

int MemoryStoreSortedMapQTableModel::columnCount(const QModelIndex&) const
{
	return 5;
}

int MemoryStoreSortedMapQTableModel::rowCount(const QModelIndex&) const
{
	return static_cast<int>(items.size());
}

QVariant MemoryStoreSortedMapQTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section == 0)
		{
			return "ID";
		}
		else if (section == 1)
		{
			return "String Sort";
		}
		else if (section == 2)
		{
			return "Numeric Sort";
		}
		else if (section == 3)
		{
			return "Value";
		}
		else if (section == 4)
		{
			return "Expiration";
		}
	}
	return QVariant{};
}

OrderedDatastoreEntryQTableModel::OrderedDatastoreEntryQTableModel(QObject* parent, const std::vector<OrderedDatastoreEntryFull>& entries) : QAbstractTableModel{ parent }, entries{ entries }
{

}

std::optional<OrderedDatastoreEntryFull> OrderedDatastoreEntryQTableModel::get_entry(const size_t row_index) const
{
	if (row_index < entries.size())
	{
		return entries.at(row_index);
	}
	else
	{
		return std::nullopt;
	}
}

QVariant OrderedDatastoreEntryQTableModel::data(const QModelIndex& index, const int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (index.row() < static_cast<int>(entries.size()))
		{
			if (index.column() == 0)
			{
				return entries.at(index.row()).get_scope();
			}
			else if (index.column() == 1)
			{
				return entries.at(index.row()).get_key_name();
			}
			else if (index.column() == 2)
			{
				return entries.at(index.row()).get_value();
			}
		}
	}
	return QVariant{};
}

int OrderedDatastoreEntryQTableModel::columnCount(const QModelIndex&) const
{
	return 3;
}

int OrderedDatastoreEntryQTableModel::rowCount(const QModelIndex&) const
{
	return static_cast<int>(entries.size());
}

QVariant OrderedDatastoreEntryQTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section == 0)
		{
			return "Scope";
		}
		else if (section == 1)
		{
			return "Key";
		}
		else if (section == 2)
		{
			return "Value";
		}
	}
	return QVariant{};
}

StandardDatastoreEntryQTableModel::StandardDatastoreEntryQTableModel(QObject* parent, const std::vector<StandardDatastoreEntryName>& entries) : QAbstractTableModel{ parent } , entries { entries }
{
	std::sort(this->entries.begin(), this->entries.end(), [](const StandardDatastoreEntryName& a, const StandardDatastoreEntryName& b) {
		if (a.get_key() == b.get_key())
		{
			return a.get_scope() < b.get_scope();
		}
		else
		{
			return a.get_key() < b.get_key();
		}
	});
}

std::optional<StandardDatastoreEntryName> StandardDatastoreEntryQTableModel::get_entry(const size_t row_index) const
{
	if (row_index < entries.size())
	{
		return entries.at(row_index);
	}
	else
	{
		return std::nullopt;
	}
}

QVariant StandardDatastoreEntryQTableModel::data(const QModelIndex& index, const int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (index.row() < static_cast<int>(entries.size()))
		{
			if (index.column() == 0)
			{
				return entries.at(index.row()).get_scope();
			}
			else if (index.column() == 1)
			{
				return entries.at(index.row()).get_key();
			}
		}
	}
	return QVariant{};
}

int StandardDatastoreEntryQTableModel::columnCount(const QModelIndex&) const
{
	return 2;
}

int StandardDatastoreEntryQTableModel::rowCount(const QModelIndex&) const
{
	return static_cast<int>(entries.size());
}

QVariant StandardDatastoreEntryQTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section == 0)
		{
			return "Scope";
		}
		else if (section == 1)
		{
			return "Key";
		}
	}
	return QVariant{};
}

StandardDatastoreEntryVersionQTableModel::StandardDatastoreEntryVersionQTableModel(QObject* parent, const std::vector<StandardDatastoreEntryVersion>& versions) : QAbstractTableModel{ parent }, versions{ versions }
{
	std::sort(this->versions.begin(), this->versions.end(), [](const StandardDatastoreEntryVersion& a, const StandardDatastoreEntryVersion& b) {
		return b.get_version() < a.get_version();
	});
}

std::optional<StandardDatastoreEntryVersion> StandardDatastoreEntryVersionQTableModel::get_version(const size_t row_index) const
{
	if (row_index < versions.size())
	{
		return versions.at(row_index);
	}
	else
	{
		return std::nullopt;
	}
}

QVariant StandardDatastoreEntryVersionQTableModel::data(const QModelIndex& index, const int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (index.row() < static_cast<int>(versions.size()))
		{
			if (index.column() == 0)
			{
				return versions.at(index.row()).get_version();
			}
			else if (index.column() == 1)
			{
				return static_cast<unsigned long long>(versions.at(index.row()).get_content_length());
			}
			else if (index.column() == 2)
			{
				return versions.at(index.row()).get_deleted();
			}
			else if (index.column() == 3)
			{
				return versions.at(index.row()).get_created_time();
			}
		}
	}
	return QVariant{};
}

int StandardDatastoreEntryVersionQTableModel::columnCount(const QModelIndex&) const
{
	return 4;
}

int StandardDatastoreEntryVersionQTableModel::rowCount(const QModelIndex&) const
{
	return static_cast<int>(versions.size());
}

QVariant StandardDatastoreEntryVersionQTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section == 0)
		{
			return "Version";
		}
		else if (section == 1)
		{
			return "Size";
		}
		else if (section == 2)
		{
			return "Deleted";
		}
		else if (section == 3)
		{
			return "Created at";
		}
	}
	return QVariant{};
}
