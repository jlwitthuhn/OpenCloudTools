#include "window_ordered_datastore_entry_view.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "assert.h"
#include "data_request.h"
#include "diag_operation_in_progress.h"
#include "model_common.h"

ViewOrderedDatastoreEntryWindow::ViewOrderedDatastoreEntryWindow(QWidget* const parent, const QString& api_key, const OrderedDatastoreEntryFull& details, const EditMode edit_mode) :
	QWidget{ parent, Qt::Window }, api_key{ api_key }
{
	OCTASSERT(parent != nullptr && parent->isWindow());
	setAttribute(Qt::WA_DeleteOnClose);

	switch (edit_mode)
	{
		case EditMode::View:
			setWindowTitle("View Ordered Datastore Entry");
			break;
		case EditMode::Increment:
			setWindowTitle("Increment Ordered Datastore Entry");
			break;
		case EditMode::Edit:
			setWindowTitle("Edit Ordered Datastore Entry");
			break;
	}
	setMinimumWidth(320);

	QWidget* const info_panel = new QWidget{ this };
	{
		universe_id_edit = new QLineEdit{ info_panel };
		universe_id_edit->setReadOnly(true);

		datastore_name_edit = new QLineEdit{ info_panel };
		datastore_name_edit->setReadOnly(true);

		scope_edit = new QLineEdit{ info_panel };
		scope_edit->setReadOnly(true);

		key_name_edit = new QLineEdit{ info_panel };
		key_name_edit->setReadOnly(true);

		value_edit = new QLineEdit{ info_panel };
		value_edit->setReadOnly(true);

		QFormLayout* const info_layout = new QFormLayout{ info_panel };
		info_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		info_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		info_layout->addRow("Universe", universe_id_edit);
		info_layout->addRow("Datastore", datastore_name_edit);
		info_layout->addRow("Scope", scope_edit);
		info_layout->addRow("Key", key_name_edit);
		info_layout->addRow("Value", value_edit);

		if (edit_mode == EditMode::Increment)
		{
			increment_edit = new QLineEdit{ info_panel };
			connect(increment_edit, &QLineEdit::textChanged, this, &ViewOrderedDatastoreEntryWindow::changed_increment);
			info_layout->addRow("Increment By", increment_edit);
		}
		else if (edit_mode == EditMode::Edit)
		{
			new_value_edit = new QLineEdit{ info_panel };
			connect(new_value_edit, &QLineEdit::textChanged, this, &ViewOrderedDatastoreEntryWindow::changed_new_value);
			info_layout->addRow("New Value", new_value_edit);
		}
	}

	QWidget* const button_panel = new QWidget{ this };
	{
		QVBoxLayout* const button_layout = new QVBoxLayout{ button_panel };
		button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });

		if (edit_mode == EditMode::Increment)
		{
			increment_submit = new QPushButton{ "Increment", button_panel };
			increment_submit->setEnabled(false);
			connect(increment_submit, &QPushButton::pressed, this, &ViewOrderedDatastoreEntryWindow::pressed_increment);
			button_layout->addWidget(increment_submit);
		}
		else if (edit_mode == EditMode::Edit)
		{
			new_value_submit = new QPushButton{ "Update", button_panel };
			new_value_submit->setEnabled(false);
			connect(new_value_submit, &QPushButton::pressed, this, &ViewOrderedDatastoreEntryWindow::pressed_new_value);
			button_layout->addWidget(new_value_submit);
		}
	}

	QVBoxLayout* const layout = new QVBoxLayout{ this };
	layout->addWidget(info_panel);
	if (edit_mode == EditMode::Increment || edit_mode == EditMode::Edit)
	{
		layout->addWidget(button_panel);
	}

	display(details);
}

bool ViewOrderedDatastoreEntryWindow::validate_contains_long(QLineEdit* const line)
{
	bool success = false;
	line->text().toLongLong(&success);
	return success;
}

void ViewOrderedDatastoreEntryWindow::display(const OrderedDatastoreEntryFull& details)
{
	universe_id_edit->setText(QString::number(details.get_universe_id()));
	datastore_name_edit->setText(details.get_datastore_name());
	scope_edit->setText(details.get_scope());
	key_name_edit->setText(details.get_key_name());
	value_edit->setText(QString::number(details.get_value()));
}

void ViewOrderedDatastoreEntryWindow::refresh()
{
	OCTASSERT(validate_contains_long(universe_id_edit));
	const long long universe_id = universe_id_edit->text().toLongLong();
	const QString datastore_name = datastore_name_edit->text();
	const QString scope = scope_edit->text();
	const QString key_name = key_name_edit->text();

	const auto req = std::make_shared<GetOrderedDatastoreEntryDetailsRequest>(
		api_key,
		universe_id,
		datastore_name,
		scope,
		key_name
	);

	OperationInProgressDialog diag{ this, req };
	diag.exec();

	if (auto new_details = req->get_details())
	{
		display(*new_details);
	}

	if (increment_edit)
	{
		increment_edit->clear();
	}
	if (new_value_edit)
	{
		new_value_edit->clear();
	}
}

void ViewOrderedDatastoreEntryWindow::changed_increment()
{
	OCTASSERT(increment_submit != nullptr && increment_edit != nullptr);
	increment_submit->setEnabled(validate_contains_long(increment_edit));
}

void ViewOrderedDatastoreEntryWindow::changed_new_value()
{
	OCTASSERT(new_value_submit != nullptr && new_value_edit != nullptr);
	new_value_submit->setEnabled(validate_contains_long(new_value_edit));
}

void ViewOrderedDatastoreEntryWindow::pressed_increment()
{
	OCTASSERT(increment_edit != nullptr);
	const bool universe_id_valid = validate_contains_long(universe_id_edit);
	const bool increment_by_valid = validate_contains_long(increment_edit);
	if (universe_id_valid && increment_by_valid)
	{
		const long long universe_id = universe_id_edit->text().toLongLong();
		const QString datastore_name = datastore_name_edit->text();
		const QString scope = scope_edit->text();
		const QString key_name = key_name_edit->text();
		const long long increment_by = increment_edit->text().toLongLong();
		auto req = std::make_shared<PostOrderedDatastoreIncrementRequest>(api_key, universe_id, datastore_name, scope, key_name, increment_by);

		OperationInProgressDialog diag{ this, req };
		diag.exec();

		if (req->req_success())
		{
			refresh();
		}
	}
}

void ViewOrderedDatastoreEntryWindow::pressed_new_value()
{
	// TODO
}
