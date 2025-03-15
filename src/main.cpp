#include <QApplication>

#include "window_main_new.h"

int main(int argc, char** argv)
{
	QApplication::setApplicationName("RobloxCloudManager");
	QApplication::setOrganizationName("RobloxCloudManager");

	QApplication app{ argc, argv };

	MyNewMainWindow* const window = new MyNewMainWindow{};
	window->show();

	return app.exec();
}
