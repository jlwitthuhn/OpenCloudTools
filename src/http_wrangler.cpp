#include "http_wrangler.h"

#include <memory>
#include <mutex>

#include <QNetworkAccessManager>

static std::once_flag network_access_once;
static std::unique_ptr<QNetworkAccessManager> network_access_manager;

static void init_network_manager()
{
	std::call_once(network_access_once, [&]() {
		network_access_manager = std::make_unique<QNetworkAccessManager>();
	});
}

QNetworkReply* HttpWrangler::send(HttpRequestType type, QNetworkRequest& request, const std::optional<QString>& body)
{
	init_network_manager();
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
