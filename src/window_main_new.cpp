#include "window_main_new.h"

#include <utility>
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
#include <QSize>
#include <QString>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "assert.h"
#include "panel_bulk_data.h"
#include "panel_datastore_ordered.h"
#include "panel_datastore_standard.h"
#include "panel_http_log.h"
#include "panel_mem_sorted_map.h"
#include "panel_messaging_service.h"
#include "panel_universe_prefs.h"
#include "profile.h"
#include "util_id.h"
#include "util_qvariant.h"
#include "window_add_universe.h"
#include "window_api_key_manage.h"
#include "window_main_menu_bar.h"

template <typename T> static QPointer<QMdiSubWindow> create_and_attach_panel(const std::shared_ptr<const ApiKeyProfile>& api_profile, const std::shared_ptr<UniverseProfile>& universe, QMdiArea* const mdi_area)
{
	T* const new_panel = new T{ nullptr, api_profile->get_key() };
	new_panel->change_universe(universe);
	return mdi_area->addSubWindow(new_panel);
}

MyNewMainWindow::MyNewMainWindow() : QMainWindow{ nullptr, Qt::Window }
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle("OpenCloudTools");
	setMinimumSize(650, 500);

	connect(&(UserProfile::get()), &UserProfile::active_api_key_changed, this, &MyNewMainWindow::handle_active_api_key_changed);

	MyMainWindowMenuBar* const menu_bar = new MyMainWindowMenuBar{ this };
	setMenuBar(menu_bar);

	connect(menu_bar, &MyMainWindowMenuBar::request_close, this, &MyNewMainWindow::close);
#ifdef OCT_NEW_GUI
	connect(menu_bar, &MyMainWindowMenuBar::request_show_http_log, this, &MyNewMainWindow::show_http_log);
#endif

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
	center_mdi_widget->setOption(QMdiArea::DontMaximizeSubWindowOnActivation, true);
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
	close_all_subwindows();

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

void MyNewMainWindow::handle_universe_details_changed(const UniverseProfile::Id id)
{
	close_universe_subwindows(id);
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
		static const std::vector<SubwindowType> subwindow_types = std::vector<SubwindowType>{
			SubwindowType::DATA_STORES_STANDARD,
			SubwindowType::DATA_STORES_ORDERED,
			SubwindowType::MEMORY_STORE_SORTED_MAP,
			SubwindowType::BULK_DATA,
			SubwindowType::MESSAGING,
			SubwindowType::UNIVERSE_PREFERENCES,
		};
		const QByteArray this_universe_id = this_universe->get_id().as_q_byte_array();
		QTreeWidgetItem* const this_item = new QTreeWidgetItem{ tree_universe };
		this_item->setText(0, this_universe->get_display_name());
		this_item->setData(0, Qt::UserRole, this_universe_id);
		for (const SubwindowType subwindow_type : subwindow_types)
		{
			QTreeWidgetItem* const subwindow_item = new QTreeWidgetItem{ this_item };
			subwindow_item->setText(0, subwindow_type_display_name(subwindow_type));
			subwindow_item->setData(0, Qt::UserRole, static_cast<int>(subwindow_type));
		}
		this_item->setExpanded(true);
	}

	gui_refresh();
}

void MyNewMainWindow::close_all_subwindows()
{
	for (const auto& this_pair : subwindows)
	{
		QMdiSubWindow* const this_subwindow = this_pair.second;
		if (this_subwindow)
		{
			this_subwindow->close();
		}
	}
	subwindows.clear();
}

void MyNewMainWindow::close_universe_subwindows(const UniverseProfile::Id& id)
{
	for (const auto& this_pair : subwindows)
	{
		const SubwindowId& subwindow_id = this_pair.first;
		const UniverseProfile::Id& universe_profile_id = subwindow_id.get_universe_profile_id();
		if (id == universe_profile_id)
		{
			const QPointer<QMdiSubWindow>& this_subwindow = this_pair.second;
			if (this_subwindow)
			{
				this_subwindow->close();
			}
		}
	}
}

