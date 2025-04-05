#include "panel_datastore_ordered_add.h"

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSizePolicy>

#include "assert.h"
#include "data_request.h"
#include "diag_confirm_change.h"
#include "diag_list_string.h"
#include "diag_operation_in_progress.h"
#include "profile.h"
#include "util_alert.h"
#include "util_validator.h"

OrderedDatastoreAddEntryPanel::OrderedDatastoreAddEntryPanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe) :
	QWidget{ parent },
	api_key{ api_key },
	attached_universe{ universe }
{
	setMinimumWidth(400);

	QWidget* const datastore_name_line = new QWidget{ this };
	{
		datastore_name_edit = new QLineEdit{ datastore_name_line };
		connect(datastore_name_edit, &QLineEdit::textChanged, this, &OrderedDatastoreAddEntryPanel::gui_refresh);

		QPushButton* const button_datastore_name_lookup = new QPushButton{ "...", datastore_name_line };
		connect(button_datastore_name_lookup, &QPushButton::clicked, this, &OrderedDatastoreAddEntryPanel::pressed_select_datastore);

		QHBoxLayout* const datastore_name_line_layout = new QHBoxLayout{ datastore_name_line };
		datastore_name_line_layout->setContentsMargins(0, 0, 0, 0);
		datastore_name_line_layout->addWidget(datastore_name_edit);
		datastore_name_line_layout->addWidget(button_datastore_name_lookup);
	}

	scope_edit = new QLineEdit{ this };
	scope_edit->setPlaceholderText("global");
	connect(scope_edit, &QLineEdit::textChanged, this, &OrderedDatastoreAddEntryPanel::gui_refresh);

	key_name_edit = new QLineEdit{ this };
	connect(key_name_edit, &QLineEdit::textChanged, this, &OrderedDatastoreAddEntryPanel::gui_refresh);

	value_edit = new QLineEdit{ this };
	connect(value_edit, &QLineEdit::textChanged, this, &OrderedDatastoreAddEntryPanel::gui_refresh);

	button_submit = new QPushButton{ "Submit", this };
	button_submit->setSizePolicy(QSizePolicy{ QSizePolicy::Expanding, QSizePolicy::Preferred });
	connect(button_submit, &QPushButton::clicked, this, &OrderedDatastoreAddEntryPanel::pressed_submit);

	QFormLayout* const layout = new QFormLayout{ this };
	layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	layout->addRow("Data store", datastore_name_line);
	layout->addRow("Scope", scope_edit);
	layout->addRow("Key", key_name_edit);
	layout->addRow("Value", value_edit);
	layout->addRow("", button_submit);

	gui_refresh();
}

void OrderedDatastoreAddEntryPanel::gui_refresh()
{
	const bool add_submit_enabled =
		datastore_name_edit->text().size() > 0 &&
		key_name_edit->text().size() > 0 &&
		value_edit->text().size() > 0;
	button_submit->setEnabled(add_submit_enabled);
}

void OrderedDatastoreAddEntryPanel::datastore_name_selected(const QString& name)
{
	datastore_name_edit->setText(name);
}

void OrderedDatastoreAddEntryPanel::pressed_select_datastore()
{
	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		OCTASSERT(false);
		return;
	}

	const std::set<QString>& string_set = universe->get_recent_ordered_datastore_set();
	const std::vector<QString> string_list{ string_set.begin(), string_set.end() };

	StringListDialog* const list_dialog = new StringListDialog{ "Select a data store", string_list, this };
	connect(list_dialog, &StringListDialog::selected, this, &OrderedDatastoreAddEntryPanel::datastore_name_selected);
	list_dialog->open();
}

void OrderedDatastoreAddEntryPanel::pressed_submit()
{
	const QString data_raw = value_edit->text();

	const bool value_valid = DataValidator::is_number(data_raw);
	if (value_valid == false)
	{
		alert_error_blocking("Validation Error", "New value is not a valid integer.");
		return;
	}

	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		OCTASSERT(false);
		return;
	}

	ConfirmChangeDialog* const confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::OrderedDatastoreCreate };
	const bool confirmed = static_cast<bool>(confirm_dialog->exec());
	if (confirmed == false)
	{
		return;
	}

	const long long universe_id = universe->get_universe_id();
	const QString datastore_name = datastore_name_edit->text();
	const QString scope = scope_edit->text().size() > 0 ? scope_edit->text() : "global";
	const QString key_name = key_name_edit->text();
	const long long value = value_edit->text().toLongLong();

	const auto post_req = std::make_shared<OrderedDatastoreEntryPostCreateV2Request>(api_key, universe_id, datastore_name, scope, key_name, value);
	OperationInProgressDialog diag{ this, post_req };
	diag.exec();

	if (post_req->req_success())
	{
		value_edit->clear();
	}
}
