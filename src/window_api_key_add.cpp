#include "window_api_key_add.h"

#include <memory>

#include <Qt>
#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QLineEdit>
#include <QMargins>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include "user_settings.h"

AddApiKeyWindow::AddApiKeyWindow(QWidget* const parent, const std::optional<size_t> existing_key_index_in) : QWidget{ parent, Qt::Window }
{
	setAttribute(Qt::WA_DeleteOnClose);

	const std::optional<ApiKeyProfile> existing_key_profile = existing_key_index_in.has_value() ? UserSettings::get()->get_api_key(*existing_key_index_in) : std::nullopt;
	if (existing_key_profile)
	{
		setWindowTitle("Edit API Key");
		existing_key_index = existing_key_index_in;
	}
	else
	{
		setWindowTitle("Add API Key");
	}

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);

	QWidget* info_panel = new QWidget{ this };
	{
		name_edit = new QLineEdit{ info_panel };
		if (existing_key_profile)
		{
			name_edit->setText(existing_key_profile->name());
		}
		connect(name_edit, &QLineEdit::textChanged, this, &AddApiKeyWindow::input_changed);

		key_edit = new QLineEdit{ info_panel };
		if (existing_key_profile)
		{
			key_edit->setText(existing_key_profile->key());
		}
		connect(key_edit, &QLineEdit::textChanged, this, &AddApiKeyWindow::input_changed);

		production_check = new QCheckBox{ "Production key (adds extra confirmations)" };
		if (existing_key_profile)
		{
			production_check->setChecked(existing_key_profile->production());
		}

		save_to_disk_check = new QCheckBox{ "Save to disk", info_panel };
		if (existing_key_profile)
		{
			save_to_disk_check->setChecked(existing_key_profile->save_to_disk());
		}

		QFormLayout* info_layout = new QFormLayout{ info_panel };
		info_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
        info_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		info_layout->addRow("Name", name_edit);
		info_layout->addRow("Key", key_edit);
		info_layout->addRow("", production_check);
		info_layout->addRow("", save_to_disk_check);
	}
	layout->addWidget(info_panel);

	QWidget* button_panel = new QWidget{ this };
	button_panel->setMinimumWidth(280);
	{
		QHBoxLayout* button_layout = new QHBoxLayout{ button_panel };

		save_button = new QPushButton{ "Save", button_panel };
		button_layout->addWidget(save_button);
		if (existing_key_profile)
		{
			connect(save_button, &QPushButton::clicked, this, &AddApiKeyWindow::update_key);
		}
		else
		{
			connect(save_button, &QPushButton::clicked, this, &AddApiKeyWindow::add_key);
		}

		QPushButton* cancel_button = new QPushButton{ "Cancel", button_panel };
		button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		button_layout->addWidget(cancel_button);
		connect(cancel_button, &QPushButton::clicked, this, &QWidget::close);
	}
	layout->addWidget(button_panel);

	input_changed();
}

bool AddApiKeyWindow::input_is_valid() const
{
	const bool name_valid = name_edit->text().size() > 0;
	const bool key_valid = key_edit->text().size() >= 24; // I think it has to be 48 exactly but I don't know that for sure
	return name_valid && key_valid;
}

void AddApiKeyWindow::input_changed()
{
	save_button->setEnabled(input_is_valid());
}

void AddApiKeyWindow::add_key()
{
	if (input_is_valid())
	{
		ApiKeyProfile new_key{ name_edit->text(), key_edit->text().trimmed(), production_check->isChecked(), save_to_disk_check->isChecked() };
		std::optional<size_t> new_key_id = UserSettings::get()->add_api_key(new_key);
		if (new_key_id)
		{
			close();
		}
		else
		{
			QMessageBox* msg_box = new QMessageBox{ this };
			msg_box->setWindowTitle("Error");
			msg_box->setText("Failed to add API key. A key with that name already exists.");
			msg_box->exec();
		}
	}
}

void AddApiKeyWindow::update_key()
{
	if (input_is_valid() && existing_key_index)
	{
		ApiKeyProfile new_key{ name_edit->text(), key_edit->text(), production_check->isChecked(), save_to_disk_check->isChecked() };
		UserSettings::get()->update_api_key(*existing_key_index, new_key);
		close();
	}
}
