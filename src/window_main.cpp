#include "window_main.h"

#include <cstddef>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <Qt>
#include <QtGlobal>
#include <QAction>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLayout>
#include <QLineEdit>
#include <QList>
#include <QMargins>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "assert.h"
#include "data_request.h"
#include "diag_operation_in_progress.h"
#include "http_wrangler.h"
#include "panel_bulk_data.h"
#include "panel_datastore_standard.h"
#include "panel_datastore_ordered.h"
#include "panel_http_log.h"
#include "panel_messaging_service.h"
#include "panel_universe_prefs.h"
#include "profile.h"
#include "window_api_key_manage.h"
#include "window_main_menu_bar.h"

MyMainWindow::MyMainWindow(QWidget* parent, QString title, QString api_key) : QMainWindow{ parent, Qt::Window }, api_key { api_key }
{
	setWindowTitle(QString{ "OpenCloudTools: " } + title);

	connect(&(UserProfile::get()), &UserProfile::universe_list_changed, this, &MyMainWindow::handle_universe_list_changed);

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
	MainWindowAddUniverseWindow* const modal_window = new MainWindowAddUniverseWindow{ this, api_key, false };
	modal_window->show();
}

void MyMainWindow::pressed_edit_universe()
{
	MainWindowAddUniverseWindow* const modal_window = new MainWindowAddUniverseWindow{ this, api_key, true };
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
	const std::shared_ptr<const ApiKeyProfile> this_profile{ UserProfile::get_selected_api_key()};
	if (this_profile)
	{
		select_universe_combo->clear();
		for (size_t i = 0; i < this_profile->get_universe_list().size(); i++)
		{
			const std::shared_ptr<const UniverseProfile> this_universe = this_profile->get_universe_list().at(i);
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


MainWindowAddUniverseWindow::MainWindowAddUniverseWindow(QWidget* const parent, const QString& api_key, const bool edit_current) : QWidget{ parent, Qt::Window }, api_key{ api_key }
{
	setAttribute(Qt::WA_DeleteOnClose);

	OCTASSERT(parent != nullptr);
	setWindowModality(Qt::WindowModality::WindowModal);

	const std::shared_ptr<const UniverseProfile> existing_universe = edit_current ? UserProfile::get_selected_universe() : nullptr;
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

	QWidget* const button_panel = new QWidget{ this };
	button_panel->setMinimumWidth(280);
	{
		fetch_name_button = new QPushButton{ "Fetch Name", button_panel };
		connect(fetch_name_button, &QPushButton::clicked, this, &MainWindowAddUniverseWindow::pressed_fetch);

		add_button = new QPushButton{ "Add", button_panel };
		if (edit_mode)
		{
			add_button->setText("Update");
		}
		connect(add_button, &QPushButton::clicked, this, &MainWindowAddUniverseWindow::pressed_add);

		QPushButton* const cancel_button = new QPushButton{ "Cancel", button_panel };
		connect(cancel_button, &QPushButton::clicked, this, &QWidget::close);

		QHBoxLayout* const button_layout = new QHBoxLayout{ button_panel };
		button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		button_layout->addWidget(fetch_name_button);
		button_layout->addWidget(add_button);
		button_layout->addWidget(cancel_button);
	}

	QVBoxLayout* const layout = new QVBoxLayout{ this };
	layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
	layout->addWidget(info_panel);
	layout->addWidget(button_panel);

	text_changed();
}

bool MainWindowAddUniverseWindow::id_is_valid() const
{
	return id_edit->text().trimmed().toLongLong() > 100000L;
}

bool MainWindowAddUniverseWindow::name_is_valid() const
{
	return name_edit->text().size() > 0;
}

void MainWindowAddUniverseWindow::text_changed()
{
	fetch_name_button->setEnabled(id_is_valid());
	add_button->setEnabled(id_is_valid() && name_is_valid());
}

void MainWindowAddUniverseWindow::pressed_add()
{
	if (id_is_valid() && name_is_valid())
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
				msg_box->setText("Failed to update universe. A universe with that name or id already exists.");
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
				msg_box->setText("Failed to add new universe. A universe with that name or id already exists.");
				msg_box->exec();
			}
		}
	}
}

void MainWindowAddUniverseWindow::pressed_fetch()
{
	if (id_is_valid())
	{
		const long long universe_id = id_edit->text().trimmed().toLongLong();
		const auto req = std::make_shared<UniverseGetDetailsRequest>(api_key, universe_id);
		OperationInProgressDialog diag{ this, req };
		diag.exec();

		const std::optional<QString> display_name = req->get_display_name();
		if (display_name)
		{
			name_edit->setText(*display_name);
		}
	}
}
