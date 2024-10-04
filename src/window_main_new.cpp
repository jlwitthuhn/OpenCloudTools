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
#include <QTreeWidgetItem>
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

	connect(&(UserProfile::get()), &UserProfile::active_api_key_changed, this, &MyNewMainWindow::on_active_api_key_changed);

	MyMainWindowMenuBar* const menu_bar = new MyMainWindowMenuBar{ this };
	setMenuBar(menu_bar);

	connect(menu_bar, &MyMainWindowMenuBar::request_close, this, &MyNewMainWindow::close);

	QToolBar* const main_tool_bar = new QToolBar{ this };
	main_tool_bar->setFloatable(false);
	main_tool_bar->setMovable(false);
	{
		QLabel* const api_key_label = new QLabel{ "Active API key: ", main_tool_bar };

		edit_api_key_name = new QLineEdit{ main_tool_bar };
		edit_api_key_name->setReadOnly(true);
		edit_api_key_name->setMaximumWidth(250);

		QPushButton* const change_key_button = new QPushButton{ "Change Key", main_tool_bar };
		connect(change_key_button, &QPushButton::clicked, this, &MyNewMainWindow::pressed_change_key);

		main_tool_bar->addWidget(api_key_label);
		main_tool_bar->addWidget(edit_api_key_name);
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

			tree_universe = new QTreeWidget{ universe_tree_container_inner };
			tree_universe->setColumnCount(1);
			tree_universe->setHeaderHidden(true);

			universe_tree_container_inner->setLayout(universe_tree_layout);
			universe_tree_layout->addWidget(tree_universe);
		}
		universe_tree_container->setWidget(universe_tree_container_inner);
	}
	addDockWidget(Qt::LeftDockWidgetArea, universe_tree_container);

	QMdiArea* const center_widget = new QMdiArea{ this };
	{
	}
	setCentralWidget(center_widget);

	resize(1100, 650);

	// Pop up key selection automatically on startup
	pressed_change_key();
}

void MyNewMainWindow::on_active_api_key_changed()
{
	if (const std::shared_ptr<const ApiKeyProfile> key_profile = UserProfile::get().get_active_api_key())
	{
		edit_api_key_name->setText(key_profile->get_name());
	}
	else
	{
		edit_api_key_name->clear();
	}

	rebuild_universe_tree();
}

void MyNewMainWindow::pressed_change_key()
{
	ManageApiKeysWindow* const manage_keys_window = new ManageApiKeysWindow{ this };
	manage_keys_window->show();
}

void MyNewMainWindow::rebuild_universe_tree()
{
	tree_universe->clear();

	const std::shared_ptr<const ApiKeyProfile> key_profile = UserProfile::get().get_active_api_key();
	if (!key_profile)
	{
		return;
	}

	const std::vector<std::shared_ptr<UniverseProfile>> universe_list = key_profile->get_universe_list();
	for (const std::shared_ptr<UniverseProfile>& this_universe : universe_list)
	{
		QTreeWidgetItem* const this_item = new QTreeWidgetItem{ tree_universe };
		this_item->setText(0, this_universe->get_name());
		{
			QTreeWidgetItem* const open_datastore = new QTreeWidgetItem{ this_item };
			open_datastore->setText(0, "Data Stores");
		}
		this_item->setExpanded(true);
	}
}
