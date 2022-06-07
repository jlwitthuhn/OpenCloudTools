#include <QApplication>

#include "window_api_key_manage.h"

int main(int argc, char** argv)
{
	QApplication::setApplicationName("RobloxCloudManager");
	QApplication::setOrganizationName("RobloxCloudManager");

	QApplication app{ argc, argv };

	{
		// Show API key selection first, this will launch other windows as needed
		ManageApiKeysWindow* window = new ManageApiKeysWindow{};
		window->show();
	}

	return app.exec();
}
