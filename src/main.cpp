#include <QApplication>

#ifdef OCT_NEW_GUI
#include "window_main_new.h"
#else
#include "window_api_key_manage.h"
#endif

int main(int argc, char** argv)
{
	QApplication::setApplicationName("RobloxCloudManager");
	QApplication::setOrganizationName("RobloxCloudManager");

	QApplication app{ argc, argv };

#ifdef OCT_NEW_GUI
	MyNewMainWindow* const window = new MyNewMainWindow{};
#else
	// Show API key selection first, this will launch other windows as needed
	ManageApiKeysWindow* window = new ManageApiKeysWindow{};
#endif
	window->show();

	return app.exec();
}
