#include "panel_universe_prefs.h"

#include <set>

#include <QGroupBox>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMargins>
#include <QPushButton>
#include <QVBoxLayout>

#include "user_settings.h"
#include "window_hidden_datastore_add.h"

UniversePreferencesPanel::UniversePreferencesPanel(QWidget* parent) : QWidget{ parent }
{
	connect(UserSettings::get().get(), &UserSettings::ignored_datastores_changed, this, &UniversePreferencesPanel::handle_ignored_datastores_changed);

	QWidget* container_widget = new QWidget{ this };
	{
		QGroupBox* hidden_datastore_group = new QGroupBox{ "Hidden Datastores", container_widget };
		{
			hidden_datastore_list = new QListWidget{ hidden_datastore_group };
			connect(hidden_datastore_list, &QListWidget::itemSelectionChanged, this, &UniversePreferencesPanel::handle_list_selection_changed);

			QWidget* button_widget = new QWidget{ hidden_datastore_group };
			{
				button_add = new QPushButton{ "Add...", button_widget };
				connect(button_add, &QPushButton::clicked, this, &UniversePreferencesPanel::pressed_add);

				button_remove = new QPushButton{ "Remove", button_widget };
				connect(button_remove, &QPushButton::clicked, this, &UniversePreferencesPanel::pressed_remove);

				QHBoxLayout* button_layout = new QHBoxLayout{ button_widget };
				button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
				button_layout->addWidget(button_add);
				button_layout->addWidget(button_remove);
			}

			QVBoxLayout* hidden_datastore_layout = new QVBoxLayout{ hidden_datastore_group };
			hidden_datastore_layout->addWidget(hidden_datastore_list);
			hidden_datastore_layout->addWidget(button_widget);
		}

		QHBoxLayout* container_layout = new QHBoxLayout{ container_widget };
		container_layout->addStretch();
		container_layout->addWidget(hidden_datastore_group);
		container_layout->addStretch();
	}

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->addWidget(container_widget);

	handle_ignored_datastores_changed();
	handle_list_selection_changed();
}

void UniversePreferencesPanel::selected_universe_changed()
{
	button_add->setEnabled(UserSettings::get()->get_selected_universe().has_value());
	handle_ignored_datastores_changed();
}

void UniversePreferencesPanel::handle_ignored_datastores_changed()
{
	while (hidden_datastore_list->count() > 0)
	{
		QListWidgetItem* this_item = hidden_datastore_list->takeItem(0);
		delete this_item;
	}

	if (std::optional<UniverseProfile> selected_universe = UserSettings::get()->get_selected_universe())
	{
		std::set<QString> datastore_names = selected_universe->ignored_datastores();
		for (const QString& this_datastore_name : datastore_names)
		{
			QListWidgetItem* this_item = new QListWidgetItem(hidden_datastore_list);
			this_item->setText(this_datastore_name);
			hidden_datastore_list->addItem(this_item);
		}
	}
}

void UniversePreferencesPanel::handle_list_selection_changed()
{
	QList<QListWidgetItem*> selected = hidden_datastore_list->selectedItems();
	button_remove->setEnabled(selected.size() == 1);
}

void UniversePreferencesPanel::pressed_add()
{
	if (std::optional<UniverseProfile> selected_universe = UserSettings::get()->get_selected_universe())
	{
		AddHiddenDatastoreWindow* hide_datastore_window = new AddHiddenDatastoreWindow{ this };
		hide_datastore_window->show();
	}
}

void UniversePreferencesPanel::pressed_remove()
{
	QList<QListWidgetItem*> selected = hidden_datastore_list->selectedItems();
	if (selected.size() == 1)
	{
		QString to_remove = selected.front()->text();
		UserSettings::get()->remove_ignored_datastore(to_remove);
	}
}
