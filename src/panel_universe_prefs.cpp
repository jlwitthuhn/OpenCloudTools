#include "panel_universe_prefs.h"

#include <memory>
#include <optional>
#include <set>

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMargins>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include "api_key.h"
#include "user_settings.h"

UniversePreferencesPanel::UniversePreferencesPanel(QWidget* parent) : QWidget{ parent }
{
	connect(UserSettings::get().get(), &UserSettings::hidden_datastores_changed, this, &UniversePreferencesPanel::handle_hidden_datastores_changed);

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

	handle_hidden_datastores_changed();
	handle_list_selection_changed();
}

void UniversePreferencesPanel::selected_universe_changed()
{
	button_add->setEnabled(UserSettings::get()->get_selected_universe().has_value());
	handle_hidden_datastores_changed();
}

void UniversePreferencesPanel::handle_hidden_datastores_changed()
{
	while (hidden_datastore_list->count() > 0)
	{
		QListWidgetItem* this_item = hidden_datastore_list->takeItem(0);
		delete this_item;
	}

	if (std::optional<UniverseProfile> selected_universe = UserSettings::get()->get_selected_universe())
	{
		std::set<QString> datastore_names = selected_universe->hidden_datastores();
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
		UniversePreferencesAddHiddenDatastoreWindow* hide_datastore_window = new UniversePreferencesAddHiddenDatastoreWindow{ this };
		hide_datastore_window->show();
	}
}

void UniversePreferencesPanel::pressed_remove()
{
	QList<QListWidgetItem*> selected = hidden_datastore_list->selectedItems();
	if (selected.size() == 1)
	{
		QString to_remove = selected.front()->text();
		UserSettings::get()->remove_hidden_datastore(to_remove);
	}
}

UniversePreferencesAddHiddenDatastoreWindow::UniversePreferencesAddHiddenDatastoreWindow(QWidget* parent) : QWidget{ parent, Qt::Window }
{
	setAttribute(Qt::WA_DeleteOnClose);

	setWindowTitle("Hide datastore");
	setWindowModality(Qt::WindowModality::ApplicationModal);

	QWidget* info_panel = new QWidget{ this };
	{
		name_edit = new QLineEdit{ info_panel };
		connect(name_edit, &QLineEdit::textChanged, this, &UniversePreferencesAddHiddenDatastoreWindow::handle_text_changed);

		QFormLayout* info_layout = new QFormLayout{ info_panel };
		info_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		info_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		info_layout->addRow("Datastore name", name_edit);
	}

	QWidget* button_panel = new QWidget{ this };
	button_panel->setMinimumWidth(280);
	{
		add_button = new QPushButton{ "Add", button_panel };
		connect(add_button, &QPushButton::clicked, this, &UniversePreferencesAddHiddenDatastoreWindow::pressed_add);

		QPushButton* cancel_button = new QPushButton{ "Cancel", button_panel };
		connect(add_button, &QPushButton::clicked, this, &UniversePreferencesAddHiddenDatastoreWindow::close);

		QHBoxLayout* button_layout = new QHBoxLayout{ button_panel };
		button_layout->addWidget(add_button);
		button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		button_layout->addWidget(cancel_button);
		connect(cancel_button, &QPushButton::clicked, this, &QWidget::close);
	}

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->addWidget(info_panel);
	layout->addWidget(button_panel);

	handle_text_changed();
}

void UniversePreferencesAddHiddenDatastoreWindow::handle_text_changed()
{
	add_button->setEnabled(name_edit->text().size() > 0);
}

void UniversePreferencesAddHiddenDatastoreWindow::pressed_add()
{
	UserSettings::get()->add_hidden_datastore(name_edit->text());
	close();
}
