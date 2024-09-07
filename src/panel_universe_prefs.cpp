#include "panel_universe_prefs.h"

#include <memory>
#include <set>

#include <Qt>
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

#include "assert.h"
#include "profile.h"

UniversePreferencesPanel::UniversePreferencesPanel(QWidget* parent) : QWidget{ parent }
{
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

void UniversePreferencesPanel::change_universe(const std::shared_ptr<UniverseProfile>& universe)
{
	attached_universe = universe;

	QObject::disconnect(conn_universe_hidden_datastores_changed);
	if (universe)
	{
		conn_universe_hidden_datastores_changed = connect(universe.get(), &UniverseProfile::hidden_datastore_list_changed, this, &UniversePreferencesPanel::handle_hidden_datastores_changed);
	}

	handle_hidden_datastores_changed();
	gui_refresh();
}

void UniversePreferencesPanel::gui_refresh()
{
	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		setEnabled(false);
		return;
	}

	setEnabled(true);

	const QList<QListWidgetItem*> selected = hidden_datastore_list->selectedItems();
	button_remove->setEnabled(selected.size() == 1);
}

void UniversePreferencesPanel::handle_hidden_datastores_changed()
{
	hidden_datastore_list->clear();

	if (const std::shared_ptr<const UniverseProfile> universe = attached_universe.lock())
	{
		std::set<QString> datastore_names = universe->get_hidden_datastore_set();
		for (const QString& this_datastore_name : datastore_names)
		{
			QListWidgetItem* this_item = new QListWidgetItem(hidden_datastore_list);
			this_item->setText(this_datastore_name);
			hidden_datastore_list->addItem(this_item);
		}
	}

	gui_refresh();
}

void UniversePreferencesPanel::handle_list_selection_changed()
{
	gui_refresh();
}

void UniversePreferencesPanel::pressed_add()
{
	if (const std::shared_ptr<UniverseProfile> universe = attached_universe.lock())
	{
		UniversePreferencesAddHiddenDatastoreWindow* hide_datastore_window = new UniversePreferencesAddHiddenDatastoreWindow{ this, universe };
		hide_datastore_window->show();
	}
}

void UniversePreferencesPanel::pressed_remove()
{
	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		return;
	}

	QList<QListWidgetItem*> selected = hidden_datastore_list->selectedItems();
	if (selected.size() != 1)
	{
		return;
	}

	QString to_remove = selected.front()->text();
	universe->remove_hidden_datastore(to_remove);
}

UniversePreferencesAddHiddenDatastoreWindow::UniversePreferencesAddHiddenDatastoreWindow(QWidget* parent, const std::shared_ptr<UniverseProfile>& universe) :
	QWidget{ parent, Qt::Window },
	attached_universe{ universe }
{
	setWindowTitle("Hide datastore");
	setAttribute(Qt::WA_DeleteOnClose);

	OCTASSERT(parent != nullptr);
	setWindowModality(Qt::WindowModality::WindowModal);

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
	if (const std::shared_ptr<UniverseProfile> universe = attached_universe.lock())
	{
		universe->add_hidden_datastore(name_edit->text());;
	}
	close();
}
