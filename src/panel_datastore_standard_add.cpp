#include "panel_datastore_standard_add.h"

#include <optional>
#include <string>
#include <vector>

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QTabWidget>
#include <QTextEdit>
#include <QVariant>

#include "assert.h"
#include "data_request.h"
#include "diag_confirm_change.h"
#include "diag_list_string.h"
#include "diag_operation_in_progress.h"
#include "profile.h"
#include "util_alert.h"
#include "util_enum.h"
#include "util_json.h"
#include "util_validator.h"

StandardDatastoreAddEntryPanel::StandardDatastoreAddEntryPanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe) :
	QWidget{ parent },
	api_key{ api_key },
	attached_universe{ universe }
{
	QWidget* const datastore_name_line = new QWidget{ this };
	{
		edit_add_datastore_name = new QLineEdit{ datastore_name_line };
		connect(edit_add_datastore_name, &QLineEdit::textChanged, this, &StandardDatastoreAddEntryPanel::gui_refresh);

		QPushButton* const button_datastore_name_lookup = new QPushButton{ "...", datastore_name_line };
		connect(button_datastore_name_lookup, &QPushButton::clicked, this, &StandardDatastoreAddEntryPanel::pressed_select_datastore);

		QHBoxLayout* const datastore_name_line_layout = new QHBoxLayout{ datastore_name_line };
		datastore_name_line_layout->setContentsMargins(0, 0, 0, 0);
		datastore_name_line_layout->addWidget(edit_add_datastore_name);
		datastore_name_line_layout->addWidget(button_datastore_name_lookup);
	}

	edit_add_datastore_scope = new QLineEdit{ this };
	edit_add_datastore_scope->setPlaceholderText("global");
	connect(edit_add_datastore_scope, &QLineEdit::textChanged, this, &StandardDatastoreAddEntryPanel::gui_refresh);

	edit_add_datastore_key_name = new QLineEdit{ this };
	connect(edit_add_datastore_key_name, &QLineEdit::textChanged, this, &StandardDatastoreAddEntryPanel::gui_refresh);

	combo_add_entry_type = new QComboBox{ this };
	combo_add_entry_type->setSizePolicy(QSizePolicy{ QSizePolicy::Expanding, QSizePolicy::Preferred });
	combo_add_entry_type->addItem(get_enum_string(DatastoreEntryType::Json), static_cast<int>(DatastoreEntryType::Json));
	combo_add_entry_type->addItem(get_enum_string(DatastoreEntryType::String), static_cast<int>(DatastoreEntryType::String));
	combo_add_entry_type->addItem(get_enum_string(DatastoreEntryType::Number), static_cast<int>(DatastoreEntryType::Number));
	combo_add_entry_type->addItem(get_enum_string(DatastoreEntryType::Bool), static_cast<int>(DatastoreEntryType::Bool));

	QTabWidget* const tab_add = new QTabWidget{ this };
	{
		edit_add_entry_data = new QTextEdit{ tab_add };
		edit_add_entry_data->setAcceptRichText(false);
		connect(edit_add_entry_data, &QTextEdit::textChanged, this, &StandardDatastoreAddEntryPanel::gui_refresh);

		edit_add_entry_userids = new QTextEdit{ tab_add };
		edit_add_entry_userids->setAcceptRichText(false);

		edit_add_entry_attributes = new QTextEdit{ tab_add };
		edit_add_entry_attributes->setAcceptRichText(false);

		tab_add->addTab(edit_add_entry_data, "Data");
		tab_add->addTab(edit_add_entry_userids, "User IDs");
		tab_add->addTab(edit_add_entry_attributes, "Attributes");
	}

	button_add_entry_submit = new QPushButton{ "Submit", this };
	button_add_entry_submit->setSizePolicy(QSizePolicy{ QSizePolicy::Expanding, QSizePolicy::Preferred });
	connect(button_add_entry_submit, &QPushButton::clicked, this, &StandardDatastoreAddEntryPanel::pressed_submit);

	QFormLayout* const layout = new QFormLayout{ this };
	layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	layout->addRow("Data store", datastore_name_line);
	layout->addRow("Scope", edit_add_datastore_scope);
	layout->addRow("Key", edit_add_datastore_key_name);
	layout->addRow("Type", combo_add_entry_type);
	layout->addRow("Data", tab_add);
	layout->addRow("", button_add_entry_submit);

	gui_refresh();
}

