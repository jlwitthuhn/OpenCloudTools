#include "window_api_key_manage.h"

#include <map>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include <Qt>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QMargins>
#include <QMetaType>
#include <QPushButton>
#include <QVariant>
#include <QVBoxLayout>

#include "api_key.h"
#include "user_settings.h"
#include "window_api_key_add.h"
#include "window_main.h"

ManageApiKeysWindow::ManageApiKeysWindow(QWidget* parent) : QWidget{ parent, Qt::Window }
{
	setWindowTitle("API Keys");

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->setAlignment(Qt::AlignHCenter);

	QLabel* title_label = new QLabel{ this };
	title_label->setText("Manage Roblox API Keys");
	layout->addWidget(title_label);

	list_widget = new QListWidget{ this };
	layout->addWidget(list_widget);

	connect(list_widget, &QListWidget::itemSelectionChanged, this, &ManageApiKeysWindow::selection_changed);

	QWidget* button_panel_top = new QWidget{ this };
	{
		QHBoxLayout* button_layout = new QHBoxLayout{ button_panel_top };
		button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });

		QPushButton* add_button = new QPushButton{ "Add...", button_panel_top };
		button_layout->addWidget(add_button);
		connect(add_button, &QPushButton::clicked, this, &ManageApiKeysWindow::click_add);

		mod_button = new QPushButton{ "Edit...", button_panel_top };
		mod_button->setEnabled(false);
		button_layout->addWidget(mod_button);
		connect(mod_button, &QPushButton::clicked, this, &ManageApiKeysWindow::click_ed);

		del_button = new QPushButton{ "Delete", button_panel_top };
		del_button->setEnabled(false);
		button_layout->addWidget(del_button);
		connect(del_button, &QPushButton::clicked, this, &ManageApiKeysWindow::click_del);
	}
	layout->addWidget(button_panel_top);

	QWidget* button_panel_bot = new QWidget{ this };
	{
		QHBoxLayout* button_layout = new QHBoxLayout{ button_panel_bot };
		button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });

		sel_button = new QPushButton{ "Select", button_panel_bot };
		sel_button->setEnabled(false);
		button_layout->addWidget(sel_button);;
		connect(sel_button, &QPushButton::clicked, this, &ManageApiKeysWindow::click_sel);

		QPushButton* close_button = new QPushButton{ "Close", button_panel_bot };
		button_layout->addWidget(close_button);
		connect(close_button, &QPushButton::clicked, this, &QWidget::close);
	}
	layout->addWidget(button_panel_bot);

	connect(UserSettings::get().get(), &UserSettings::api_key_list_changed, this, &ManageApiKeysWindow::rebuild_slots);

	rebuild_slots();
	selection_changed();
}

ManageApiKeysWindow::~ManageApiKeysWindow()
{

}

void ManageApiKeysWindow::click_add()
{
	AddApiKeyWindow* add_key_window = new AddApiKeyWindow{ this };
	add_key_window->setWindowModality(Qt::WindowModality::ApplicationModal);
	add_key_window->show();
}

void ManageApiKeysWindow::click_ed()
{
	QList<QListWidgetItem*> selected = list_widget->selectedItems();
	if (selected.size() == 1)
	{
		QVariant data = selected.first()->data(Qt::UserRole);
		if (data.metaType().id() == QMetaType::UInt)
		{
			unsigned int key_id =  data.toUInt();
			std::optional<ApiKeyProfile> opt_details = UserSettings::get()->get_api_key(key_id);
			if (opt_details)
			{
				AddApiKeyWindow* add_key_window = new AddApiKeyWindow{ this, std::make_pair(key_id, *opt_details) };
				add_key_window->setWindowModality(Qt::WindowModality::ApplicationModal);
				add_key_window->show();
			}
		}
	}
}

void ManageApiKeysWindow::click_del()
{
	QList<QListWidgetItem*> selected = list_widget->selectedItems();
	if (selected.size() == 1)
	{
		QVariant data = selected.first()->data(Qt::UserRole);
		if (data.metaType().id() == QMetaType::UInt)
		{
			UserSettings::get()->delete_api_key(data.toUInt());
		}
	}
}

void ManageApiKeysWindow::click_sel()
{
	QList<QListWidgetItem*> selected = list_widget->selectedItems();
	if (selected.size() == 1)
	{
		QVariant data = selected.first()->data(Qt::UserRole);
		if (data.metaType().id() == QMetaType::UInt)
		{
			UserSettings::get()->select_api_key(data.toUInt());
			std::optional<ApiKeyProfile> details = UserSettings::get()->get_selected_profile();
			if (details)
			{
				MyMainWindow* main_window = new MyMainWindow{ nullptr, details->name(), details->key() };
				main_window->show();
				close();
			}
		}
	}
}

void ManageApiKeysWindow::rebuild_slots()
{
	while (list_widget->count() > 0)
	{
		QListWidgetItem* this_item = list_widget->takeItem(0);
		delete this_item;
	}

	std::map<unsigned int, ApiKeyProfile> keys = UserSettings::get()->get_all_api_keys();
	for (std::pair<unsigned int, ApiKeyProfile> this_pair : keys)
	{
		QListWidgetItem* this_item = new QListWidgetItem(list_widget);
		this_item->setData(Qt::UserRole, this_pair.first);
		this_item->setText(this_pair.second.name());
		list_widget->addItem(this_item);
	}
}

void ManageApiKeysWindow::selection_changed()
{
	QList<QListWidgetItem*> selected = list_widget->selectedItems();
	if (selected.size() == 1)
	{
		mod_button->setEnabled(true);
		del_button->setEnabled(true);
		sel_button->setEnabled(true);
	}
	else
	{
		mod_button->setEnabled(false);
		del_button->setEnabled(false);
		sel_button->setEnabled(false);
	}
}
