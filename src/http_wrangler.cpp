#include "http_wrangler.h"

#include <memory>
#include <mutex>

#include <QNetworkAccessManager>

static std::once_flag network_access_once;
static std::unique_ptr<QNetworkAccessManager> network_access_manager;
static std::vector<HttpLogEntry> http_log_entries;

static void init_network_manager()
{
	std::call_once(network_access_once, [&]() {
		network_access_manager = std::make_unique<QNetworkAccessManager>();
	});
}

HttpLogEntry::HttpLogEntry(HttpRequestType type, const QString& url) : _timestamp{ QDateTime::currentDateTime() }, _type{type}, _url{url}
{

}

HttpLogModel::HttpLogModel(QObject* parent, const std::vector<HttpLogEntry>& entries) : entries{ entries }
{
	std::sort(this->entries.begin(), this->entries.end(), [](const HttpLogEntry& a, const HttpLogEntry& b) {
		return b.timestamp() < a.timestamp();
	});
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

QNetworkReply* HttpWrangler::send(HttpRequestType type, QNetworkRequest& request, const std::optional<QString>& body)
{
	init_network_manager();
	request.setAttribute(QNetworkRequest::Attribute::Http2AllowedAttribute, false);
	http_log_entries.push_back(HttpLogEntry{ type, request.url().toString() });
	switch (type)
	{
	case HttpRequestType::Get:
		return network_access_manager->get(request);
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
	init_network_manager();
	return new HttpLogModel{ parent, http_log_entries };
}
