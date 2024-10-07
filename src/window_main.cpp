#include "window_main.h"

#include <cstddef>

#include <memory>
#include <optional>
#include <vector>

#include <Qt>
#include <QByteArray>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMargins>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "assert.h"
#include "http_wrangler.h"
#include "panel_bulk_data.h"
#include "panel_datastore_standard.h"
#include "panel_datastore_ordered.h"
#include "panel_http_log.h"
#include "panel_mem_sorted_map.h"
#include "panel_messaging_service.h"
#include "panel_universe_prefs.h"
#include "profile.h"
#include "util_id.h"
#include "window_add_universe.h"
#include "window_api_key_manage.h"
#include "window_main_menu_bar.h"

MyMainWindow::MyMainWindow(QWidget* parent, QString title, QString api_key) : QMainWindow{ parent, Qt::Window }, api_key { api_key }
{
	setWindowTitle(QString{ "OpenCloudTools: " } + title);
	setAttribute(Qt::WA_DeleteOnClose);

	// API Key should never change for a given main window
	connect(&(UserProfile::get()), &UserProfile::active_api_key_changed, this, [] { OCTASSERT(false); });

	const std::shared_ptr<ApiKeyProfile> api_profile = UserProfile::get_active_api_key();
	connect(api_profile.get(), &ApiKeyProfile::universe_list_changed, this, &MyMainWindow::handle_universe_list_changed);

	MyMainWindowMenuBar* menu_bar = new MyMainWindowMenuBar{ this };
	connect(menu_bar, &MyMainWindowMenuBar::OLDGUI_request_change_api_key, this, &MyMainWindow::pressed_change_key);
	connect(menu_bar, &MyMainWindowMenuBar::request_close, this, &MyMainWindow::close);
	setMenuBar(menu_bar);

	QWidget* central_widget = new QWidget{ this };
	{
		QWidget* top_bar_widget = new QWidget{ central_widget };
		{
			QGroupBox* select_universe_group = new QGroupBox{ "Select universe", top_bar_widget };
			{
				select_universe_combo = new QComboBox{ select_universe_group };
#ifdef QT5_COMPAT
				connect(select_universe_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MyMainWindow::selected_universe_combo_changed);
#else
				connect(select_universe_combo, &QComboBox::currentIndexChanged, this, &MyMainWindow::selected_universe_combo_changed);
#endif

				QPushButton* add_universe_button = new QPushButton{ "Add...", select_universe_group };
				add_universe_button->setMaximumWidth(120);
				connect(add_universe_button, &QPushButton::clicked, this, &MyMainWindow::pressed_add_universe);

				edit_universe_button = new QPushButton{ "Edit...", select_universe_group };
				edit_universe_button->setMaximumWidth(120);
				connect(edit_universe_button, &QPushButton::clicked, this, &MyMainWindow::pressed_edit_universe);

				del_universe_button = new QPushButton{ "Delete", select_universe_group };
				del_universe_button->setMaximumWidth(120);
				connect(del_universe_button, &QPushButton::clicked, this, &MyMainWindow::pressed_remove_universe);

				QHBoxLayout* select_universe_layout = new QHBoxLayout{ select_universe_group };
				select_universe_layout->addWidget(select_universe_combo);
				select_universe_layout->addWidget(add_universe_button);
				select_universe_layout->addWidget(edit_universe_button);
				select_universe_layout->addWidget(del_universe_button);
			}

			QVBoxLayout* top_bar_layout = new QVBoxLayout{ top_bar_widget };
			top_bar_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
			top_bar_layout->addWidget(select_universe_group);
		}

		panel_tabs = new QTabWidget{ central_widget };
		{
			standard_datastore_panel = new StandardDatastorePanel{ panel_tabs, api_key };
			panel_tabs->addTab(standard_datastore_panel, "Datastore");

			ordered_datastore_panel = new OrderedDatastorePanel{ panel_tabs, api_key };
			panel_tabs->addTab(ordered_datastore_panel, "Ordered Datastore");

			memory_store_panel = new MemoryStoreSortedMapPanel{ panel_tabs, api_key };
			panel_tabs->addTab(memory_store_panel, "Memstore Sorted Map");

			bulk_data_panel = new BulkDataPanel{ panel_tabs, api_key };
			panel_tabs->addTab(bulk_data_panel, "Bulk Data");

			messaging_service_panel = new MessagingServicePanel{ panel_tabs, api_key };
			panel_tabs->addTab(messaging_service_panel, "Messaging");

			http_log_panel = new HttpLogPanel{ panel_tabs };
			panel_tabs->addTab(http_log_panel, "HTTP Log");

			universe_preferences_panel = new UniversePreferencesPanel{ panel_tabs };
			panel_tabs->addTab(universe_preferences_panel, "Universe Preferences");

			connect(panel_tabs, &QTabWidget::currentChanged, this, &MyMainWindow::handle_tab_changed);
		}

		QVBoxLayout* central_layout = new QVBoxLayout{ central_widget };
		central_layout->addWidget(top_bar_widget);
		central_layout->addWidget(panel_tabs);
	}
	setCentralWidget(central_widget);

	setMinimumWidth(720);
	setMinimumHeight(520);

	handle_universe_list_changed(std::nullopt);
}

