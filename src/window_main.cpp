#include "window_main.h"

#include <cstddef>

#include <memory>
#include <optional>
#include <vector>

#include <Qt>
#include <QtGlobal>
#include <QAction>
#include <QComboBox>
#include <QDesktopServices>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLayout>
#include <QLineEdit>
#include <QList>
#include <QMargins>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QStyleFactory>
#include <QTabWidget>
#include <QUrl>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include <sqlite3.h>

#include "build_info.h"
#include "http_wrangler.h"
#include "panel_bulk_data.h"
#include "panel_datastore_standard.h"
#include "panel_datastore_ordered.h"
#include "panel_http_log.h"
#include "panel_messaging_service.h"
#include "panel_universe_prefs.h"
#include "profile.h"
#include "window_api_key_manage.h"

MyMainWindow::MyMainWindow(QWidget* parent, QString title, QString api_key) : QMainWindow{ parent, Qt::Window }, api_key { api_key }
{
	setWindowTitle(QString{ "OpenCloudTools: " } + title);

	connect(UserProfile::get().get(), &UserProfile::qt_theme_changed, this, &MyMainWindow::handle_qt_theme_changed);
	connect(UserProfile::get().get(), &UserProfile::universe_list_changed, this, &MyMainWindow::handle_universe_list_changed);
	connect(UserProfile::get().get(), &UserProfile::autoclose_changed, this, &MyMainWindow::handle_autoclose_changed);

	QMenuBar* menu_bar = new QMenuBar{ this };
	{

		QMenu* file_menu = new QMenu{ "&File", menu_bar };
		{
			QAction* action_change_key = new QAction{ "Change API &key", menu_bar };
			connect(action_change_key, &QAction::triggered, this, &MyMainWindow::pressed_change_key);

			QAction* action_exit = new QAction{ "E&xit", menu_bar };
			connect(action_exit, &QAction::triggered, this, &MyMainWindow::close);

			file_menu->addAction(action_change_key);
			file_menu->addSeparator();
			file_menu->addAction(action_exit);
		}

		QMenu* preferences_menu = new QMenu{ "&Preferences", menu_bar };
		{
			QMenu* theme_menu = new QMenu{ "Theme", preferences_menu };
			{
				for (const QString& this_theme : QStyleFactory::keys())
				{
					QAction* this_action = new QAction{ this_theme, theme_menu };
					connect(this_action, &QAction::triggered, [this_theme]() {
						UserProfile::get()->set_qt_theme(this_theme);
					});
					theme_actions.push_back(this_action);
					theme_menu->addAction(this_action);
					if (this_theme.toLower() == "fusion")
					{
						QAction* fusion_dark_action = new QAction{ "Fusion (Dark)", theme_menu };
						connect(fusion_dark_action, &QAction::triggered, []() {
							UserProfile::get()->set_qt_theme("_fusion_dark");
						});
						theme_actions.push_back(fusion_dark_action);
						theme_menu->addAction(fusion_dark_action);
					}
				}
			}

			action_toggle_autoclose = new QAction{ "&Automatically close progress window", preferences_menu };
			action_toggle_autoclose->setCheckable(true);
			action_toggle_autoclose->setChecked(UserProfile::get()->get_autoclose_progress_window());
			connect(action_toggle_autoclose, &QAction::triggered, this, &MyMainWindow::pressed_toggle_autoclose);

			action_toggle_less_verbose_bulk = new QAction{ "Less &verbose bulk data operations", preferences_menu };
			action_toggle_less_verbose_bulk->setCheckable(true);
			action_toggle_less_verbose_bulk->setChecked(UserProfile::get()->get_less_verbose_bulk_operations());
			connect(action_toggle_less_verbose_bulk, &QAction::triggered, this, &MyMainWindow::pressed_toggle_less_verbose_bulk);

			action_toggle_datastore_name_filter = new QAction{ "Show datastore name &filter text box", preferences_menu };
			action_toggle_datastore_name_filter->setCheckable(true);
			action_toggle_datastore_name_filter->setChecked(UserProfile::get()->get_show_datastore_name_filter());
			connect(action_toggle_datastore_name_filter, &QAction::triggered, this, &MyMainWindow::pressed_toggle_datastore_name_filter);

			preferences_menu->addMenu(theme_menu);
			preferences_menu->addSeparator();
			preferences_menu->addAction(action_toggle_autoclose);
			preferences_menu->addAction(action_toggle_less_verbose_bulk);
			preferences_menu->addAction(action_toggle_datastore_name_filter);
		}

		QMenu* about_menu = new QMenu{ "&About", menu_bar };
		{
			QAction* action_github = new QAction{ "Visit repository on &Github", menu_bar };
			connect(action_github, &QAction::triggered, []() {
				QDesktopServices::openUrl(QUrl{ "https://github.com/jlwitthuhn/OpenCloudTools" });
			});

#ifdef GIT_DESCRIBE
			const QString label_git_describe = QString{ "Git: %1" }.arg(GIT_DESCRIBE);
			QAction* action_git_describe = new QAction{ label_git_describe, menu_bar };
#endif

			QMenu* about_build_menu = new QMenu{ "Build information", about_menu };
			{
				const QString label_date = QString{ "Build date: %1" }.arg(QString::fromStdString(get_build_date()));
				QAction* action_date = new QAction{ label_date, menu_bar };

				const QString label_compiler = QString{ "Compiler: %1" }.arg( QString::fromStdString( get_cxx_compiler_version_string() ) );
				QAction* action_compiler = new QAction{ label_compiler, menu_bar };

				about_build_menu->addAction(action_date);
				about_build_menu->addAction(action_compiler);
			}

			QMenu* about_libraries_menu = new QMenu{ "Third-party libraries", about_menu };
			{
				const QString label_qt = QString{ "Qt %1" }.arg(qVersion());
				QAction* action_qt = new QAction{ label_qt, menu_bar };

				const QString label_sqlite = QString{ "SQLite %1" }.arg(sqlite3_libversion());
				QAction* action_sqlite = new QAction{ label_sqlite, menu_bar };

				about_libraries_menu->addAction(action_qt);
				about_libraries_menu->addAction(action_sqlite);
			}

			about_menu->addAction(action_github);
			about_menu->addSeparator();
#ifdef GIT_DESCRIBE
			about_menu->addAction(action_git_describe);
#endif
			about_menu->addMenu(about_build_menu);
			about_menu->addMenu(about_libraries_menu);
		}

		menu_bar->addMenu(file_menu);
		menu_bar->addMenu(preferences_menu);
		menu_bar->addMenu(about_menu);
	}
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

	handle_qt_theme_changed();
	handle_universe_list_changed(std::nullopt);
}

