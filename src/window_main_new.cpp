#include "window_main_new.h"

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>

#include "window_api_key_manage.h"
#include "window_main_menu_bar.h"

MyNewMainWindow::MyNewMainWindow() : QMainWindow{ nullptr, Qt::Window }
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle("OpenCloudTools");
	setMinimumSize(640, 480);

	MyMainWindowMenuBar* const menu_bar = new MyMainWindowMenuBar{ this };
	setMenuBar(menu_bar);

	connect(menu_bar, &MyMainWindowMenuBar::request_close, this, &MyNewMainWindow::close);

	QToolBar* const main_tool_bar = new QToolBar{ this };
	main_tool_bar->setFloatable(false);
	main_tool_bar->setMovable(false);
	{
		QLabel* const api_key_label = new QLabel{ "Active API key: ", main_tool_bar };

		QLineEdit* const api_key_name_edit = new QLineEdit{ main_tool_bar };
		api_key_name_edit->setReadOnly(true);
		api_key_name_edit->setMaximumWidth(250);

		QPushButton* const change_key_button = new QPushButton{ "Change Key", main_tool_bar };

		main_tool_bar->addWidget(api_key_label);
		main_tool_bar->addWidget(api_key_name_edit);
		main_tool_bar->addSeparator();
		main_tool_bar->addWidget(change_key_button);
	}
	addToolBar(main_tool_bar);

	QWidget* const center_widget = new QWidget{ this };
	{


	}
	setCentralWidget(center_widget);

	// Pop up key selection automatically on startup
	ManageApiKeysWindow* const manage_keys_window = new ManageApiKeysWindow{ this };
	manage_keys_window->show();
}