void MyMainWindow::selected_universe_combo_changed()
{
	if (select_universe_combo->count() == 0)
	{
		edit_universe_button->setEnabled(false);
		del_universe_button->setEnabled(false);
		
		standard_datastore_panel->change_universe(nullptr);
		ordered_datastore_panel->change_universe(nullptr);
		memory_store_panel->change_universe(nullptr);
		bulk_data_panel->change_universe(nullptr);
		messaging_service_panel->change_universe(nullptr);
		universe_preferences_panel->change_universe(nullptr);
	}
	else
	{
		edit_universe_button->setEnabled(true);
		del_universe_button->setEnabled(true);

		const std::shared_ptr<UniverseProfile> universe = get_selected_universe();

		standard_datastore_panel->change_universe(universe);
		ordered_datastore_panel->change_universe(universe);
		memory_store_panel->change_universe(universe);
		bulk_data_panel->change_universe(universe);
		messaging_service_panel->change_universe(universe);
		universe_preferences_panel->change_universe(universe);
	}
}

std::shared_ptr<UniverseProfile> MyMainWindow::get_selected_universe() const
{
	if (select_universe_combo->count() == 0)
	{
		return nullptr;
	}
	const QByteArray q_universe_id = select_universe_combo->currentData().toByteArray();
	const UniverseProfile::Id universe_id{ q_universe_id };
	return UserProfile::get_active_api_key()->get_universe_profile_by_id(universe_id);
}

void MyMainWindow::pressed_add_universe()
{
	AddUniverseWindow* const modal_window = new AddUniverseWindow{ this, api_key, nullptr };
	modal_window->show();
}

void MyMainWindow::pressed_edit_universe()
{
	const std::shared_ptr<UniverseProfile> universe = get_selected_universe();
	OCTASSERT(static_cast<bool>(universe));
	AddUniverseWindow* const modal_window = new AddUniverseWindow{ this, api_key, universe };
	modal_window->show();
}

void MyMainWindow::pressed_remove_universe()
{
	QMessageBox* msg_box = new QMessageBox{ this };
	msg_box->setWindowTitle("Confirm deletion");
	msg_box->setText("Are you sure you want to delete this universe? This cannot be undone.");
	msg_box->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	int result = msg_box->exec();
	if (result == QMessageBox::Yes)
	{
		const std::shared_ptr<UniverseProfile> universe = get_selected_universe();
		OCTASSERT(static_cast<bool>(universe));
		if (universe)
		{
			UserProfile::get_active_api_key()->delete_universe(universe->get_id());
		}
	}
}

void MyMainWindow::pressed_change_key()
{
	HttpWrangler::clear_log();
	ManageApiKeysWindow* api_window = new ManageApiKeysWindow{ nullptr };
	api_window->show();
	close();
}

void MyMainWindow::handle_tab_changed(const int index)
{
	if (panel_tabs)
	{
		QWidget* the_panel = panel_tabs->widget(index);
		if (HttpLogPanel* http_panel = dynamic_cast<HttpLogPanel*>(the_panel))
		{
			http_panel->tab_opened();
		}
	}
}

void MyMainWindow::handle_universe_list_changed(const std::optional<UniverseProfile::Id> new_universe)
{
	const std::shared_ptr<const ApiKeyProfile> this_profile{ UserProfile::get_active_api_key()};
	if (this_profile)
	{
		for (const QMetaObject::Connection& this_conn : universe_details_updated_conns)
		{
			disconnect(this_conn);
		}
		universe_details_updated_conns.clear();

		std::optional<UniverseProfile::Id> already_selected_id;
		const int already_selected_index = select_universe_combo->currentIndex();
		if (already_selected_index > -1)
		{
			const QByteArray this_q_id = select_universe_combo->itemData(static_cast<int>(already_selected_index)).toByteArray();
			already_selected_id = this_q_id;
		}

		select_universe_combo->clear();

		for (const std::shared_ptr<const UniverseProfile> this_universe : this_profile->get_universe_list())
		{
			const QString formatted = QString{ "%1 [%2]" }.arg(this_universe->get_name()).arg(this_universe->get_universe_id());
			select_universe_combo->addItem(formatted, QVariant{ this_universe->get_id().as_q_byte_array() });

			const QMetaObject::Connection updated_conn = connect(this_universe.get(), &UniverseProfile::details_changed, this,
				[this] {
					handle_universe_list_changed(std::nullopt);
				}
			);
			universe_details_updated_conns.push_back(updated_conn);

			if (already_selected_id && !new_universe)
			{
				if (this_universe->get_id() == *already_selected_id)
				{
					select_universe_combo->setCurrentIndex(select_universe_combo->count() - 1);
				}
			}
		}

		if (new_universe)
		{
			for (size_t i = 0; i < this_profile->get_universe_list().size(); i++)
			{
				const QByteArray this_q_id = select_universe_combo->itemData(static_cast<int>(i)).toByteArray();
				const UniverseProfile::Id this_id{ this_q_id };
				if (this_id == *new_universe)
				{
					select_universe_combo->setCurrentIndex(static_cast<int>(i));
					break;
				}
			}
		}
	}
	selected_universe_combo_changed();
}
