#include "roblox_time.h"

#include <string>
#include <utility>

#include <QByteArray>
#include <QDateTime>

std::chrono::steady_clock::time_point RobloxTime::last_updated;
std::optional<QDateTime> RobloxTime::last_parsed_time;

static QString trim_header(const QByteArray& date_header)
{
	const QString date_string{ date_header };
	return date_string.mid(0, date_string.size() - 4);
}

// Format: "Fri, 01 Jul 2022 21:34:54 GMT"
static QDateTime parse_header(const QByteArray& date_header)
{
	const QString date_string = trim_header(date_header);
	return QDateTime::fromString(date_string, "ddd, dd MMM yyyy HH:mm:ss");
}

void RobloxTime::update_time_from_headers(const QList<QNetworkReply::RawHeaderPair>& headers)
{
	for (auto this_pair : headers)
	{
		if (this_pair.first.toStdString() == "date")
		{
			last_parsed_time = parse_header(this_pair.second);
			last_updated = std::chrono::steady_clock::now();
		}
	}
}

std::optional<QDateTime> RobloxTime::get_roblox_time()
{
	if (last_parsed_time)
	{
		std::chrono::duration<long long> time_passed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - last_updated);
		return last_parsed_time->addSecs(time_passed.count());
	}
	else
	{
		return std::nullopt;
	}
}

// Format: "2022-06-05T09:44:26.9224165Z"
std::optional<QDateTime> RobloxTime::parse_version_date(const QString& version_date)
{
	QString mutable_date{ version_date };
	mutable_date.replace("T", " ");
	mutable_date = mutable_date.mid(0, 19);
	// Now the format is "2022-06-05 09:44:26"
	const QDateTime result{ QDateTime::fromString(mutable_date, "yyyy-MM-dd HH:mm:ss") };
	if (result.isValid())
	{
		return result;
	}
	else
	{
		return std::nullopt;
	}
}
