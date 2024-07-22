#include "window_api_key_manage.h"

#include <cstddef>

#include <memory>
#include <optional>
#include <vector>

#include <Qt>
#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMargins>
#include <QMessageBox>
#include <QMetaType>
#include <QPushButton>
#include <QString>
#include <QVariant>
#include <QVBoxLayout>

#include "assert.h"
#include "profile.h"
#include "tooltip_text.h"
#include "window_main.h"

static bool variant_is_byte_array(const QVariant& variant)
{
#ifdef QT5_COMPAT
	return variant.type() == QVariant::Type::QByteArray;
#else
	return variant.metaType().id() == QMetaType::QByteArray;
#endif
}

ManageApiKeysWindow::ManageApiKeysWindow(QWidget* parent) : QWidget{ parent, Qt::Window }
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle("API Keys");

#ifdef OCT_NEW_GUI
	OCTASSERT(parent);
	setWindowModality(Qt::WindowModality::WindowModal);
#endif

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->setAlignment(Qt::AlignHCenter);

	QLabel* title_label = new QLabel{ this };
	title_label->setText("Manage Roblox API Keys");
	layout->addWidget(title_label);

	list_widget = new QListWidget{ this };
	layout->addWidget(list_widget);

	connect(list_widget, &QListWidget::itemSelectionChanged, this, &ManageApiKeysWindow::selection_changed);
	connect(list_widget, &QListWidget::itemDoubleClicked, this, &ManageApiKeysWindow::double_clicked_profile);

	QWidget* button_panel_top = new QWidget{ this };
	{
		QHBoxLayout* button_layout = new QHBoxLayout{ button_panel_top };
		button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });

		QPushButton* add_button = new QPushButton{ "Add...", button_panel_top };
		button_layout->addWidget(add_button);
		connect(add_button, &QPushButton::clicked, this, &ManageApiKeysWindow::pressed_add);

		mod_button = new QPushButton{ "Edit...", button_panel_top };
		mod_button->setEnabled(false);
		button_layout->addWidget(mod_button);
		connect(mod_button, &QPushButton::clicked, this, &ManageApiKeysWindow::pressed_edit);

		del_button = new QPushButton{ "Delete", button_panel_top };
		del_button->setEnabled(false);
		button_layout->addWidget(del_button);
		connect(del_button, &QPushButton::clicked, this, &ManageApiKeysWindow::pressed_delete);
	}
	layout->addWidget(button_panel_top);

	QWidget* button_panel_bot = new QWidget{ this };
	{
		QHBoxLayout* button_layout = new QHBoxLayout{ button_panel_bot };
		button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });

		sel_button = new QPushButton{ "Select", button_panel_bot };
		sel_button->setEnabled(false);
		button_layout->addWidget(sel_button);;
		connect(sel_button, &QPushButton::clicked, this, &ManageApiKeysWindow::pressed_select);

		QPushButton* close_button = new QPushButton{ "Close", button_panel_bot };
		button_layout->addWidget(close_button);
		connect(close_button, &QPushButton::clicked, this, &QWidget::close);
	}
	layout->addWidget(button_panel_bot);

	connect(&(UserProfile::get()), &UserProfile::api_key_list_changed, this, &ManageApiKeysWindow::rebuild_slots);

	rebuild_slots(std::nullopt);
	selection_changed();
}

ManageApiKeysWindow::~ManageApiKeysWindow()
{

}

void ManageApiKeysWindow::double_clicked_profile(QListWidgetItem* const item)
{
	if (item)
	{
		const QVariant data_var = item->data(Qt::UserRole);
		if (variant_is_byte_array(data_var))
		{
			const ApiKeyProfile::Id id{ data_var.toByteArray() };
			UserProfile::get().select_api_key(id);
#ifdef OCT_NEW_GUI
			close();
#else
			if (ApiKeyProfile* profile = UserProfile::get_selected_api_key())
			{
				MyMainWindow* main_window = new MyMainWindow{ nullptr, profile->get_name(), profile->get_key() };
				main_window->show();
				close();
			}
#endif
		}
	}
}

void ManageApiKeysWindow::pressed_add()
{
	AddApiKeyWindow* const add_key_window = new AddApiKeyWindow{ this };
	add_key_window->show();
}

void ManageApiKeysWindow::pressed_edit()
{
	QList<QListWidgetItem*> selected = list_widget->selectedItems();
	if (selected.size() == 1)
	{
		const QVariant data_var = selected.first()->data(Qt::UserRole);
		if (variant_is_byte_array(data_var))
		{
			const ApiKeyProfile::Id id{ data_var.toByteArray() };
			if (UserProfile::get().get_api_key_by_id(id))
			{
				AddApiKeyWindow* add_key_window = new AddApiKeyWindow{ this, id };
				add_key_window->show();
			}
		}
	}
}

void ManageApiKeysWindow::pressed_delete()
{
	const QList<QListWidgetItem*> selected = list_widget->selectedItems();
	if (selected.size() == 1)
	{
		const QVariant selected_data = selected.first()->data(Qt::UserRole);
		if (variant_is_byte_array(selected_data))
		{
			QMessageBox* msg_box = new QMessageBox{ this };
			msg_box->setWindowTitle("Confirm deletion");
			msg_box->setText("Are you sure you want to delete this api key? This cannot be undone.");
			msg_box->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			int result = msg_box->exec();
			if (result == QMessageBox::Yes)
			{
				const ApiKeyProfile::Id id{ selected_data.toByteArray() };
				UserProfile::get().delete_api_key(id);
			}
		}
	}
}

