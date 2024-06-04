#include "diag_confirm_change.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "profile.h"

ConfirmChangeDialog::ConfirmChangeDialog(QWidget* const parent, const ChangeType change_type) : QDialog{ parent }
{
	setWindowTitle("Confirm Change");

	QLabel* info = new QLabel{ this };
	switch (change_type)
	{
	case ChangeType::OrderedDatastoreCreate:
		info->setText("This action will create a new ordered datastore entry. Are you sure you want to do this?");
		break;
	case ChangeType::OrderedDatastoreIncrement:
		info->setText("This action will increment the selected ordered datastore entry. Are you sure you want to do this?");
		break;
	case ChangeType::OrderedDatastoreUpdate:
		info->setText("This action will update the selected ordered datastore entry. Are you sure you want to do this?");
		break;
	case ChangeType::StandardDatastoreRevert:
		info->setText("This action will re-save an old version of a datastore entry as the newest version. Are you sure you want to do this?");
		break;
	case ChangeType::StandardDatastoreUpdate:
		info->setText("This action will update the current value of the selected datastore entry. Are you sure you want to do this?");
		break;
	case ChangeType::StandardDatastoreDelete:
		info->setText("This action will delete the selected entry. Are you sure you want to do this?");
		break;
	case ChangeType::StandardDatastoreMultiDelete:
		info->setText("This action will delete all of the selected entries. Are you sure you want to do this?");
		break;
	case ChangeType::StandardDatastoreBulkDelete:
		info->setText("This action will delete the contents of the selected datastores. Are you sure you want to do this?");
		break;
	case ChangeType::StandardDatastoreBulkUndelete:
		info->setText("This action will restore previously deleted data in the selected datastores. Are you sure you want to do this?");
		break;
	case ChangeType::StandardDatastoreBulkUpload:
		info->setText("This action will load the entire contents of a sqlite dump into this universe's datastore(s). Are you sure you want to do this?");
		break;
	}
	info->setWordWrap(true);

	QGroupBox* prod_box = nullptr;
	if (ApiKeyProfile* profile = UserProfile::get_selected_api_key())
	{
		if (profile->get_production())
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
