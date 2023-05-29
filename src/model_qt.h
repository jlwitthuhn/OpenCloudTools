#pragma once

#include <cstddef>

#include <optional>
#include <vector>

#include <Qt>
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>

#include "api_response.h"

class StandardDatastoreEntryQTableModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	StandardDatastoreEntryQTableModel(QObject* parent, const std::vector<StandardDatastoreEntry>& entries);

	std::optional<StandardDatastoreEntry> get_entry(size_t row_index) const;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex{}) const override;
	virtual int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	std::vector<StandardDatastoreEntry> entries;
};

class StandardDatastoreEntryVersionQTableModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	StandardDatastoreEntryVersionQTableModel(QObject* parent, const std::vector<StandardDatastoreEntryVersion>& versions);

	std::optional<StandardDatastoreEntryVersion> get_version(size_t row_index) const;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex{}) const override;
	virtual int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	std::vector<StandardDatastoreEntryVersion> versions;
};
