#include "window_ordered_datastore_entry_view.h"

#include <QFormLayout>
#include <QLineEdit>

#include "model_common.h"

ViewOrderedDatastoreEntryWindow::ViewOrderedDatastoreEntryWindow(QWidget* const parent, const QString& api_key, const OrderedDatastoreEntryFull& details, const ViewEditMode view_edit_mode) :
	QWidget{ parent, Qt::Window }
{
	setAttribute(Qt::WA_DeleteOnClose);

	setWindowTitle("View Ordered Datastore Entry");
	setMinimumWidth(320);

	QWidget* const info_panel = new QWidget{ this };
	{
		universe_id_edit = new QLineEdit{ info_panel };
		universe_id_edit->setReadOnly(true);
		universe_id_edit->setText(QString::number(details.get_universe_id()));

		datastore_name_edit = new QLineEdit{ info_panel };
		datastore_name_edit->setReadOnly(true);
		datastore_name_edit->setText(details.get_datastore_name());

		scope_edit = new QLineEdit{ info_panel };
		scope_edit->setReadOnly(true);
		scope_edit->setText(details.get_scope());

		key_name_edit = new QLineEdit{ info_panel };
		key_name_edit->setReadOnly(true);
		key_name_edit->setText(details.get_key_id());

		value_edit = new QLineEdit{ info_panel };
		value_edit->setReadOnly(true);
		value_edit->setText(QString::number(details.get_value()));

		QFormLayout* const info_layout = new QFormLayout{ info_panel };
		info_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		info_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		info_layout->addRow("Universe", universe_id_edit);
		info_layout->addRow("Datastore", datastore_name_edit);
		info_layout->addRow("Scope", scope_edit);
		info_layout->addRow("Key", key_name_edit);
		info_layout->addRow("Value", value_edit);
	}

	QVBoxLayout* const layout = new QVBoxLayout{ this };
	layout->addWidget(info_panel);
}