void StandardDatastoreAddEntryPanel::gui_refresh()
{
	const bool submit_enabled = edit_add_datastore_name->text().size() > 0 && edit_add_datastore_key_name->text().size() > 0 && edit_add_entry_data->toPlainText().size() > 0;
	button_add_entry_submit->setEnabled(submit_enabled);
}

void StandardDatastoreAddEntryPanel::datastore_name_selected(const QString& name)
{
	edit_add_datastore_name->setText(name);
}

void StandardDatastoreAddEntryPanel::pressed_select_datastore()
{
	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		return;
	}

	const auto req = std::make_shared<StandardDatastoreGetListRequest>(api_key, universe->get_universe_id());
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	if (req->get_datastore_names().size() == 0)
	{
		return;
	}

	StringListDialog* const list_dialog = new StringListDialog{ "Select a data store", req->get_datastore_names(), this };
	connect(list_dialog, &StringListDialog::selected, this, &StandardDatastoreAddEntryPanel::datastore_name_selected);
	list_dialog->open();
}

void StandardDatastoreAddEntryPanel::pressed_submit()
{
	const QString data_raw = edit_add_entry_data->toPlainText();
	const QString userids_raw = edit_add_entry_userids->toPlainText().trimmed();
	const QString attributes_str_raw = edit_add_entry_attributes->toPlainText().trimmed();

	std::optional<QString> attributes;
	if (attributes_str_raw.size() > 0)
	{
		attributes = condense_json(attributes_str_raw);
	}

	const DatastoreEntryType data_type = static_cast<DatastoreEntryType>(combo_add_entry_type->currentData().toInt());
	{
		bool data_valid = false;
		switch (data_type)
		{
		case DatastoreEntryType::Error:
			data_valid = false;
			break;
		case DatastoreEntryType::Bool:
			data_valid = DataValidator::is_bool(data_raw);
			break;
		case DatastoreEntryType::Number:
			data_valid = DataValidator::is_number(data_raw);
			break;
		case DatastoreEntryType::String:
			data_valid = true;
			break;
		case DatastoreEntryType::Json:
			data_valid = DataValidator::is_json(data_raw);
			break;
		}

		if (data_valid == false)
		{
			alert_error_blocking("Validation Error", std::string{ "New data is not a valid " } + get_enum_string(data_type).toStdString() + ".");
			return;
		}
	}

	if (userids_raw != "" && DataValidator::is_json_array(userids_raw) == false)
	{
		alert_error_blocking("Validation Error", "New user list is not empty or a valid Json array.");
		return;
	}

	if (attributes_str_raw != "" && DataValidator::is_json(attributes_str_raw) == false)
	{
		alert_error_blocking("Validation Error", "New attributes is not empty or a valid Json object.");
		return;
	}

	ConfirmChangeDialog* confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::StandardDatastoreUpdate };
	bool confirmed = static_cast<bool>(confirm_dialog->exec());
	if (confirmed == false)
	{
		return;
	}

	std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		return;
	}

	const long long universe_id = universe->get_universe_id();
	const QString datastore_name = edit_add_datastore_name->text();
	const QString scope = edit_add_datastore_scope->text().size() > 0 ? edit_add_datastore_scope->text() : "global";
	const QString key_name = edit_add_datastore_key_name->text();

	const std::optional<QString> userids = condense_json(userids_raw);
	std::optional<QString> data_formatted;
	if (data_type == DatastoreEntryType::Json)
	{
		data_formatted = condense_json(data_raw);
	}
	else if (data_type == DatastoreEntryType::String)
	{
		data_formatted = encode_json_string(data_raw);
	}
	else
	{
		data_formatted = data_raw;
	}

	OCTASSERT(data_formatted.has_value());

	const auto post_req = std::make_shared<StandardDatastoreEntryPostSetRequest>(api_key, universe_id, datastore_name, scope, key_name, userids, attributes, *data_formatted);
	OperationInProgressDialog diag{ this, post_req };
	diag.exec();

	if (post_req->req_success())
	{
		edit_add_entry_data->clear();
		edit_add_entry_userids->clear();
		edit_add_entry_attributes->clear();
	}
}
