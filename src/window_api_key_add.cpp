#include "window_api_key_add.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "api_key.h"
#include "user_settings.h"

AddApiKeyWindow::AddApiKeyWindow(QWidget* parent, std::optional<std::pair<uint, ApiKeyProfile>> existing) : QWidget{ parent, Qt::Window }
{
	if (existing)
	{
		setWindowTitle("Edit API Key");
		existing_id = existing->first;
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
		if (existing)
		{
			name_edit->setText(existing->second.name());
		}
		connect(name_edit, &QLineEdit::textChanged, this, &AddApiKeyWindow::input_changed);

		key_edit = new QLineEdit{ info_panel };
		if (existing)
		{
			key_edit->setText(existing->second.key());
		}
		connect(key_edit, &QLineEdit::textChanged, this, &AddApiKeyWindow::input_changed);

		production_check = new QCheckBox{ "Production key (adds extra confirmations)" };
		if (existing)
		{
			production_check->setChecked(existing->second.production());
		}

		save_to_disk_check = new QCheckBox{ "Save to disk", info_panel };
		if (existing)
		{
			save_to_disk_check->setChecked(existing->second.save_to_disk());
		}

		QFormLayout* info_layout = new QFormLayout{ info_panel };
		info_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
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
		if (existing)
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
		UserSettings::get()->add_api_key(new_key);
		close();
	}
}

void AddApiKeyWindow::update_key()
{
	if (input_is_valid() && existing_id > 0)
	{
		ApiKeyProfile new_key{ name_edit->text(), key_edit->text(), production_check->isChecked(), save_to_disk_check->isChecked() };
		UserSettings::get()->update_api_key(existing_id, new_key);
		close();
	}
}
