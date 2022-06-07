#include "window_main.h"

#include <map>
#include <memory>
#include <utility>

#include <Qt>
#include <QAction>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMargins>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "api_key.h"
#include "panel_datastore_explore.h"
#include "user_settings.h"
#include "window_api_key_manage.h"
#include "window_datastore_universe_add.h"

MyMainWindow::MyMainWindow(QWidget* parent, QString title, QString api_key) : QMainWindow{ parent, Qt::Window }, api_key { api_key }
{
	setWindowTitle(QString{ "OpenCloudTools: " } + title);

	connect(UserSettings::get().get(), &UserSettings::universe_list_changed, this, &MyMainWindow::universe_list_changed);
	connect(UserSettings::get().get(), &UserSettings::autoclose_changed, this, &MyMainWindow::handle_autoclose_changed);

	QMenuBar* menu_bar = new QMenuBar{ this };
	{
		QAction* action_change_key = new QAction{ "Change API &key", menu_bar };
		connect(action_change_key, &QAction::triggered, this, &MyMainWindow::pressed_change_key);

		QAction* action_exit = new QAction{ "E&xit", menu_bar };
		connect(action_exit, &QAction::triggered, this, &MyMainWindow::close);

		QMenu* file_menu = new QMenu{ "&File", menu_bar };
		file_menu->addAction(action_change_key);
		file_menu->addSeparator();
		file_menu->addAction(action_exit);
		menu_bar->addMenu(file_menu);

		action_toggle_autoclose = new QAction{ "&Automatically close progress window" };
		action_toggle_autoclose->setCheckable(true);
		action_toggle_autoclose->setChecked(UserSettings::get()->get_autoclose_progress_window());
		connect(action_toggle_autoclose, &QAction::triggered, this, &MyMainWindow::pressed_toggle_autoclose);

		QMenu* preferences_menu = new QMenu{ "&Preferences", menu_bar };
		preferences_menu->addAction(action_toggle_autoclose);
		menu_bar->addMenu(preferences_menu);
	}
	setMenuBar(menu_bar);

	QWidget* central_widget = new QWidget{ this };
	{
		QWidget* top_bar_widget = new QWidget{ central_widget };
		{
			QGroupBox* select_universe_group = new QGroupBox{ "Select universe", top_bar_widget };
			{
				select_universe_combo = new QComboBox{ select_universe_group };
				connect(select_universe_combo, &QComboBox::currentIndexChanged, this, &MyMainWindow::selected_universe_changed);

				QPushButton* add_universe_button = new QPushButton{ "Add...", select_universe_group };
				add_universe_button->setMaximumWidth(150);
				connect(add_universe_button, &QPushButton::clicked, this, &MyMainWindow::pressed_add_universe);

				del_universe_button = new QPushButton{ "Remove", select_universe_group };
				del_universe_button->setMaximumWidth(150);
				connect(del_universe_button, &QPushButton::clicked, this, &MyMainWindow::pressed_remove_universe);

				QHBoxLayout* select_universe_layout = new QHBoxLayout{ select_universe_group };
				select_universe_layout->addWidget(select_universe_combo);
				select_universe_layout->addWidget(add_universe_button);
				select_universe_layout->addWidget(del_universe_button);
			}

			QVBoxLayout* top_bar_layout = new QVBoxLayout{ top_bar_widget };
			top_bar_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
			top_bar_layout->addWidget(select_universe_group);
		}

		explore_datastore_panel = new ExploreDatastorePanel{ central_widget, api_key, selected_universe_id };

		QVBoxLayout* central_layout = new QVBoxLayout{ central_widget };
		central_layout->addWidget(top_bar_widget);
		central_layout->addWidget(explore_datastore_panel);
	}
	setCentralWidget(central_widget);

	setMinimumWidth(700);
	setMinimumHeight(500);

	universe_list_changed();
}

void MyMainWindow::selected_universe_changed()
{
	if (select_universe_combo->count() > 0)
	{
		const long long universe_id = select_universe_combo->currentData().toLongLong();
		selected_universe_id = universe_id;
	}
	else
	{
		selected_universe_id = std::nullopt;
	}
	del_universe_button->setEnabled(select_universe_combo->count() > 0);
	explore_datastore_panel->selected_universe_changed(selected_universe_id);
}

void MyMainWindow::universe_list_changed()
{
	std::optional<ApiKeyProfile> this_profile{ UserSettings::get()->get_selected_profile() };
	if (this_profile)
	{
		select_universe_combo->clear();
		for (const std::pair<const long long, QString>& this_universe : this_profile->universe_ids())
		{
			QString formatted = QString{ "%1 [%2]" }.arg(this_universe.second).arg(this_universe.first);
			select_universe_combo->addItem(formatted, QVariant{ this_universe.first });
		}
	}
	selected_universe_changed();
}

void MyMainWindow::pressed_add_universe()
{
	AddUniverseToDatastoreWindow* modal_window = new AddUniverseToDatastoreWindow{ this };
	modal_window->setWindowModality(Qt::WindowModality::ApplicationModal);
	modal_window->show();
}

void MyMainWindow::pressed_remove_universe()
{
	if (selected_universe_id)
	{
		UserSettings::get()->selected_remove_universe(*selected_universe_id);
	}
}

void MyMainWindow::pressed_change_key()
{
	ManageApiKeysWindow* api_window = new ManageApiKeysWindow{ nullptr };
	api_window->show();
	close();
}

void MyMainWindow::pressed_toggle_autoclose()
{
	UserSettings::get()->set_autoclose_progress_window(action_toggle_autoclose->isChecked());
}

void MyMainWindow::handle_autoclose_changed()
{
	const bool autoclose = UserSettings::get()->get_autoclose_progress_window();
	if (autoclose != action_toggle_autoclose->isChecked())
	{
		action_toggle_autoclose->setChecked(autoclose);
	}
}