void ManageApiKeysWindow::pressed_select()
{
	const QList<QListWidgetItem*> selected = list_widget->selectedItems();
	if (selected.size() == 1)
	{
		const QVariant selected_data = selected.first()->data(Qt::UserRole);
		if (variant_is_byte_array(selected_data))
		{
			ApiKeyProfile::Id id{ selected_data.toByteArray() };
			UserProfile::get().select_api_key(id);
#ifdef OCT_NEW_GUI
			close();
#else
			if (ApiKeyProfile* details = UserProfile::get_selected_api_key())
			{
				MyMainWindow* main_window = new MyMainWindow{ nullptr, details->get_name(), details->get_key() };
				main_window->show();
				close();
			}
#endif
		}
	}
}

void ManageApiKeysWindow::rebuild_slots(const std::optional<ApiKeyProfile::Id> selected_id)
{
	list_widget->clear();

	QListWidgetItem* selected_item = nullptr;

	for (const ApiKeyProfile* const this_key : UserProfile::get().get_api_key_list())
	{
		QListWidgetItem* const this_item = new QListWidgetItem(list_widget);
		const ApiKeyProfile::Id this_id = this_key->get_id();
		this_item->setData(Qt::UserRole, this_key->get_id().as_q_byte_array());
		this_item->setText(this_key->get_name());
		list_widget->addItem(this_item);
		if (selected_id && *selected_id == this_id)
		{
			selected_item = this_item;
		}
	}
	
	if (selected_item)
	{
		list_widget->setCurrentItem(selected_item);
	}
}

void ManageApiKeysWindow::selection_changed()
{
	const QList<QListWidgetItem*> selected = list_widget->selectedItems();
	if (selected.size() == 1)
	{
		mod_button->setEnabled(true);
		del_button->setEnabled(true);
		sel_button->setEnabled(true);
	}
	else
	{
		mod_button->setEnabled(false);
		del_button->setEnabled(false);
		sel_button->setEnabled(false);
	}
}

AddApiKeyWindow::AddApiKeyWindow(QWidget* const parent, const std::optional<ApiKeyProfile::Id> existing_key_id_in) : QWidget{ parent, Qt::Window }
{
	setAttribute(Qt::WA_DeleteOnClose);

	OCTASSERT(parent != nullptr);
	setWindowModality(Qt::WindowModality::WindowModal);

	const ApiKeyProfile* const existing_key_profile = existing_key_id_in.has_value() ? UserProfile::get().get_api_key_by_id(*existing_key_id_in) : nullptr;
	if (existing_key_profile)
	{
		setWindowTitle("Edit API Key");
		existing_key_id = existing_key_id_in;
	}
	else
	{
		setWindowTitle("Add API Key");
	}

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);

	QWidget* info_panel = new QWidget{ this };
	{
		QLabel* const name_label = new QLabel{ "Name" };
		name_label->setToolTip(ToolTip::AddApiKeyWindow_Name);

		name_edit = new QLineEdit{ info_panel };
		name_edit->setToolTip(ToolTip::AddApiKeyWindow_Name);
		if (existing_key_profile)
		{
			name_edit->setText(existing_key_profile->get_name());
		}
		connect(name_edit, &QLineEdit::textChanged, this, &AddApiKeyWindow::input_changed);

		QLabel* const key_label = new QLabel{ "API Key" };
		key_label->setToolTip(ToolTip::AddApiKeyWindow_ApiKey);

		key_edit = new QLineEdit{ info_panel };
		key_edit->setToolTip(ToolTip::AddApiKeyWindow_ApiKey);
		if (existing_key_profile)
		{
			key_edit->setText(existing_key_profile->get_key());
		}
		connect(key_edit, &QLineEdit::textChanged, this, &AddApiKeyWindow::input_changed);

		production_check = new QCheckBox{ "Production key" };
		production_check->setToolTip(ToolTip::AddApiKeyWindow_IsProduction);
		if (existing_key_profile)
		{
			production_check->setChecked(existing_key_profile->get_production());
		}

		save_to_disk_check = new QCheckBox{ "Save to disk", info_panel };
		save_to_disk_check->setToolTip(ToolTip::AddApiKeyWindow_SaveToDisk);
		if (existing_key_profile)
		{
			save_to_disk_check->setChecked(existing_key_profile->get_save_to_disk());
		}

		QFormLayout* const info_layout = new QFormLayout{ info_panel };
		info_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		info_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		info_layout->addRow(name_label, name_edit);
		info_layout->addRow(key_label, key_edit);
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
		const QString& name = name_edit->text();
		const QString& key = key_edit->text().trimmed();
		const bool production = production_check->isChecked();
		const bool save_to_disk = save_to_disk_check->isChecked();
		std::optional<ApiKeyProfile::Id> new_key_id = UserProfile::get().add_api_key(name, key, production, save_to_disk);
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
	if (input_is_valid() && existing_key_id)
	{
		if (ApiKeyProfile* api_key_profile = UserProfile::get().get_api_key_by_id(*existing_key_id))
		{
			const bool result = api_key_profile->set_details(name_edit->text(), key_edit->text(), production_check->isChecked(), save_to_disk_check->isChecked());
			if (result)
			{
				close();
			}
			else
			{
				QMessageBox* msg_box = new QMessageBox{ this };
				msg_box->setWindowTitle("Error");
				msg_box->setText("Failed to update API key. A key with that name already exists.");
				msg_box->exec();
			}
		}
	}
}
