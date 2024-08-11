#include "window_main_menu_bar.h"

#include <string>

#include <QtGlobal>
#include <QAction>
#include <QDesktopServices>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QString>
#include <QStyleFactory>
#include <QUrl>

#include <sqlite3.h>

#include "assert.h"
#include "build_info.h"
#include "profile.h"
#include "window_api_key_manage.h"

MyMainWindowMenuBar::MyMainWindowMenuBar(QMainWindow* parent) : QMenuBar{ parent }
{
	connect(&(UserProfile::get()), &UserProfile::qt_theme_changed, this, &MyMainWindowMenuBar::handle_qt_theme_changed);
	connect(&(UserProfile::get()), &UserProfile::autoclose_changed, this, &MyMainWindowMenuBar::handle_autoclose_changed);

	QMenu* const file_menu = new QMenu{ "&File", this };
	{
		QAction* const action_change_key = new QAction{ "Change API &key", file_menu };
		connect(action_change_key, &QAction::triggered, this, &MyMainWindowMenuBar::pressed_change_api_key);

		QAction* const action_exit = new QAction{ "E&xit", file_menu };
		connect(action_exit, &QAction::triggered, this, &MyMainWindowMenuBar::request_close);

		file_menu->addAction(action_change_key);
		file_menu->addSeparator();
		file_menu->addAction(action_exit);
	}

	QMenu* const preferences_menu = new QMenu{ "&Preferences", this };
	{
		QMenu* const theme_menu = new QMenu{ "Theme", preferences_menu };
		{
			for (const QString& this_theme : QStyleFactory::keys())
			{
				QAction* const this_action = new QAction{ this_theme, theme_menu };
				connect(this_action, &QAction::triggered, [this_theme]() {
					UserProfile::get().set_qt_theme(this_theme);
				});
				theme_actions.push_back(this_action);
				theme_menu->addAction(this_action);
				if (this_theme.toLower() == "fusion")
				{
					QAction* fusion_dark_action = new QAction{ "Fusion (Dark)", theme_menu };
					connect(fusion_dark_action, &QAction::triggered, []() {
						UserProfile::get().set_qt_theme("_fusion_dark");
					});
					theme_actions.push_back(fusion_dark_action);
					theme_menu->addAction(fusion_dark_action);
				}
			}
		}

		action_toggle_autoclose = new QAction{ "&Close progress windows when complete", preferences_menu };
		action_toggle_autoclose->setCheckable(true);
		action_toggle_autoclose->setChecked(UserProfile::get().get_autoclose_progress_window());
		connect(action_toggle_autoclose, &QAction::triggered, this, &MyMainWindowMenuBar::pressed_toggle_autoclose);

		action_toggle_less_verbose_bulk = new QAction{ "Less &verbose bulk data operations", preferences_menu };
		action_toggle_less_verbose_bulk->setCheckable(true);
		action_toggle_less_verbose_bulk->setChecked(UserProfile::get().get_less_verbose_bulk_operations());
		connect(action_toggle_less_verbose_bulk, &QAction::triggered, this, &MyMainWindowMenuBar::pressed_toggle_less_verbose_bulk);

		action_toggle_datastore_name_filter = new QAction{ "Show datastore name &filter text box", preferences_menu };
		action_toggle_datastore_name_filter->setCheckable(true);
		action_toggle_datastore_name_filter->setChecked(UserProfile::get().get_show_datastore_name_filter());
		connect(action_toggle_datastore_name_filter, &QAction::triggered, this, &MyMainWindowMenuBar::pressed_toggle_datastore_name_filter);

		preferences_menu->addMenu(theme_menu);
		preferences_menu->addSeparator();
		preferences_menu->addAction(action_toggle_autoclose);
		preferences_menu->addAction(action_toggle_less_verbose_bulk);
		preferences_menu->addAction(action_toggle_datastore_name_filter);
	}

	QMenu* const about_menu = new QMenu{ "&About", this };
	{
		QAction* const action_github = new QAction{ "Visit repository on &Github", about_menu };
		connect(action_github, &QAction::triggered, []() {
			QDesktopServices::openUrl(QUrl{ "https://github.com/jlwitthuhn/OpenCloudTools" });
		});

#ifdef GIT_DESCRIBE
		const QString label_git_describe = QString{ "Git: %1" }.arg(GIT_DESCRIBE);
		QAction* action_git_describe = new QAction{ label_git_describe, about_menu };
#endif

		QMenu* const about_build_menu = new QMenu{ "Build information", about_menu };
		{
			const QString label_date = QString{ "Build date: %1" }.arg(QString::fromStdString(get_build_date()));
			QAction* const action_date = new QAction{ label_date, about_build_menu };

			const QString label_compiler = QString{ "Compiler: %1" }.arg(QString::fromStdString(get_cxx_compiler_version_string()));
			QAction* const action_compiler = new QAction{ label_compiler, about_build_menu };

			about_build_menu->addAction(action_date);
			about_build_menu->addAction(action_compiler);
		}

		QMenu* const about_libraries_menu = new QMenu{ "Third-party libraries", about_menu };
		{
			const QString label_qt = QString{ "Qt %1" }.arg(qVersion());
			QAction* const action_qt = new QAction{ label_qt, about_libraries_menu };

			const QString label_sqlite = QString{ "SQLite %1" }.arg(sqlite3_libversion());
			QAction* const action_sqlite = new QAction{ label_sqlite, about_libraries_menu };

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

	addMenu(file_menu);
	addMenu(preferences_menu);
	addMenu(about_menu);

	handle_qt_theme_changed();
}

void MyMainWindowMenuBar::handle_autoclose_changed()
{
	// This can be toggled form elsewhere in the program, this keeps it in syn everywhere
	const bool autoclose = UserProfile::get().get_autoclose_progress_window();
	if (autoclose != action_toggle_autoclose->isChecked())
	{
		action_toggle_autoclose->setChecked(autoclose);
	}
}

void MyMainWindowMenuBar::handle_qt_theme_changed()
{
	const QString& selected_theme = UserProfile::get().get_qt_theme();
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

void MyMainWindowMenuBar::pressed_change_api_key()
{
#ifdef OCT_NEW_GUI
	QMainWindow* const parent_window = dynamic_cast<QMainWindow*>(window());
	OCTASSERT(parent_window);
	ManageApiKeysWindow* const manage_key_window = new ManageApiKeysWindow{ parent_window };
	manage_key_window->show();
#else
	emit OLDGUI_request_change_api_key();
#endif
}

void MyMainWindowMenuBar::pressed_toggle_autoclose()
{
	UserProfile::get().set_autoclose_progress_window(action_toggle_autoclose->isChecked());
}

void MyMainWindowMenuBar::pressed_toggle_datastore_name_filter()
{
	UserProfile::get().set_show_datastore_name_filter(action_toggle_datastore_name_filter->isChecked());
}

void MyMainWindowMenuBar::pressed_toggle_less_verbose_bulk()
{
	UserProfile::get().set_less_verbose_bulk_operations(action_toggle_less_verbose_bulk->isChecked());
}
