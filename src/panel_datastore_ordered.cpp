#include "panel_datastore_ordered.h"

#include <cstddef>

#include <memory>
#include <set>
#include <vector>

#include <QBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QLayout>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QTreeView>
#include <QWidget>

#include "data_request.h"
#include "diag_operation_in_progress.h"
#include "model_common.h"
#include "model_qt.h"
#include "profile.h"
#include "window_ordered_datastore_entry_view.h"

OrderedDatastorePanel::OrderedDatastorePanel(QWidget* parent, const QString& api_key) : BaseDatastorePanel(parent, api_key)
{
	connect(UserProfile::get().get(), &UserProfile::recent_ordered_datastore_list_changed, this, &OrderedDatastorePanel::handle_recent_datastores_changed);

	// Left bar
	{
		save_recent_datastores_check = new QCheckBox{ "Add used datastores", select_datastore_group };
		connect(save_recent_datastores_check, &QCheckBox::stateChanged, this, &OrderedDatastorePanel::handle_save_recent_datastores_toggled);

		remove_datastore_button = new QPushButton{ "Remove", select_datastore_group };
		connect(remove_datastore_button, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_remove_datastore);

		QLayout* group_layout = select_datastore_group->layout();
		group_layout->addWidget(save_recent_datastores_check);
		group_layout->addWidget(remove_datastore_button);
	}

	// Main panel
	{
		find_ascending_button = new QPushButton{ "Find all (ascending)", search_submit_widget };
		connect(find_ascending_button, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_find_ascending);

		find_descending_button = new QPushButton{ "Find all (descending)", search_submit_widget };
		connect(find_descending_button, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_find_descending);

		QBoxLayout* const search_submit_layout = dynamic_cast<QBoxLayout*>(search_submit_widget->layout());
		search_submit_layout->insertWidget(0, find_ascending_button);
		search_submit_layout->insertWidget(1, find_descending_button);

		// Bottom buttons
		{
			QWidget* const read_button_panel = new QWidget{ search_panel };
			{
				view_entry_button = new QPushButton{ "View entry...", read_button_panel };
				connect(view_entry_button, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_view_entry);

				QHBoxLayout* const right_read_layout = new QHBoxLayout{ read_button_panel };
				right_read_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
				right_read_layout->addWidget(view_entry_button);
			}

			QFrame* const separator = new QFrame{ search_panel };
			separator->setFrameShape(QFrame::HLine);
			separator->setFrameShadow(QFrame::Sunken);

			QWidget* const edit_button_panel = new QWidget{ search_panel };
			{
				increment_entry_button = new QPushButton{ "Increment entry", edit_button_panel };
				increment_entry_button->setEnabled(false);

				edit_entry_button = new QPushButton{ "Edit entry...", edit_button_panel };
				edit_entry_button->setEnabled(false);

				QHBoxLayout* const right_edit_layout = new QHBoxLayout{ edit_button_panel };
				right_edit_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
				right_edit_layout->addWidget(increment_entry_button);
				right_edit_layout->addWidget(edit_entry_button);
			}

			QLayout* const search_layout = search_panel->layout();
			search_layout->addWidget(read_button_panel);
			search_layout->addWidget(separator);
			search_layout->addWidget(edit_button_panel);
		}
	}

	clear_model();
	selected_universe_changed();
	handle_search_text_changed();
}

void OrderedDatastorePanel::selected_universe_changed()
{
	BaseDatastorePanel::selected_universe_changed();

	const UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
	const bool enabled = selected_universe != nullptr;
	save_recent_datastores_check->setEnabled(enabled);
	if (enabled)
	{
		save_recent_datastores_check->setChecked(selected_universe->get_save_recent_ordered_datastores());
	}
	else
	{
		save_recent_datastores_check->setChecked(false);
	}
	handle_recent_datastores_changed();
}

void OrderedDatastorePanel::set_datastore_entry_model(OrderedDatastoreEntryQTableModel* entry_model)
{
	if (entry_model)
	{
		datastore_entry_tree->setModel(entry_model);
	}
	else
	{
		datastore_entry_tree->setModel(new OrderedDatastoreEntryQTableModel{ datastore_entry_tree, std::vector<OrderedDatastoreEntryFull>{} });
	}
	datastore_entry_tree->setColumnWidth(0, 140);
	datastore_entry_tree->setColumnWidth(1, 140);
	connect(datastore_entry_tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &OrderedDatastorePanel::handle_selected_datastore_entry_changed);
	handle_selected_datastore_entry_changed();
}

