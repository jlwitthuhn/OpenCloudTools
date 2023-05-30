#include "panel_datastore_ordered.h"

#include <QBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTreeView>

#include "data_request.h"
#include "diag_operation_in_progress.h"
#include "model_qt.h"
#include "profile.h"

OrderedDatastorePanel::OrderedDatastorePanel(QWidget* parent, const QString& api_key) : BaseDatastorePanel(parent, api_key)
{
	// Main panel
	{
		find_ascending_button = new QPushButton{ "Find all (ascending)", search_submit_widget };
		connect(find_ascending_button, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_find_ascending);

		find_descending_button = new QPushButton{ "Find all (descending)", search_submit_widget };
		connect(find_descending_button, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_find_descending);

		QBoxLayout* const search_submit_layout = dynamic_cast<QBoxLayout*>(search_submit_widget->layout());
		search_submit_layout->insertWidget(0, find_ascending_button);
		search_submit_layout->insertWidget(1, find_descending_button);
	}

	clear_model();
	handle_search_text_changed();
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
	handle_selected_datastore_entry_changed();
}

void OrderedDatastorePanel::handle_search_text_changed()
{
	BaseDatastorePanel::handle_search_text_changed();
	const UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
	const bool enabled = search_datastore_name_edit->text().size() > 0 && selected_universe;
	find_ascending_button->setEnabled(enabled);
	find_descending_button->setEnabled(enabled);
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

void OrderedDatastorePanel::pressed_find_ascending()
{
	const UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
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

			const auto req = std::make_shared<GetOrderedDatastoreEntryListRequest>(api_key, universe_id, datastore_name, scope, true);
			if (result_limit > 0)
			{
				req->set_result_limit(result_limit);
			}
			OperationInProgressDialog diag{ this, req };
			diag.exec();

			OrderedDatastoreEntryQTableModel* const qt_model = new OrderedDatastoreEntryQTableModel{ datastore_entry_tree, req->get_entries() };
			set_datastore_entry_model(qt_model);
		}
	}
}

void OrderedDatastorePanel::pressed_find_descending()
{
	const UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
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

			const auto req = std::make_shared<GetOrderedDatastoreEntryListRequest>(api_key, universe_id, datastore_name, scope, false);
			if (result_limit > 0)
			{
				req->set_result_limit(result_limit);
			}
			OperationInProgressDialog diag{ this, req };
			diag.exec();

			OrderedDatastoreEntryQTableModel* const qt_model = new OrderedDatastoreEntryQTableModel{ datastore_entry_tree, req->get_entries() };
			set_datastore_entry_model(qt_model);
		}
	}
}
