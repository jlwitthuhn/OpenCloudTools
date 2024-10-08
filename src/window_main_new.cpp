#include "window_main_new.h"

#include <vector>

#include <Qt>
#include <QByteArray>
#include <QDockWidget>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "assert.h"
#include "panel_datastore_standard.h"
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

	connect(&(UserProfile::get()), &UserProfile::active_api_key_changed, this, &MyNewMainWindow::handle_active_api_key_changed);

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
			connect(tree_universe, &QTreeWidget::itemDoubleClicked, this, &MyNewMainWindow::show_subwindow_from_item);
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

	center_mdi_widget = new QMdiArea{ this };
	setCentralWidget(center_mdi_widget);

	resize(1200, 650);

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
		edit_api_key_name->setText(api_profile->get_name());
	}
	else
	{
		setEnabled(false);
		edit_api_key_name->clear();
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

void MyNewMainWindow::handle_active_api_key_changed()
{
	const std::shared_ptr<ApiKeyProfile> key_profile = UserProfile::get().get_active_api_key();
	attached_profile = key_profile;
	disconnect(conn_attached_profile_details_changed);
	conn_attached_profile_details_changed = connect(key_profile.get(), &ApiKeyProfile::details_changed, this, &MyNewMainWindow::handle_active_api_key_details_changed);
	disconnect(conn_attached_profile_universe_details_changed);
	conn_attached_profile_universe_details_changed = connect(key_profile.get(), &ApiKeyProfile::universe_details_changed, this, &MyNewMainWindow::handle_universe_details_changed);
	disconnect(conn_attached_profile_universe_list_changed);
	conn_attached_profile_universe_list_changed = connect(key_profile.get(), &ApiKeyProfile::universe_list_changed, this, &MyNewMainWindow::handle_universe_list_changed);

	rebuild_universe_tree();
}

void MyNewMainWindow::handle_active_api_key_details_changed()
{
	gui_refresh();
}

void MyNewMainWindow::handle_subwindow_closed(const SubwindowId& id)
{
	OCTASSERT(subwindows.count(id) == 1);
	subwindows.erase(id);
}

void MyNewMainWindow::handle_universe_details_changed(const UniverseProfile::Id)
{
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
		this_item->setText(0, this_universe->get_display_name());
		this_item->setData(0, Qt::UserRole, this_universe_id);
		{
			QTreeWidgetItem* const open_datastore = new QTreeWidgetItem{ this_item };
			open_datastore->setText(0, "Data Stores");
			open_datastore->setData(0, Qt::UserRole, static_cast<int>(SubwindowType::DATA_STORES_STANDARD));
		}
		this_item->setExpanded(true);
	}

	gui_refresh();
}

void MyNewMainWindow::show_subwindow(const SubwindowId& id)
{
	const std::shared_ptr<ApiKeyProfile> api_profile = attached_profile.lock();
	if (!api_profile)
	{
		OCTASSERT(false);
		return;
	}

	const std::map<SubwindowId, QMdiSubWindow*>::const_iterator existing_iter = subwindows.find(id);
	if (existing_iter != subwindows.end())
	{
		existing_iter->second->show();
		existing_iter->second->raise();
		return;
	}

	const std::shared_ptr<UniverseProfile> universe = api_profile->get_universe_profile_by_id(id.get_universe_id());
	if (!universe)
	{
		OCTASSERT(false);
		return;
	}

	QMdiSubWindow* new_subwindow = nullptr;
	switch (id.get_type())
	{
		case SubwindowType::DATA_STORES_STANDARD:
			StandardDatastorePanel* const new_datastore_panel = new StandardDatastorePanel{ nullptr, api_profile->get_key() };
			new_datastore_panel->change_universe(universe);
			new_subwindow = center_mdi_widget->addSubWindow(new_datastore_panel);
	}

	if (!new_subwindow)
	{
		OCTASSERT(false);
		return;
	}
	connect(new_subwindow, &QMdiSubWindow::destroyed, this, [this, id]() {
		this->handle_subwindow_closed(id);
	});
	subwindows.emplace(id, new_subwindow);
	new_subwindow->show();
}

void MyNewMainWindow::show_subwindow_from_item(QTreeWidgetItem* const item)
{
	QTreeWidgetItem* const item_parent = item->parent();
	if (item_parent == nullptr)
	{
		OCTASSERT(false);
		return;
	}

	const QVariant universe_id_var = item_parent->data(0, Qt::UserRole);
	if (qvariant_is_byte_array(universe_id_var, static_cast<int>(RandomId128::LENGTH)) == false)
	{
		OCTASSERT(false);
		return;
	}
	const UniverseProfile::Id universe_id{ universe_id_var.toByteArray() };

	const QVariant type_var = item->data(0, Qt::UserRole);
	const SubwindowType type = static_cast<SubwindowType>(type_var.toInt());

	const SubwindowId subwindow_id{ type, universe_id };
	show_subwindow(subwindow_id);
}
