#include "http_wrangler.h"

#include <algorithm>
#include <memory>

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>

#include "assert.h"
#include "util_enum.h"

constexpr size_t LOG_MAX_ENTRIES = 1000;

HttpLogEntry::HttpLogEntry(HttpRequestType type, const QString& url) : _timestamp{ QDateTime::currentDateTime() }, _type{type}, _url{url}
{

}

HttpLogModel::HttpLogModel(QObject* parent, const std::vector<HttpLogEntry>& entries) : QAbstractTableModel{ parent }, entries{ entries }
{
	std::sort(this->entries.begin(), this->entries.end(), [](const HttpLogEntry& a, const HttpLogEntry& b) {
		return b.timestamp() < a.timestamp();
	});
}

std::optional<HttpLogEntry> HttpLogModel::get_entry(const size_t row_index) const
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

QVariant HttpLogModel::data(const QModelIndex& index, const int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (index.row() < static_cast<int>(entries.size()))
		{
			if (index.column() == 0)
			{
				return entries.at(index.row()).timestamp().toString(Qt::ISODateWithMs);
			}
			else if (index.column() == 1)
			{
				return get_enum_string(entries.at(index.row()).type());
			}
			else if (index.column() == 2 || index.column() == 3)
			{
				const QString& url = entries.at(index.row()).url();
				qsizetype paramIndex = url.indexOf('?');
				if (index.column() == 2)
				{
					if (paramIndex >= 0)
					{
						return url.left(paramIndex);
					}
					else
					{
						return url;
					}
				}
				else
				{
					if (paramIndex >= 0)
					{
						return url.right(url.size() - paramIndex);
					}
					else
					{
						return "";
					}
				}
			}
		}
	}
	return QVariant{};
}

int HttpLogModel::columnCount(const QModelIndex&) const
{
	return 4;
}

int HttpLogModel::rowCount(const QModelIndex&) const
{
	return static_cast<int>(entries.size());
}

QVariant HttpLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section == 0)
		{
			return "Time";
		}
		else if (section == 1)
		{
			return "Method";
		}
		else if (section == 2)
		{
			return "Base URL";
		}
		else if (section == 3)
		{
			return "URL Params";
		}
	}
	return QVariant{};
}

const std::unique_ptr<HttpWrangler>& HttpWrangler::get()
{
	static std::unique_ptr<HttpWrangler> wrangler = std::make_unique<HttpWrangler>(ConstructorToken{});
	return wrangler;
}

QNetworkReply* HttpWrangler::send(HttpRequestType type, QNetworkRequest& request, const std::optional<QString>& body)
{
	request.setAttribute(QNetworkRequest::Attribute::Http2AllowedAttribute, false);
	add_log_entry(HttpLogEntry{ type, request.url().toString() });
	switch (type)
	{
	case HttpRequestType::Get:
		OCTASSERT(body.has_value() == false);
		return network_access_manager->get(request);
	case HttpRequestType::Patch:
		if (body)
		{
			return network_access_manager->sendCustomRequest(request, "PATCH", body->toUtf8());
		}
		else
		{
			return network_access_manager->sendCustomRequest(request, "PATCH", "");
		}
	case HttpRequestType::Post:
		if (body)
		{
			return network_access_manager->post(request, body->toUtf8());
		}
		else
		{
			return network_access_manager->post(request, "");
		}
	case HttpRequestType::Delete:
		return network_access_manager->deleteResource(request);
	default:
		return nullptr;
	}
}

void HttpWrangler::clear_log()
{
	http_log_entries.clear();
}

HttpLogModel* HttpWrangler::make_log_model(QObject* parent)
{
	return new HttpLogModel{ parent, http_log_entries };
}

void HttpWrangler::add_log_entry(const HttpLogEntry& log_entry)
{
	http_log_entries.push_back(log_entry);
	if (http_log_entries.size() > LOG_MAX_ENTRIES)
	{
		const size_t erase_count = http_log_entries.size() - LOG_MAX_ENTRIES;
		http_log_entries.erase(http_log_entries.begin(), http_log_entries.begin() + erase_count);
	}
}

HttpWrangler::HttpWrangler(ConstructorToken)
{
	network_access_manager = std::make_unique<QNetworkAccessManager>();
}
