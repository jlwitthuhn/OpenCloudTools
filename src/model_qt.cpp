#include "model_qt.h"

#include <algorithm>
#include <memory>

#include <QString>

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
				return entries.at(index.row()).get_key();
			}
			else if (index.column() == 1)
			{
				return entries.at(index.row()).get_scope();
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
			return "Key";
		}
		else if (section == 1)
		{
			return "Scope";
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