void MyMainWindow::selected_universe_combo_changed()
{
	if (select_universe_combo->count() > 0)
	{
		const size_t universe_index = select_universe_combo->currentData().toULongLong();
		UserProfile::get_selected_api_key()->select_universe(universe_index);
	}
	else
	{
		UserProfile::get_selected_api_key()->select_universe(std::nullopt);
	}
	edit_universe_button->setEnabled(select_universe_combo->count() > 0);
	del_universe_button->setEnabled(select_universe_combo->count() > 0);
	// TODO: replace this with a signal that is set up on construction
	standard_datastore_panel->selected_universe_changed();
	ordered_datastore_panel->selected_universe_changed();
	bulk_data_panel->selected_universe_changed();
	messaging_service_panel->selected_universe_changed();
	universe_preferences_panel->selected_universe_changed();
}

void MyMainWindow::pressed_add_universe()
{
	MainWindowAddUniverseWindow* modal_window = new MainWindowAddUniverseWindow{ this };
	modal_window->setWindowModality(Qt::WindowModality::ApplicationModal);
	modal_window->show();
}

void MyMainWindow::pressed_edit_universe()
{
	MainWindowAddUniverseWindow* modal_window = new MainWindowAddUniverseWindow{ this, true };
	modal_window->setWindowModality(Qt::WindowModality::ApplicationModal);
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
		UserProfile::get_selected_api_key()->remove_selected_universe();
	}
}

void MyMainWindow::pressed_change_key()
{
	HttpWrangler::clear_log();
	ManageApiKeysWindow* api_window = new ManageApiKeysWindow{ nullptr };
	api_window->show();
	close();
}

void MyMainWindow::pressed_toggle_autoclose()
{
	UserProfile::get()->set_autoclose_progress_window(action_toggle_autoclose->isChecked());
}

void MyMainWindow::pressed_toggle_datastore_name_filter()
{
	UserProfile::get()->set_show_datastore_name_filter(action_toggle_datastore_name_filter->isChecked());
}

void MyMainWindow::pressed_toggle_less_verbose_bulk()
{
	UserProfile::get()->set_less_verbose_bulk_operations(action_toggle_less_verbose_bulk->isChecked());
}


void MyMainWindow::handle_autoclose_changed()
{
	const bool autoclose = UserProfile::get()->get_autoclose_progress_window();
	if (autoclose != action_toggle_autoclose->isChecked())
	{
		action_toggle_autoclose->setChecked(autoclose);
	}
}

