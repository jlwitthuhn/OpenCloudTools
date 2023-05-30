#include "panel_datastore_ordered.h"

#include <QBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

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
	// TODO: Once model class is defined clear the model here
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

}

void OrderedDatastorePanel::pressed_find_descending()
{

}
