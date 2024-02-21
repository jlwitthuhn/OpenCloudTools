#include "window_datastore_entry_view.h"

#include <memory>

#include <Qt>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QLineEdit>
#include <QMargins>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QString>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

#include "data_request.h"
#include "diag_confirm_change.h"
#include "diag_operation_in_progress.h"
#include "model_common.h"
#include "util_enum.h"
#include "util_json.h"
#include "util_validator.h"

ViewDatastoreEntryWindow::ViewDatastoreEntryWindow(QWidget* parent, const QString& api_key, const StandardDatastoreEntryFull& details, const ViewEditMode view_edit_mode) : QWidget{ parent, Qt::Window }, data_type{ details.get_entry_type() }, api_key{ api_key }
{
	setAttribute(Qt::WA_DeleteOnClose);

	switch (view_edit_mode)
	{
	case ViewEditMode::Edit:
		setWindowTitle("Edit Datastore Entry");
		setMinimumWidth(640);
		break;
	case ViewEditMode::View:
		setWindowTitle("View Datastore Entry");
		setMinimumWidth(320);
		break;
	}

	QString displayed_data = details.get_data_decoded();
	if (details.get_entry_type() == DatastoreEntryType::Json)
	{
		if (std::optional<QString> formatted = format_json(displayed_data))
		{
			displayed_data = *formatted;
		}
	}

	std::optional<QString> displayed_userids = details.get_userids();
	if (displayed_userids)
	{
		if (std::optional<QString> formatted = format_json(*displayed_userids))
		{
			displayed_userids = *formatted;
		}
	}

	std::optional<QString> displayed_attributes = details.get_attributes();
	if (displayed_attributes)
	{
		if (std::optional<QString> formatted = format_json(*displayed_attributes))
		{
			displayed_attributes = *formatted;
		}
	}

	QWidget* info_panel = new QWidget{ this };
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
		key_name_edit->setText(details.get_key_name());

		version_edit = new QLineEdit{ info_panel };
		version_edit->setReadOnly(true);
		version_edit->setText(details.get_version());

		QWidget* type_panel = new QWidget{ info_panel };
		{
			QRadioButton* type_radio_json = new QRadioButton{ get_enum_string(DatastoreEntryType::Json), type_panel };
			type_radio_json->setChecked(details.get_entry_type() == DatastoreEntryType::Json);
			type_radio_json->setCheckable(details.get_entry_type() == DatastoreEntryType::Json);

			QRadioButton* type_radio_string = new QRadioButton{ get_enum_string(DatastoreEntryType::String), type_panel };
			type_radio_string->setChecked(details.get_entry_type() == DatastoreEntryType::String);
			type_radio_string->setCheckable(details.get_entry_type() == DatastoreEntryType::String);

			QRadioButton* type_radio_number = new QRadioButton{ get_enum_string(DatastoreEntryType::Number), type_panel };
			type_radio_number->setChecked(details.get_entry_type() == DatastoreEntryType::Number);
			type_radio_number->setCheckable(details.get_entry_type() == DatastoreEntryType::Number);

			QRadioButton* type_radio_bool = new QRadioButton{ get_enum_string(DatastoreEntryType::Bool), type_panel };
			type_radio_bool->setChecked(details.get_entry_type() == DatastoreEntryType::Bool);
			type_radio_bool->setCheckable(details.get_entry_type() == DatastoreEntryType::Bool);

			QVBoxLayout* type_layout = new QVBoxLayout{ type_panel };
			type_layout->setContentsMargins(QMargins{ 2, 2, 2, 2 });
			type_layout->setSpacing(0);
			type_layout->addWidget(type_radio_json);
			type_layout->addWidget(type_radio_string);
			type_layout->addWidget(type_radio_number);
			type_layout->addWidget(type_radio_bool);
		}

		QFormLayout* info_layout = new QFormLayout{ info_panel };
		info_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
        info_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		info_layout->addRow("Universe", universe_id_edit);
		info_layout->addRow("Datastore", datastore_name_edit);
		info_layout->addRow("Scope", scope_edit);
		info_layout->addRow("Key", key_name_edit);
		info_layout->addRow("Version", version_edit);
		info_layout->addRow("Type", type_panel);
	}

	QWidget* data_panel = new QWidget{ this };
	{
		QGroupBox* data_group = new QGroupBox{ "Data", data_panel };
		{
			QTabWidget* tab_widget = new QTabWidget{ data_group };
			{
				data_edit = new QTextEdit{ tab_widget };
				data_edit->setReadOnly(true);
				data_edit->setText(displayed_data);

				if (displayed_userids)
				{
					userids_edit = new QTextEdit{ tab_widget };
					userids_edit->setReadOnly(true);
					userids_edit->setText(*displayed_userids);
				}

				if (displayed_attributes)
				{
					attributes_edit = new QTextEdit{ tab_widget };
					attributes_edit->setReadOnly(true);
					attributes_edit->setText(*displayed_attributes);
				}

				tab_widget->addTab(data_edit, "Data");
				if (userids_edit != nullptr)
				{
					tab_widget->addTab(userids_edit, "User IDs");
				}
				if (attributes_edit != nullptr)
				{
					tab_widget->addTab(attributes_edit, "Attributes");
				}
			}

			QVBoxLayout* data_group_layout = new QVBoxLayout{ data_group };
			data_group_layout->addWidget(tab_widget);
		}

		QGroupBox* edit_group = nullptr;
		if (view_edit_mode == ViewEditMode::Edit)
		{
			edit_group = new QGroupBox{ "New Data", data_panel };
			QTabWidget* tab_widget = new QTabWidget{ edit_group };
			{
				new_data_edit = new QTextEdit{ tab_widget };
				new_data_edit->setAcceptRichText(false);
				new_data_edit->setText(displayed_data);

				new_userids_edit = new QTextEdit{ tab_widget };
				new_userids_edit->setAcceptRichText(false);
				if (displayed_userids)
				{
					new_userids_edit->setText(*displayed_userids);
				}

				new_attributes_edit = new QTextEdit{ tab_widget };
				new_attributes_edit->setAcceptRichText(false);
				if (displayed_attributes)
				{
					new_attributes_edit->setText(*displayed_attributes);
				}

				tab_widget->addTab(new_data_edit, "Data");
				tab_widget->addTab(new_userids_edit, "User IDs");
				tab_widget->addTab(new_attributes_edit, "Attributes");
			}

			QVBoxLayout* edit_group_layout = new QVBoxLayout{ edit_group };
			edit_group_layout->addWidget(tab_widget);
		}

		QHBoxLayout* data_layout = new QHBoxLayout{ data_panel };
		data_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		data_layout->addWidget(data_group);
		if (edit_group)
		{
			data_layout->addWidget(edit_group);
		}
	}

	if (view_edit_mode == ViewEditMode::Edit)
	{
		save_button = new QPushButton{ "Save", this };
		connect(save_button, &QPushButton::clicked, this, &ViewDatastoreEntryWindow::pressed_save);
	}

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->addWidget(info_panel);
	layout->addWidget(data_panel);
	if (save_button)
	{
		layout->addWidget(save_button);
	}

	switch (view_edit_mode)
	{
	case ViewEditMode::View:
		resize(420, 460);
		break;
	case ViewEditMode::Edit:
		resize(840, 480);
		break;
	}
}

