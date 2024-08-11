#include "window_main_new.h"

#include <memory>

#include <Qt>
#include <QDockWidget>
#include <QLabel>
#include <QLineEdit>
#include <QMdiArea>
#include <QPushButton>
#include <QString>
#include <QToolBar>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "profile.h"
#include "window_api_key_manage.h"
#include "window_main_menu_bar.h"

MyNewMainWindow::MyNewMainWindow() : QMainWindow{ nullptr, Qt::Window }
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle("OpenCloudTools");
	setMinimumSize(650, 500);

	connect(&(UserProfile::get()), &UserProfile::selected_api_key_changed, this, &MyNewMainWindow::on_selected_api_key_changed);

	MyMainWindowMenuBar* const menu_bar = new MyMainWindowMenuBar{ this };
	setMenuBar(menu_bar);

	connect(menu_bar, &MyMainWindowMenuBar::request_close, this, &MyNewMainWindow::close);

	QToolBar* const main_tool_bar = new QToolBar{ this };
	main_tool_bar->setFloatable(false);
	main_tool_bar->setMovable(false);
	{
		QLabel* const api_key_label = new QLabel{ "Active API key: ", main_tool_bar };

		api_key_name_edit = new QLineEdit{ main_tool_bar };
		api_key_name_edit->setReadOnly(true);
		api_key_name_edit->setMaximumWidth(250);

		QPushButton* const change_key_button = new QPushButton{ "Change Key", main_tool_bar };
		connect(change_key_button, &QPushButton::clicked, this, &MyNewMainWindow::pressed_change_key);

		main_tool_bar->addWidget(api_key_label);
		main_tool_bar->addWidget(api_key_name_edit);
		main_tool_bar->addSeparator();
		main_tool_bar->addWidget(change_key_button);
	}
	addToolBar(main_tool_bar);

	QDockWidget* const universe_tree_container = new QDockWidget{ "Universes", this};
	universe_tree_container->setFeatures(QDockWidget::DockWidgetMovable);
	universe_tree_container->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	universe_tree_container->setMinimumWidth(150);
	{
		QWidget* const universe_tree_container_inner = new QWidget{ universe_tree_container };
		{
			QVBoxLayout* const universe_tree_layout = new QVBoxLayout{ universe_tree_container_inner };
			universe_tree_layout->setAlignment(Qt::AlignHCenter);

			QTreeWidget* const universe_tree = new QTreeWidget{ universe_tree_container_inner };
			universe_tree->setHeaderHidden(true);

			universe_tree_container_inner->setLayout(universe_tree_layout);
			universe_tree_layout->addWidget(universe_tree);
		}
		universe_tree_container->setWidget(universe_tree_container_inner);
	}
	addDockWidget(Qt::LeftDockWidgetArea, universe_tree_container);

	QMdiArea* const center_widget = new QMdiArea{ this };
	{
	}
	setCentralWidget(center_widget);

	// Pop up key selection automatically on startup
	pressed_change_key();

	resize(1100, 650);
}

void MyNewMainWindow::on_selected_api_key_changed()
{
	if (const std::shared_ptr<const ApiKeyProfile> key_profile = UserProfile::get().get_selected_api_key())
	{
		api_key_name_edit->setText(key_profile->get_name());
	}
	else
	{
		api_key_name_edit->clear();
	}
}

void MyNewMainWindow::pressed_change_key()
{
	ManageApiKeysWindow* const manage_keys_window = new ManageApiKeysWindow{ this };
	manage_keys_window->show();
}
