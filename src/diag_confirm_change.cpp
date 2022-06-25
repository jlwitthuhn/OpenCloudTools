#include "diag_confirm_change.h"

#include <memory>
#include <optional>

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "api_key.h"
#include "user_settings.h"

ConfirmChangeDialog::ConfirmChangeDialog(QWidget* const parent, const ChangeType change_type) : QDialog{ parent }
{
	setWindowTitle("Confirm Change");

	QLabel* info = new QLabel{ this };
	switch (change_type)
	{
	case ChangeType::Revert:
		info->setText("This action will re-save an old version of a datastore entry as the newest version. Are you sure you want to do this?");
		break;
	case ChangeType::Update:
		info->setText("This action will update the current value of the selected entry. Are you sure you want to do this?");
		break;
	case ChangeType::Delete:
		info->setText("This action will delete the selected entry. Are you sure you want to do this?");
		break;
	case ChangeType::BulkDelete:
		info->setText("This action will delete the contents of the selected datastores. Are you sure you want to do this?");
		break;
	case ChangeType::BulkUndelete:
		info->setText("This action will restore previously deleted data in the selected datastores. Are you sure you want to do this?");
		break;
	}
	info->setWordWrap(true);

	QGroupBox* prod_box = nullptr;
	if (std::optional<ApiKeyProfile> profile = UserSettings::get()->get_selected_profile())
	{
		if (profile->production())
		{
			prod_box = new QGroupBox{ "Production Warning", this };

			QLabel* prod_info = new QLabel{ prod_box };
			prod_info->setText("!! You are using a Production key !!");
			prod_info->setWordWrap(true);

			QLabel* prod_info2 = new QLabel{ prod_box };
			prod_info2->setText("Check the box below to confirm that you have carefully reviewed the change you are making.");
			prod_info2->setWordWrap(true);

			prod_confirm_check = new QCheckBox{ "This change will not break production", prod_box };
			connect(prod_confirm_check, &QCheckBox::stateChanged, this, &ConfirmChangeDialog::handle_prod_confirm_check_changed);

			QVBoxLayout* layout = new QVBoxLayout{ prod_box };
			layout->addWidget(prod_info);
			layout->addWidget(prod_info2);
			layout->addWidget(prod_confirm_check);
		}
	}

	QWidget* button_panel = new QWidget{ this };
	{
		yes_button = new QPushButton{ "&Yes", button_panel };
		connect(yes_button, &QPushButton::clicked, this, &QDialog::accept);

		QPushButton* no_button = new QPushButton{ "&No", button_panel };
		connect(no_button, &QPushButton::clicked, this, &QDialog::reject);

		QHBoxLayout* button_layout = new QHBoxLayout{ button_panel };
		button_layout->addWidget(yes_button);
		button_layout->addWidget(no_button);
	}

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
	layout->addWidget(info);
	if (prod_box)
	{
		layout->addWidget(prod_box);
	}
	layout->addWidget(button_panel);

	setFixedWidth(265);
	handle_prod_confirm_check_changed();
}

void ConfirmChangeDialog::handle_prod_confirm_check_changed()
{
	if (prod_confirm_check)
	{
		yes_button->setEnabled(prod_confirm_check->isChecked());
	}
	else
	{
		yes_button->setEnabled(true);
	}
}
