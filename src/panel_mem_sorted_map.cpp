#include "panel_mem_sorted_map.h"

#include <cstddef>

#include <memory>
#include <set>
#include <vector>

#include <Qt>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMargins>
#include <QPushButton>
#include <QSizePolicy>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

#include "assert.h"
#include "data_request.h"
#include "diag_operation_in_progress.h"
#include "gui_constants.h"
#include "model_common.h"
#include "model_qt.h"
#include "profile.h"

MemoryStoreSortedMapPanel::MemoryStoreSortedMapPanel(QWidget* const parent, const QString& api_key) :
	QWidget{ parent },
	api_key{ api_key }
{
	QSplitter* const splitter = new QSplitter{ this };
	splitter->setChildrenCollapsible(false);
	{
		QWidget* const panel_index = new QWidget{ splitter };
		{
			QGroupBox* const group_box = new QGroupBox{ "Sorted Maps", panel_index };
			{
				list_maps = new QListWidget{ group_box };
				connect(list_maps, &QListWidget::itemSelectionChanged, this, &MemoryStoreSortedMapPanel::handle_selected_map_changed);

				check_save_recent_maps = new QCheckBox{ "Add used maps", group_box };
				connect(check_save_recent_maps, &QCheckBox::stateChanged, this, &MemoryStoreSortedMapPanel::handle_save_recent_maps_toggled);

				button_remove_recent_map = new QPushButton{ "Remove", group_box };
				connect(button_remove_recent_map, &QPushButton::clicked, this, &MemoryStoreSortedMapPanel::pressed_remove_recent_map);

				QVBoxLayout* const group_layout = new QVBoxLayout{ group_box };
				group_layout->addWidget(list_maps);
				group_layout->addWidget(check_save_recent_maps);
				group_layout->addWidget(button_remove_recent_map);
			}


			QVBoxLayout* const layout_index = new QVBoxLayout{ panel_index };
			layout_index->setContentsMargins(QMargins{ 0, 0, 0, 0 });
			layout_index->addWidget(group_box);
		}

		QWidget* const panel_main = new QWidget{ splitter };
		{
			QGroupBox* const group_box = new QGroupBox{ "Find Sorted Map Items", panel_main };
			{
				QWidget* const panel_map_name = new QWidget{ group_box };
				{
					QLabel* const label_map_name = new QLabel{ "Map Name:", panel_map_name };
					edit_map_name = new QLineEdit{ panel_map_name };
					connect(edit_map_name, &QLineEdit::textChanged, this, &MemoryStoreSortedMapPanel::handle_search_name_changed);

					QHBoxLayout* const layout_map_name = new QHBoxLayout{ panel_map_name };
					layout_map_name->setContentsMargins(QMargins{ 0, 0, 0, 0 });
					layout_map_name->addWidget(label_map_name);
					layout_map_name->addWidget(edit_map_name);
				}

				QWidget* const panel_filter = new QWidget{ group_box };
				{
					QLabel* const label_filter = new QLabel{ "Filter:", panel_filter };
					QLineEdit* const edit_filter = new QLineEdit{ panel_filter };

					QHBoxLayout* const layout_search_params = new QHBoxLayout{ panel_filter };
					layout_search_params->setContentsMargins(QMargins{ 0, 0, 0, 0 });
					layout_search_params->addWidget(label_filter);
					layout_search_params->addWidget(edit_filter);
				}

				QWidget* const panel_find_buttons = new QWidget{ group_box };
				{
					button_list_all_asc = new QPushButton{ "List all (ascending)" };
					connect(button_list_all_asc, &QPushButton::clicked, this, &MemoryStoreSortedMapPanel::pressed_list_all_asc);

					button_list_all_desc = new QPushButton{ "List all (descending)" };
					connect(button_list_all_desc, &QPushButton::clicked, this, &MemoryStoreSortedMapPanel::pressed_list_all_desc);

					QLabel* const label_list_limit = new QLabel{ "Limit:", panel_find_buttons };
					label_list_limit->setSizePolicy(QSizePolicy{ QSizePolicy::Fixed, QSizePolicy::Fixed });

					edit_list_limit = new QLineEdit{ panel_find_buttons };
					edit_list_limit->setText("1200");
					edit_list_limit->setFixedWidth(60);

					QHBoxLayout* const layout_find_buttons = new QHBoxLayout{ panel_find_buttons };
					layout_find_buttons->setContentsMargins(QMargins{ 0, 0, 0, 0 });
					layout_find_buttons->addWidget(button_list_all_asc);
					layout_find_buttons->addWidget(button_list_all_desc);
					layout_find_buttons->addWidget(label_list_limit);
					layout_find_buttons->addWidget(edit_list_limit);
				}

				tree_view = new QTreeView{ group_box };
				tree_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
				tree_view->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

				QVBoxLayout* const group_layout = new QVBoxLayout{ group_box };
				group_layout->addWidget(panel_map_name);
				group_layout->addWidget(panel_filter);
				group_layout->addWidget(panel_find_buttons);
				group_layout->addWidget(tree_view);
			}

			QVBoxLayout* const layout_main = new QVBoxLayout{ panel_main };
			layout_main->setContentsMargins(QMargins{ 0, 0, 0, 0 });
			layout_main->addWidget(group_box);
		}
		splitter->addWidget(panel_index);
		splitter->addWidget(panel_main);
		splitter->setSizes({ OCT_LIST_WIDGET_LIST_WIDTH, OCT_LIST_WIDGET_MAIN_WIDTH });
	}

	QHBoxLayout* const layout = new QHBoxLayout{ this };
	layout->addWidget(splitter);

	set_table_model(nullptr);
	change_universe(nullptr);
}