void OrderedDatastorePanel::view_entry(const QModelIndex& index)
{
	if (index.isValid())
	{
		if (OrderedDatastoreEntryQTableModel* const model = dynamic_cast<OrderedDatastoreEntryQTableModel*>(datastore_entry_tree->model()))
		{
			std::optional<OrderedDatastoreEntryFull> opt_entry = model->get_entry(index.row());
			if (opt_entry)
			{
				const auto req = std::make_shared<GetOrderedDatastoreEntryDetailsRequest>(
					api_key,
					opt_entry->get_universe_id(),
					opt_entry->get_datastore_name(),
					opt_entry->get_scope(),
					opt_entry->get_key_name()
				);

				OperationInProgressDialog diag{ this, req };
				diag.exec();

				const std::optional<OrderedDatastoreEntryFull> opt_details = req->get_details();
				if (opt_details)
				{
					ViewOrderedDatastoreEntryWindow* const view_entry_window = new ViewOrderedDatastoreEntryWindow{ this, api_key, *opt_details };
					view_entry_window->setWindowModality(Qt::WindowModality::ApplicationModal);
					view_entry_window->show();
				}
				else
				{
					QMessageBox* const msg_box = new QMessageBox{ this };
					msg_box->setWindowTitle("Not Found");
					msg_box->setText("This entry does not exist or has been deleted.");
					msg_box->exec();
				}
			}
		}
	}
}

void OrderedDatastorePanel::handle_datastore_entry_double_clicked(const QModelIndex& index)
{
	view_entry(index);
}

void OrderedDatastorePanel::handle_search_text_changed()
{
	BaseDatastorePanel::handle_search_text_changed();
	const UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
	const bool enabled = search_datastore_name_edit->text().size() > 0 && selected_universe;
	find_ascending_button->setEnabled(enabled);
	find_descending_button->setEnabled(enabled);
}

void OrderedDatastorePanel::handle_selected_datastore_entry_changed()
{
	size_t count = 0;
	bool single_selected = false;
	if (const QItemSelectionModel* const select_model = datastore_entry_tree->selectionModel())
	{
		count = select_model->selectedRows().count();
		single_selected = count == 1;
	}
	view_entry_button->setEnabled(single_selected);
}

void OrderedDatastorePanel::handle_recent_datastores_changed()
{
	select_datastore_list->clear();
	if (const UniverseProfile* const selected_universe = UserProfile::get_selected_universe())
	{
		for (const QString& this_topic : selected_universe->get_recent_ordered_datastore_set())
		{
			select_datastore_list->addItem(this_topic);
		}
	}
}

void OrderedDatastorePanel::handle_save_recent_datastores_toggled()
{
	if (UniverseProfile* const selected_universe = UserProfile::get_selected_universe())
	{
		selected_universe->set_save_recent_ordered_datastores(save_recent_datastores_check->isChecked());
	}
}

void OrderedDatastorePanel::clear_model()
{
	set_datastore_entry_model(nullptr);
}

void OrderedDatastorePanel::refresh_datastore_list()
{
	const UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
	if (selected_universe)
	{
		for (int i = 0; i < select_datastore_list->count(); i++)
		{
			QListWidgetItem* const this_item = select_datastore_list->item(i);
			const bool matches_filter = this_item->text().contains(select_datastore_name_filter_edit->text());
			this_item->setHidden(!matches_filter);
		}
	}
}

void OrderedDatastorePanel::pressed_find(const bool ascending)
{
	UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
	if (selected_universe && search_datastore_name_edit->text().trimmed().size() > 0)
	{
		const long long universe_id = selected_universe->get_universe_id();
		if (universe_id > 0)
		{
			QString datastore_name = search_datastore_name_edit->text().trimmed();
			QString scope = search_datastore_scope_edit->text().trimmed();

			if (scope.size() == 0)
			{
				scope = "global";
			}

			const size_t result_limit = find_limit_edit->text().trimmed().toULongLong();

			const auto req = std::make_shared<GetOrderedDatastoreEntryListRequest>(api_key, universe_id, datastore_name, scope, ascending);
			if (result_limit > 0)
			{
				req->set_result_limit(result_limit);
			}
			OperationInProgressDialog diag{ this, req };
			diag.exec();

			if (save_recent_datastores_check->isChecked() && req->get_entries().size() > 0)
			{
				selected_universe->add_recent_ordered_datastore(datastore_name);
			}

			OrderedDatastoreEntryQTableModel* const qt_model = new OrderedDatastoreEntryQTableModel{ datastore_entry_tree, req->get_entries() };
			set_datastore_entry_model(qt_model);
		}
	}
}

void OrderedDatastorePanel::pressed_find_ascending()
{
	pressed_find(true);
}

void OrderedDatastorePanel::pressed_find_descending()
{
	pressed_find(false);
}

void OrderedDatastorePanel::pressed_remove_datastore()
{
	if (UniverseProfile* const selected_universe = UserProfile::get_selected_universe())
	{
		QList<QListWidgetItem*> selected = select_datastore_list->selectedItems();
		if (selected.size() == 1)
		{
			selected_universe->remove_recent_ordered_datastore(selected.front()->text());
		}
	}
}

void OrderedDatastorePanel::pressed_view_entry()
{
	view_entry(get_selected_single_index());
}