void MyNewMainWindow::show_http_log()
{
	if (subwindow_http_log)
	{
		subwindow_http_log->show();
		subwindow_http_log->setFocus();
		return;
	}

	HttpLogPanel* const new_panel = new HttpLogPanel{ nullptr };
	subwindow_http_log = center_mdi_widget->addSubWindow(new_panel);
	subwindow_http_log->setWindowTitle("HTTP Log");
	QSize window_size = subwindow_http_log->size();
	window_size.setWidth(600);
	subwindow_http_log->resize(window_size);
	subwindow_http_log->show();
}

void MyNewMainWindow::show_subwindow(const SubwindowId& id)
{
	const std::shared_ptr<ApiKeyProfile> api_profile = attached_profile.lock();
	if (!api_profile)
	{
		OCTASSERT(false);
		return;
	}

	const std::map<SubwindowId, QPointer<QMdiSubWindow>>::const_iterator existing_iter = subwindows.find(id);
	if (existing_iter != subwindows.end())
	{
		const QPointer<QMdiSubWindow>& subwindow = existing_iter->second;
		if (subwindow)
		{
			subwindow->show();
			subwindow->setFocus();
			return;
		}
	}

	const std::shared_ptr<UniverseProfile> universe = api_profile->get_universe_profile_by_id(id.get_universe_profile_id());
	if (!universe)
	{
		OCTASSERT(false);
		return;
	}

	QPointer<QMdiSubWindow> new_subwindow;
	switch (id.get_type())
	{
		case SubwindowType::DATA_STORES_STANDARD:
			new_subwindow = create_and_attach_panel<StandardDatastorePanel>(api_profile, universe, center_mdi_widget);
			break;
		case SubwindowType::DATA_STORES_ORDERED:
			new_subwindow = create_and_attach_panel<OrderedDatastorePanel>(api_profile, universe, center_mdi_widget);
			break;
		case SubwindowType::MEMORY_STORE_SORTED_MAP:
			new_subwindow = create_and_attach_panel<MemoryStoreSortedMapPanel>(api_profile, universe, center_mdi_widget);
			break;
		case SubwindowType::BULK_DATA:
			new_subwindow = create_and_attach_panel<BulkDataPanel>(api_profile, universe, center_mdi_widget);
			break;
		case SubwindowType::MESSAGING:
			new_subwindow = create_and_attach_panel<MessagingServicePanel>(api_profile, universe, center_mdi_widget);
			break;
		case SubwindowType::UNIVERSE_PREFERENCES:
			new_subwindow = create_and_attach_panel<UniversePreferencesPanel>(api_profile, universe, center_mdi_widget);
			break;
	}

	if (!new_subwindow)
	{
		OCTASSERT(false);
		return;
	}

	new_subwindow->setWindowTitle(id.get_window_title());

	subwindows[id] = new_subwindow;
	new_subwindow->show();
}

void MyNewMainWindow::show_subwindow_from_item(QTreeWidgetItem* const item)
{
	QTreeWidgetItem* const item_parent = item->parent();
	if (item_parent == nullptr)
	{
		// User double-clicked a universe label, do nothing
		return;
	}

	const QVariant universe_profile_id_var = item_parent->data(0, Qt::UserRole);
	if (qvariant_is_byte_array(universe_profile_id_var, static_cast<int>(RandomId128::LENGTH)) == false)
	{
		OCTASSERT(false);
		return;
	}
	const UniverseProfile::Id universe_profile_id{ universe_profile_id_var.toByteArray() };

	const QVariant type_var = item->data(0, Qt::UserRole);
	const SubwindowType type = static_cast<SubwindowType>(type_var.toInt());

	const SubwindowId subwindow_id{ type, universe_profile_id };
	show_subwindow(subwindow_id);
}
