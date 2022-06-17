#include "window_datastore_universe_add.h"

#include <cstddef>

#include <memory>
#include <optional>

#include <Qt>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QLineEdit>
#include <QMargins>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include "api_key.h"
#include "user_settings.h"

AddUniverseToDatastoreWindow::AddUniverseToDatastoreWindow(QWidget* const parent, const bool edit_current) : QWidget{ parent, Qt::Window }
{
	std::optional<UniverseProfile> existing_universe = edit_current ? UserSettings::get()->get_selected_universe() : std::nullopt;
	edit_mode = existing_universe.has_value();

	if (edit_mode)
	{
		setWindowTitle("Edit universe");
	}
	else
	{
		setWindowTitle("Add universe");
	}

	QWidget* info_panel = new QWidget{ this };
	{
		name_edit = new QLineEdit{ info_panel };
		if (edit_mode)
		{
			name_edit->setText(existing_universe->name());
		}
		connect(name_edit, &QLineEdit::textChanged, this, &AddUniverseToDatastoreWindow::text_changed);

		id_edit = new QLineEdit{ info_panel };
		if (edit_mode)
		{
			id_edit->setText(QString::number(existing_universe->universe_id()));
		}
		connect(id_edit, &QLineEdit::textChanged, this, &AddUniverseToDatastoreWindow::text_changed);

		QFormLayout* info_layout = new QFormLayout{ info_panel };
		info_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
        info_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		info_layout->addRow("Name", name_edit);
		info_layout->addRow("Universe ID", id_edit);
	}

	QWidget* button_panel = new QWidget{ this };
	button_panel->setMinimumWidth(280);
	{
		add_button = new QPushButton{ "Add", button_panel };
		if (edit_mode)
		{
			add_button->setText("Update");
		}
		connect(add_button, &QPushButton::clicked, this, &AddUniverseToDatastoreWindow::pressed_add);

		QPushButton* cancel_button = new QPushButton{ "Cancel", button_panel };

		QHBoxLayout* button_layout = new QHBoxLayout{ button_panel };
		button_layout->addWidget(add_button);
		button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		button_layout->addWidget(cancel_button);
		connect(cancel_button, &QPushButton::clicked, this, &QWidget::close);
	}

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
	layout->addWidget(info_panel);
	layout->addWidget(button_panel);

	text_changed();
}

bool AddUniverseToDatastoreWindow::input_is_valid() const
{
	const long long universe_id = id_edit->text().trimmed().toLongLong();
	return name_edit->text().size() > 0 && universe_id > 100000L;
}

void AddUniverseToDatastoreWindow::text_changed()
{
	add_button->setEnabled(input_is_valid());
}

void AddUniverseToDatastoreWindow::pressed_add()
{
	const QString name = name_edit->text();
	const long long universe_id = id_edit->text().trimmed().toLongLong();
	if (edit_mode)
	{
		if (UserSettings::get()->update_selected_universe(name, universe_id))
		{
			close();
		}
		else
		{
			QMessageBox* msg_box = new QMessageBox{ this };
			msg_box->setWindowTitle("Update Failed");
			msg_box->setText("Failed to update universe. A universe with that name and id already exists.");
			msg_box->exec();
		}
	}
	else
	{
		const std::optional<size_t> new_id = UserSettings::get()->selected_profile_add_universe(UniverseProfile{ name, universe_id });
		if (new_id)
		{
			close();
		}
		else
		{
			QMessageBox* msg_box = new QMessageBox{ this };
			msg_box->setWindowTitle("Add Failed");
			msg_box->setText("Failed to add new universe. A universe with that name and id already exists.");
			msg_box->exec();
		}
	}
}
