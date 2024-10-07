#include "window_main_new.h"

#include <Qt>
#include <QDockWidget>
#include <QLabel>
#include <QLineEdit>
#include <QMdiArea>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

#include "assert.h"
#include "profile.h"
#include "util_qvariant.h"
#include "window_add_universe.h"
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
			QVBoxLayout* const layout_universe_tree = new QVBoxLayout{ universe_tree_container_inner };
			layout_universe_tree->setAlignment(Qt::AlignHCenter);

			tree_universe = new QTreeWidget{ universe_tree_container_inner };
			tree_universe->setColumnCount(1);
			tree_universe->setHeaderHidden(true);
			connect(tree_universe, &QTreeWidget::itemSelectionChanged, this, &MyNewMainWindow::gui_refresh);

			QPushButton* const button_add_universe = new QPushButton{ "Add universe...", universe_tree_container_inner };
			connect(button_add_universe, &QPushButton::clicked, this, &MyNewMainWindow::pressed_add_universe);

			button_edit_universe = new QPushButton{ "Edit universe...", universe_tree_container_inner };
			connect(button_edit_universe, &QPushButton::clicked, this, &MyNewMainWindow::pressed_edit_universe);

			button_delete_universe = new QPushButton{ "Delete universe", universe_tree_container_inner };
			connect(button_delete_universe, &QPushButton::clicked, this, &MyNewMainWindow::pressed_delete_universe);

			universe_tree_container_inner->setLayout(layout_universe_tree);
			layout_universe_tree->addWidget(tree_universe);
			layout_universe_tree->addWidget(button_add_universe);
			layout_universe_tree->addWidget(button_edit_universe);
			layout_universe_tree->addWidget(button_delete_universe);
		}
		universe_tree_container->setWidget(universe_tree_container_inner);
	}
	addDockWidget(Qt::LeftDockWidgetArea, universe_tree_container);

	QMdiArea* const center_widget = new QMdiArea{ this };
	setCentralWidget(center_widget);

	resize(1100, 650);

	gui_refresh();

	// Pop up key selection automatically on startup
	pressed_change_key();
}

void MyNewMainWindow::gui_refresh()
{
	const std::shared_ptr<ApiKeyProfile> api_profile = attached_profile.lock();
	if (api_profile)
	{
		setEnabled(true);
	}
	else
	{
		setEnabled(false);
	}
	const std::optional<UniverseProfile::Id> selected_universe = get_selected_universe_id();
	const bool universe_buttons_active = static_cast<bool>(selected_universe);
	button_edit_universe->setEnabled(universe_buttons_active);
	button_delete_universe->setEnabled(universe_buttons_active);
}

std::optional<UniverseProfile::Id> MyNewMainWindow::get_selected_universe_id()
{
	const QList<QTreeWidgetItem*> items = tree_universe->selectedItems();
	if (items.size() == 0)
	{
		return {};
	}

	QTreeWidgetItem* const selected_item = items.front();
	const bool top_level_item = selected_item->parent() == nullptr;
	if (top_level_item == false)
	{
		return {};
	}

	const QVariant user_data = selected_item->data(0, Qt::UserRole);
	if (qvariant_is_byte_array(user_data) == false)
	{
		OCTASSERT(false);
		return {};
	}

	return ApiKeyProfile::Id{ user_data.toByteArray() };
}

void MyNewMainWindow::on_active_api_key_changed()
{
	const std::shared_ptr<ApiKeyProfile> key_profile = UserProfile::get().get_active_api_key();
	attached_profile = key_profile;
	disconnect(attached_profile_universe_list_changed_conn);
	attached_profile_universe_list_changed_conn = connect(key_profile.get(), &ApiKeyProfile::universe_list_changed, this, &MyNewMainWindow::handle_universe_list_changed);

	if (key_profile)
	{
		edit_api_key_name->setText(key_profile->get_name());
	}
	else
	{
		edit_api_key_name->clear();
	}

	rebuild_universe_tree();
}

void MyNewMainWindow::handle_universe_list_changed()
{
	rebuild_universe_tree();
}

void MyNewMainWindow::pressed_change_key()
{
	ManageApiKeysWindow* const manage_keys_window = new ManageApiKeysWindow{ this };
	manage_keys_window->show();
}

void MyNewMainWindow::pressed_add_universe()
{
	if (const std::shared_ptr<ApiKeyProfile> shared_profile = attached_profile.lock())
	{
		AddUniverseWindow* const modal_window = new AddUniverseWindow{ this, shared_profile->get_key(), nullptr};
		modal_window->show();
	}
}

void MyNewMainWindow::pressed_edit_universe()
{
	const std::shared_ptr<ApiKeyProfile> api_profile = attached_profile.lock();
	OCTASSERT(api_profile);
	const std::optional<UniverseProfile::Id> universe_id = get_selected_universe_id();
	OCTASSERT(universe_id);
	const std::shared_ptr<UniverseProfile> universe_profile = api_profile->get_universe_profile_by_id(*universe_id);
	OCTASSERT(universe_profile);
	AddUniverseWindow* const modal_window = new AddUniverseWindow{ this, api_profile->get_key(), universe_profile};
	modal_window->show();
}

void MyNewMainWindow::pressed_delete_universe()
{
	const std::shared_ptr<ApiKeyProfile> api_profile = attached_profile.lock();
	OCTASSERT(api_profile);
	const std::optional<UniverseProfile::Id> universe_id = get_selected_universe_id();
	OCTASSERT(universe_id);

	QMessageBox* msg_box = new QMessageBox{ this };
	msg_box->setWindowTitle("Confirm deletion");
	msg_box->setText("Are you sure you want to delete this universe? This cannot be undone.");
	msg_box->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	int result = msg_box->exec();
	if (result == QMessageBox::Yes)
	{
		api_profile->delete_universe(*universe_id);
	}
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
		const QByteArray this_universe_id = this_universe->get_id().as_q_byte_array();
		QTreeWidgetItem* const this_item = new QTreeWidgetItem{ tree_universe };
		this_item->setText(0, this_universe->get_name());
		this_item->setData(0, Qt::UserRole, this_universe_id);
		{
			QTreeWidgetItem* const open_datastore = new QTreeWidgetItem{ this_item };
			open_datastore->setText(0, "Data Stores");
		}
		this_item->setExpanded(true);
	}

	gui_refresh();
}