void MyMainWindow::handle_qt_theme_changed()
{
	const QString& selected_theme = UserProfile::get()->get_qt_theme();
	for (QAction* const this_action : theme_actions)
	{
		if (this_action->text() == selected_theme || (this_action->text() == "Fusion (Dark)" && selected_theme == "_fusion_dark"))
		{
			this_action->setCheckable(true);
			this_action->setChecked(true);
		}
		else
		{
			this_action->setCheckable(false);
			this_action->setChecked(false);
		}
	}
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

void MyMainWindow::handle_universe_list_changed(std::optional<size_t> universe_index)
{
	ApiKeyProfile* this_profile{ UserProfile::get_selected_api_key()};
	if (this_profile)
	{
		select_universe_combo->clear();
		for (size_t i = 0; i < this_profile->get_universe_list().size(); i++)
		{
			const UniverseProfile* const this_universe = this_profile->get_universe_list().at(i);
			QString formatted = QString{ "%1 [%2]" }.arg(this_universe->get_name()).arg(this_universe->get_universe_id());
			select_universe_combo->addItem(formatted, QVariant{ static_cast<unsigned long long>(i) });
		}
		if (universe_index)
		{
			for (size_t i = 0; i < this_profile->get_universe_list().size(); i++)
			{
				if (select_universe_combo->itemData(static_cast<int>(i)) == static_cast<unsigned long long>(*universe_index))
				{
					select_universe_combo->setCurrentIndex(static_cast<int>(i));
					break;
				}
			}
		}
	}
	selected_universe_combo_changed();
}


MainWindowAddUniverseWindow::MainWindowAddUniverseWindow(QWidget* const parent, const bool edit_current) : QWidget{ parent, Qt::Window }
{
	setAttribute(Qt::WA_DeleteOnClose);

	const UniverseProfile* const existing_universe = edit_current ? UserProfile::get_selected_universe() : nullptr;
	edit_mode = existing_universe != nullptr;

	if (edit_mode)
	{
		setWindowTitle("Edit universe");
	}
	else
	{
		setWindowTitle("Add universe");
	}

	QWidget* info_panel = new QWidget{ this };
	{
		name_edit = new QLineEdit{ info_panel };
		if (edit_mode)
		{
			name_edit->setText(existing_universe->get_name());
		}
		connect(name_edit, &QLineEdit::textChanged, this, &MainWindowAddUniverseWindow::text_changed);

		id_edit = new QLineEdit{ info_panel };
		if (edit_mode)
		{
			id_edit->setText(QString::number(existing_universe->get_universe_id()));
		}
		connect(id_edit, &QLineEdit::textChanged, this, &MainWindowAddUniverseWindow::text_changed);

		QFormLayout* info_layout = new QFormLayout{ info_panel };
		info_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		info_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		info_layout->addRow("Name", name_edit);
		info_layout->addRow("Universe ID", id_edit);
	}

	QWidget* button_panel = new QWidget{ this };
	button_panel->setMinimumWidth(280);
	{
		add_button = new QPushButton{ "Add", button_panel };
		if (edit_mode)
		{
			add_button->setText("Update");
		}
		connect(add_button, &QPushButton::clicked, this, &MainWindowAddUniverseWindow::pressed_add);

		QPushButton* cancel_button = new QPushButton{ "Cancel", button_panel };

		QHBoxLayout* button_layout = new QHBoxLayout{ button_panel };
		button_layout->addWidget(add_button);
		button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		button_layout->addWidget(cancel_button);
		connect(cancel_button, &QPushButton::clicked, this, &QWidget::close);
	}

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
	layout->addWidget(info_panel);
	layout->addWidget(button_panel);

	text_changed();
}

bool MainWindowAddUniverseWindow::input_is_valid() const
{
	const long long universe_id = id_edit->text().trimmed().toLongLong();
	return name_edit->text().size() > 0 && universe_id > 100000L;
}

void MainWindowAddUniverseWindow::text_changed()
{
	add_button->setEnabled(input_is_valid());
}

void MainWindowAddUniverseWindow::pressed_add()
{
	const QString name = name_edit->text();
	const long long universe_id = id_edit->text().trimmed().toLongLong();
	if (edit_mode)
	{
		if (UserProfile::get_selected_universe()->set_details(name, universe_id))
		{
			close();
		}
		else
		{
			QMessageBox* msg_box = new QMessageBox{ this };
			msg_box->setWindowTitle("Update Failed");
			msg_box->setText("Failed to update universe. A universe with that name and id already exists.");
			msg_box->exec();
		}
	}
	else
	{
		const std::optional<size_t> new_id = UserProfile::get_selected_api_key()->add_universe(name, universe_id);
		if (new_id)
		{
			close();
		}
		else
		{
			QMessageBox* msg_box = new QMessageBox{ this };
			msg_box->setWindowTitle("Add Failed");
			msg_box->setText("Failed to add new universe. A universe with that name and id already exists.");
			msg_box->exec();
		}
	}
}
