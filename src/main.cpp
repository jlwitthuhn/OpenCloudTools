#include <QApplication>

#include "window_main.h"

int main(int argc, char** argv)
{
	QApplication::setApplicationName("RobloxCloudManager");
	QApplication::setOrganizationName("RobloxCloudManager");

	QApplication app{ argc, argv };

	MyMainWindow* const window = new MyMainWindow{};
	window->show();

	return app.exec();
}
