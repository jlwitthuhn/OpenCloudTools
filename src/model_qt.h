#pragma once

#include <cstddef>

#include <optional>
#include <vector>

#include <Qt>
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>

#include "model_common.h"

class BanListQTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	BanListQTableModel(QObject* parent, const std::vector<BanListUserRestriction>& restrictions);

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual int columnCount(const QModelIndex& index = QModelIndex{}) const override;
	virtual int rowCount(const QModelIndex& index = QModelIndex{}) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	std::vector<BanListUserRestriction> restrictions;
};

class MemoryStoreSortedMapQTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	MemoryStoreSortedMapQTableModel(QObject* parent, const std::vector<MemoryStoreSortedMapItem>& items);

	std::optional<MemoryStoreSortedMapItem> get_item(size_t row_index) const;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual int columnCount(const QModelIndex& index = QModelIndex{}) const override;
	virtual int rowCount(const QModelIndex& index = QModelIndex{}) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	std::vector<MemoryStoreSortedMapItem> items;
};

class OrderedDatastoreEntryQTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	OrderedDatastoreEntryQTableModel(QObject* parent, const std::vector<OrderedDatastoreEntryFull>& entries);

	std::optional<OrderedDatastoreEntryFull> get_entry(size_t row_index) const;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex{}) const override;
	virtual int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	std::vector<OrderedDatastoreEntryFull> entries;
};

class StandardDatastoreEntryQTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	StandardDatastoreEntryQTableModel(QObject* parent, const std::vector<StandardDatastoreEntryName>& entries);

	std::optional<StandardDatastoreEntryName> get_entry(size_t row_index) const;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex{}) const override;
	virtual int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	std::vector<StandardDatastoreEntryName> entries;
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
