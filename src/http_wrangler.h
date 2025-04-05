#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include <Qt>
#include <QAbstractTableModel>
#include <QDateTime>
#include <QModelIndex>
#include <QObject>
#include <QString>

enum class HttpRequestType : std::uint8_t;

class QNetworkAccessManager;
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

	std::optional<HttpLogEntry> get_entry(std::size_t row_index) const;
	void append_entry(const HttpLogEntry& entry);

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex{}) const override;
	virtual int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	std::vector<HttpLogEntry> entries;

private:
	std::size_t convert_offset(std::size_t in) const;
};

class HttpWrangler : public QObject
{
	Q_OBJECT
public:
	static const std::unique_ptr<HttpWrangler>& get();

	QNetworkReply* send(HttpRequestType type, QNetworkRequest& request, const std::optional<QString>& body = std::nullopt);

	void clear_log();
	HttpLogModel* make_log_model(QObject* parent);

signals:
	void log_entry_added(HttpLogEntry log_entry);

private:
	class ConstructorToken {};

	void add_log_entry(const HttpLogEntry& log_entry);

	QNetworkAccessManager* network_access_manager;
	std::vector<HttpLogEntry> http_log_entries;

public:
	HttpWrangler(ConstructorToken token);
};