std::optional<QString> ViewDatastoreEntryWindow::format_json(const QString& input_json)
{
	QJsonDocument doc = QJsonDocument::fromJson(input_json.toUtf8());
	if (doc.isNull())
	{
		return std::nullopt;
	}
	else
	{
		return doc.toJson();
	}
}

static void show_validation_error(QWidget* const parent, const QString& message)
{
	QMessageBox* message_box = new QMessageBox{ parent };
	message_box->setWindowTitle("Validation Error");
	message_box->setIcon(QMessageBox::Critical);
	message_box->setText(message);
	message_box->exec();
}

void ViewDatastoreEntryWindow::pressed_save()
{
	QString data_raw = new_data_edit->toPlainText();
	QString userids_raw = new_userids_edit->toPlainText().trimmed();
	QString attributes_str_raw = new_attributes_edit->toPlainText().trimmed();
	std::optional<QString> attributes;
	if (attributes_str_raw.size() > 0)
	{
		attributes = condense_json(attributes_str_raw);
	}

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
			show_validation_error(this, QString{ "New data is not a valid " } + get_enum_string(data_type) + ".");
			return;
		}
	}

	if (userids_raw != "" && DataValidator::is_json_array(userids_raw) == false)
	{
		show_validation_error(this, "New user list is not empty or a valid Json array.");
		return;
	}

	if (attributes_str_raw != "" && DataValidator::is_json(attributes_str_raw) == false)
	{
		show_validation_error(this, "New attributes is not empty or a valid Json object.");
		return;
	}

	ConfirmChangeDialog* confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::Update };
	bool confirmed = static_cast<bool>(confirm_dialog->exec());
	if (confirmed == false)
	{
		return;
	}

	const long long universe_id = universe_id_edit->text().toLongLong();
	const QString datastore_name = datastore_name_edit->text();
	const QString scope = scope_edit->text();
	const QString key_name = key_name_edit->text();

	const std::optional<QString> userids = condense_json(userids_raw);
	std::optional<QString> data;
	if (data_type == DatastoreEntryType::Json)
	{
		data = condense_json(data_raw);
	}
	else if (data_type == DatastoreEntryType::String)
	{
		data = encode_json_string(data_raw);
	}
	else
	{
		data = data_raw;
	}

	if (data.has_value() == false)
	{
		show_validation_error(this, "Failed to format data. This probably shouldn't happen.");
		return;
	}

	const auto post_req = std::make_shared<PostStandardDatastoreEntryRequest>(api_key, universe_id, datastore_name, scope, key_name, userids, attributes, *data);
	OperationInProgressDialog diag{ this, post_req };
	diag.exec();

	close();
}