void MemoryStoreSortedMapPanel::change_universe(const std::shared_ptr<UniverseProfile>& universe)
{
	attached_universe = universe;

	QObject::disconnect(conn_universe_mem_sorted_map_list_changed);
	if (universe)
	{
		conn_universe_mem_sorted_map_list_changed = connect(universe.get(), &UniverseProfile::recent_mem_sorted_map_list_changed, this, &MemoryStoreSortedMapPanel::handle_recent_maps_changed);
	}

	list_maps->clear();
	edit_map_name->setText("");

	const bool enabled = static_cast<bool>(universe);
	check_save_recent_maps->setEnabled(enabled);
	if (universe)
	{
		check_save_recent_maps->setChecked(universe->get_save_recent_mem_sorted_maps());
	}
	else
	{
		check_save_recent_maps->setChecked(false);
	}

	set_table_model(nullptr);
	handle_recent_maps_changed();
	gui_refresh();
}

void MemoryStoreSortedMapPanel::gui_refresh()
{
	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		setEnabled(false);
		return;
	}

	setEnabled(true);

	const bool list_enabled = edit_map_name->text().size() > 0;
	button_list_all_asc->setEnabled(list_enabled);
	button_list_all_desc->setEnabled(list_enabled);

	const QList<QListWidgetItem*> selected = list_maps->selectedItems();
	button_remove_recent_map->setEnabled(selected.size() == 1);
}

void MemoryStoreSortedMapPanel::set_table_model(MemoryStoreSortedMapQTableModel* const table_model)
{
	if (table_model)
	{
		tree_view->setModel(table_model);
	}
	else
	{
		tree_view->setModel(new MemoryStoreSortedMapQTableModel{ tree_view, std::vector<MemoryStoreSortedMapItem>{} });
	}
}

void MemoryStoreSortedMapPanel::handle_recent_maps_changed()
{
	const std::shared_ptr<const UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		return;
	}

	list_maps->clear();
	for (const QString& this_map : universe->get_recent_mem_sorted_map_set())
	{
		list_maps->addItem(this_map);
	}
}

void MemoryStoreSortedMapPanel::handle_save_recent_maps_toggled()
{
	if (const std::shared_ptr<UniverseProfile> universe = attached_universe.lock())
	{
		universe->set_save_recent_mem_sorted_maps(check_save_recent_maps->isChecked());
	}
}

void MemoryStoreSortedMapPanel::handle_search_name_changed()
{
	gui_refresh();
}

void MemoryStoreSortedMapPanel::handle_selected_map_changed()
{
	const QList<QListWidgetItem*> selected = list_maps->selectedItems();
	if (selected.size() == 1)
	{
		edit_map_name->setText(selected.first()->text());
	}
}

void MemoryStoreSortedMapPanel::pressed_list_all(const bool ascending)
{
	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		return;
	}

	if (edit_map_name->text().trimmed().size() == 0)
	{
		return;
	}

	const long long universe_id = universe->get_universe_id();
	const QString map_name = edit_map_name->text().trimmed();
	const size_t result_limit = edit_list_limit->text().trimmed().toULongLong();

	const auto req = std::make_shared<MemoryStoreSortedMapGetListRequest>(api_key, universe_id, map_name, ascending);
	if (result_limit > 0)
	{
		req->set_result_limit(result_limit);
	}

	OperationInProgressDialog diag{ this, req };
	diag.exec();

	if (check_save_recent_maps->isChecked() && req->get_items().size() > 0)
	{
		universe->add_recent_mem_sorted_map(map_name);
	}

	MemoryStoreSortedMapQTableModel* const model = new MemoryStoreSortedMapQTableModel{ tree_view, req->get_items() };
	set_table_model(model);
}

void MemoryStoreSortedMapPanel::pressed_remove_recent_map()
{
	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (universe)
	{
		return;
	}

	const QList<QListWidgetItem*> selected = list_maps->selectedItems();
	if (selected.size() != 1)
	{
		return;
	}

	universe->remove_recent_mem_sorted_map(selected.front()->text());
}
