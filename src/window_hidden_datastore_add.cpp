#include "window_hidden_datastore_add.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "user_settings.h"

AddHiddenDatastoreWindow::AddHiddenDatastoreWindow(QWidget* parent) : QWidget{ parent, Qt::Window }
{
	setWindowTitle("Hide datastore");
	setWindowModality(Qt::WindowModality::ApplicationModal);

	QWidget* info_panel = new QWidget{ this };
	{
		name_edit = new QLineEdit{ info_panel };
		connect(name_edit, &QLineEdit::textChanged, this, &AddHiddenDatastoreWindow::handle_text_changed);

		QFormLayout* info_layout = new QFormLayout{ info_panel };
		info_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		info_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		info_layout->addRow("Datastore name", name_edit);
	}

	QWidget* button_panel = new QWidget{ this };
	button_panel->setMinimumWidth(280);
	{
		add_button = new QPushButton{ "Add", button_panel };
		connect(add_button, &QPushButton::clicked, this, &AddHiddenDatastoreWindow::pressed_add);

		QPushButton* cancel_button = new QPushButton{ "Cancel", button_panel };
		connect(add_button, &QPushButton::clicked, this, &AddHiddenDatastoreWindow::close);

		QHBoxLayout* button_layout = new QHBoxLayout{ button_panel };
		button_layout->addWidget(add_button);
		button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		button_layout->addWidget(cancel_button);
		connect(cancel_button, &QPushButton::clicked, this, &QWidget::close);
	}

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->addWidget(info_panel);
	layout->addWidget(button_panel);

	handle_text_changed();
}

void AddHiddenDatastoreWindow::handle_text_changed()
{
	add_button->setEnabled(name_edit->text().size() > 0);
}

void AddHiddenDatastoreWindow::pressed_add()
{
	UserSettings::get()->add_ignored_datastore(name_edit->text());
	close();
}
