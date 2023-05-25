#include "panel_datastore_ordered.h"

#include <QLineEdit>
#include <QListWidget>

#include "profile.h"

OrderedDatastorePanel::OrderedDatastorePanel(QWidget* parent, const QString& api_key) : BaseDatastorePanel(parent, api_key)
{
	clear_model();
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
