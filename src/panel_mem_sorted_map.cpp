#include "panel_mem_sorted_map.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSplitter>
#include <QTreeView>

#include "gui_constants.h"

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
				QListWidget* const store_list = new QListWidget{ group_box };

				QVBoxLayout* const group_layout = new QVBoxLayout{ group_box };
				group_layout->addWidget(store_list);
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
					QLineEdit* const edit_map_name = new QLineEdit{ panel_map_name };

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
					QPushButton* const button_list_all_asc = new QPushButton{ "List all (ascending)" };

					QPushButton* const button_list_all_desc = new QPushButton{ "List all (descending)" };

					QHBoxLayout* const layout_find_buttons = new QHBoxLayout{ panel_find_buttons };
					layout_find_buttons->setContentsMargins(QMargins{ 0, 0, 0, 0 });
					layout_find_buttons->addWidget(button_list_all_asc);
					layout_find_buttons->addWidget(button_list_all_desc);
				}

				QTreeView* const tree_view = new QTreeView{ group_box };
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
}

void MemoryStoreSortedMapPanel::selected_universe_changed()
{

}
