#pragma once

#include <optional>

#include <QString>

#include "util_enum.h"

class QNetworkReply;
class QNetworkRequest;

class HttpWrangler
{
public:
	static QNetworkReply* send(HttpRequestType type, QNetworkRequest& request, const std::optional<QString>& body = std::nullopt);
};
