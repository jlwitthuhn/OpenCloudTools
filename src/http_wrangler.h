#pragma once

#include <optional>

#include <QAbstractTableModel>
#include <QDateTime>
#include <QObject>
#include <QString>

#include "util_enum.h"

class QNetworkReply;
class QNetworkRequest;

class HttpLogEntry
{
public:
	HttpLogEntry(HttpRequestType type, const QString& url);

	const QDateTime& timestamp() const { return _timestamp; }
	HttpRequestType type() const { return _type; }
	const QString& url() const { return _url; }

private:
	QDateTime _timestamp;
	HttpRequestType _type;
	QString _url;
};

class HttpLogModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	HttpLogModel(QObject* parent, const std::vector<HttpLogEntry>& entries);

	std::optional<HttpLogEntry> get_entry(size_t row_index) const;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex{}) const override;
	virtual int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	std::vector<HttpLogEntry> entries;
};

class HttpWrangler
{
public:
	static QNetworkReply* send(HttpRequestType type, QNetworkRequest& request, const std::optional<QString>& body = std::nullopt);

	static HttpLogModel* make_log_model(QObject* parent);
};
