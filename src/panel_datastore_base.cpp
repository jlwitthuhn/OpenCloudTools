#include "panel_datastore_base.h"

#include <cstddef>

#include <algorithm>
#include <memory>
#include <optional>
#include <set>
#include <vector>

#include <Qt>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QClipboard>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMargins>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSizePolicy>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QTreeView>
#include <QVariant>
#include <QVBoxLayout>

#include "data_request.h"
#include "diag_confirm_change.h"
#include "diag_operation_in_progress.h"
#include "model_api_opencloud.h"
#include "model_qt.h"
#include "profile.h"
#include "util_enum.h"
#include "util_json.h"
#include "util_validator.h"
#include "window_datastore_entry_versions_view.h"
#include "window_datastore_entry_view.h"

BaseDatastorePanel::BaseDatastorePanel(QWidget* parent, const QString& api_key) :
	QWidget{ parent },
	api_key{ api_key }
{
	connect(UserProfile::get().get(), &UserProfile::hidden_datastore_list_changed, this, &BaseDatastorePanel::refresh_datastore_list);
	connect(UserProfile::get().get(), &UserProfile::show_datastore_filter_changed, this, &BaseDatastorePanel::handle_show_datastore_filter_changed);

	panel_left = new QWidget{ this };
	{
		QSizePolicy left_size_policy{ QSizePolicy::Preferred, QSizePolicy::Preferred };
		left_size_policy.setHorizontalStretch(5);
		panel_left->setSizePolicy(left_size_policy);

		select_datastore_group = new QGroupBox{ "Datastores", panel_left };
		{
			select_datastore_list = new QListWidget{ select_datastore_group };
			select_datastore_list->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
			connect(select_datastore_list, &QListWidget::customContextMenuRequested, this, &BaseDatastorePanel::pressed_right_click_datastore_list);
			connect(select_datastore_list, &QListWidget::itemSelectionChanged, this, &BaseDatastorePanel::handle_selected_datastore_changed);

			select_datastore_name_filter_edit = new QLineEdit{ select_datastore_group };
			select_datastore_name_filter_edit->setToolTip("Only datastore names matching this text box will be displayed.");
			connect(select_datastore_name_filter_edit, &QLineEdit::textChanged, this, &BaseDatastorePanel::refresh_datastore_list);

			QVBoxLayout* select_datastore_layout = new QVBoxLayout{ select_datastore_group };
			select_datastore_layout->addWidget(select_datastore_list);
			select_datastore_layout->addWidget(select_datastore_name_filter_edit);
		}

		QVBoxLayout* left_bar_layout = new QVBoxLayout{ panel_left };
		left_bar_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		left_bar_layout->addWidget(select_datastore_group);
	}

	QWidget* main_widget = new QWidget{ this };
	{
		QSizePolicy main_size_policy{ QSizePolicy::Preferred, QSizePolicy::Preferred };
		main_size_policy.setHorizontalStretch(7);
		main_widget->setSizePolicy(main_size_policy);

		main_tab_widget = new QTabWidget{ main_widget };
		{
			search_panel = new QWidget{ main_tab_widget };
			{
				search_params_widget = new QWidget{ search_panel };
				{
					QLabel* search_datastore_name_label = new QLabel{ "Datastore:", search_params_widget };

					search_datastore_name_edit = new QLineEdit{ search_params_widget };
					connect(search_datastore_name_edit, &QLineEdit::textChanged, this, &BaseDatastorePanel::handle_search_text_changed);

					QLabel* search_datastore_scope_label = new QLabel{ "Scope:", search_params_widget };

					search_datastore_scope_edit = new QLineEdit{ search_params_widget };
					search_datastore_scope_edit->setPlaceholderText("global");
					connect(search_datastore_scope_edit, &QLineEdit::textChanged, this, &BaseDatastorePanel::handle_search_text_changed);

					QHBoxLayout* const main_top_layout = new QHBoxLayout{ search_params_widget };
					main_top_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
					main_top_layout->addWidget(search_datastore_name_label);
					main_top_layout->addWidget(search_datastore_name_edit);
					main_top_layout->addWidget(search_datastore_scope_label);
					main_top_layout->addWidget(search_datastore_scope_edit);
				}

				search_submit_widget = new QWidget{ search_panel };
				{
					QLabel* find_limit_label = new QLabel{ "Limit:", search_submit_widget };
					find_limit_label->setSizePolicy(QSizePolicy{ QSizePolicy::Fixed, QSizePolicy::Fixed });

					find_limit_edit = new QLineEdit{ search_submit_widget };
					find_limit_edit->setText("1200");
					find_limit_edit->setFixedWidth(60);

					QHBoxLayout* search_submit_layout = new QHBoxLayout{ search_submit_widget };
					search_submit_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
					search_submit_layout->addWidget(find_limit_label);
					search_submit_layout->addWidget(find_limit_edit);
				}

				datastore_entry_tree = new QTreeView{ search_panel };
				datastore_entry_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
				datastore_entry_tree->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
				connect(datastore_entry_tree, &QListWidget::customContextMenuRequested, this, &BaseDatastorePanel::pressed_right_click_entry_list);
				connect(datastore_entry_tree, &QTreeView::doubleClicked, this, &BaseDatastorePanel::handle_datastore_entry_double_clicked);

				QVBoxLayout* right_box_layout = new QVBoxLayout{ search_panel };
				right_box_layout->addWidget(search_params_widget);
				right_box_layout->addWidget(search_submit_widget);
				right_box_layout->addWidget(datastore_entry_tree);
			}

			main_tab_widget->addTab(search_panel, "Search");
		}
		QVBoxLayout* const main_layout = new QVBoxLayout{ main_widget };
		main_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		main_layout->addWidget(main_tab_widget);
	}

	QSplitter* splitter = new QSplitter{ this };
	splitter->setChildrenCollapsible(false);
	splitter->addWidget(panel_left);
	splitter->addWidget(main_widget);

	QHBoxLayout* layout = new QHBoxLayout{ this };
	layout->addWidget(splitter);

	handle_search_text_changed();
	handle_selected_datastore_changed();
	handle_selected_datastore_entry_changed();
	handle_show_datastore_filter_changed();
}

void BaseDatastorePanel::selected_universe_changed()
{
	select_datastore_list->clear();
	handle_search_text_changed();
	clear_model();
}

QModelIndex BaseDatastorePanel::get_selected_single_index() const
{
	if (const QItemSelectionModel* select_model = datastore_entry_tree->selectionModel())
	{
		if (select_model->selectedRows().count() == 1)
		{
			return select_model->selectedRows().front();
		}
	}

	return QModelIndex{};
}

void BaseDatastorePanel::handle_selected_datastore_changed()
{
	QList<QListWidgetItem*> selected = select_datastore_list->selectedItems();
	if (selected.size() == 1)
	{
		search_datastore_name_edit->setText(selected.first()->text());
	}
}

void BaseDatastorePanel::handle_show_datastore_filter_changed()
{
	const bool active = UserProfile::get()->get_show_datastore_name_filter();
	if (!active)
	{
		select_datastore_name_filter_edit->setText("");
	}
	select_datastore_name_filter_edit->setVisible(active);
}
