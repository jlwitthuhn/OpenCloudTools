#pragma once

#include <chrono>
#include <optional>

#include <QDateTime>
#include <QNetworkReply>
#include <QString>

template <typename T> class QList;

class RobloxTime
{
public:
	static void update_time_from_headers(const QList<QNetworkReply::RawHeaderPair>& headers);

	static bool is_initialized() { return last_parsed_time.has_value(); }

	// Gets the current time according to roblox servers
	static std::optional<QDateTime> get_roblox_time();
	// Parses a date in the format used by standard datastore entry versions
	static std::optional<QDateTime> parse_version_date(const QString& version_date);

private:
	static std::chrono::steady_clock::time_point last_updated;
	static std::optional<QDateTime> last_parsed_time;
};
