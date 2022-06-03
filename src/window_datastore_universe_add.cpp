#include "window_datastore_universe_add.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "user_settings.h"
#include "window_datastore_explore.h"

AddUniverseToDatastoreWindow::AddUniverseToDatastoreWindow(QWidget* const parent) : QWidget{ parent, Qt::Window }
{
	setWindowTitle("Add universe");

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);

	QWidget* info_panel = new QWidget{ this };
	{
		name_edit = new QLineEdit{ info_panel };
		connect(name_edit, &QLineEdit::textChanged, this, &AddUniverseToDatastoreWindow::text_changed);

		id_edit = new QLineEdit{ info_panel };
		connect(id_edit, &QLineEdit::textChanged, this, &AddUniverseToDatastoreWindow::text_changed);

		QFormLayout* info_layout = new QFormLayout{ info_panel };
		info_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
        info_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		info_layout->addRow("Name", name_edit);
		info_layout->addRow("Universe ID", id_edit);
	}
	layout->addWidget(info_panel);

	QWidget* button_panel = new QWidget{ this };
	button_panel->setMinimumWidth(280);
	{
		QHBoxLayout* button_layout = new QHBoxLayout{ button_panel };

		add_button = new QPushButton{ "Add", button_panel };
		button_layout->addWidget(add_button);
		connect(add_button, &QPushButton::clicked, this, &AddUniverseToDatastoreWindow::pressed_add);

		QPushButton* cancel_button = new QPushButton{ "Cancel", button_panel };
		button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		button_layout->addWidget(cancel_button);
		connect(cancel_button, &QPushButton::clicked, this, &QWidget::close);
	}
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
	const long long universe_id = id_edit->text().trimmed().toLongLong();
	const QString name = name_edit->text();
	UserSettings::get()->selected_add_universe(universe_id, name);
	close();
}
