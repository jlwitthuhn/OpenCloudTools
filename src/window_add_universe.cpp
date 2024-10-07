#include "window_add_universe.h"

#include <optional>

#include <Qt>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QLineEdit>
#include <QMargins>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "assert.h"
#include "data_request.h"
#include "diag_operation_in_progress.h"
#include "profile.h"

AddUniverseWindow::AddUniverseWindow(QWidget* const parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& existing_universe) :
	QWidget{ parent, Qt::Window },
	api_key{ api_key },
	attached_universe{ existing_universe }
{
	setAttribute(Qt::WA_DeleteOnClose);

	OCTASSERT(parent != nullptr);
	setWindowModality(Qt::WindowModality::ApplicationModal);

	if (existing_universe)
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
		if (existing_universe)
		{
			name_edit->setText(existing_universe->get_name());
		}
		connect(name_edit, &QLineEdit::textChanged, this, &AddUniverseWindow::text_changed);

		id_edit = new QLineEdit{ info_panel };
		if (existing_universe)
		{
			id_edit->setText(QString::number(existing_universe->get_universe_id()));
		}
		connect(id_edit, &QLineEdit::textChanged, this, &AddUniverseWindow::text_changed);

		QFormLayout* info_layout = new QFormLayout{ info_panel };
		info_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		info_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		info_layout->addRow("Name", name_edit);
		info_layout->addRow("Universe ID", id_edit);
	}

	QWidget* const button_panel = new QWidget{ this };
	button_panel->setMinimumWidth(280);
	{
		fetch_name_button = new QPushButton{ "Fetch Name", button_panel };
		connect(fetch_name_button, &QPushButton::clicked, this, &AddUniverseWindow::pressed_fetch);

		add_button = new QPushButton{ "Add", button_panel };
		if (existing_universe)
		{
			add_button->setText("Update");
		}
		connect(add_button, &QPushButton::clicked, this, &AddUniverseWindow::pressed_add);

		QPushButton* const cancel_button = new QPushButton{ "Cancel", button_panel };
		connect(cancel_button, &QPushButton::clicked, this, &QWidget::close);

		QHBoxLayout* const button_layout = new QHBoxLayout{ button_panel };
		button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		button_layout->addWidget(fetch_name_button);
		button_layout->addWidget(add_button);
		button_layout->addWidget(cancel_button);
	}

	QVBoxLayout* const layout = new QVBoxLayout{ this };
	layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
	layout->addWidget(info_panel);
	layout->addWidget(button_panel);

	text_changed();
}

bool AddUniverseWindow::id_is_valid() const
{
	return id_edit->text().trimmed().toLongLong() > 100000L;
}

bool AddUniverseWindow::name_is_valid() const
{
	return name_edit->text().size() > 0;
}

void AddUniverseWindow::text_changed()
{
	fetch_name_button->setEnabled(id_is_valid());
	add_button->setEnabled(id_is_valid() && name_is_valid());
}

void AddUniverseWindow::pressed_add()
{
	if (id_is_valid() && name_is_valid())
	{
		const QString name = name_edit->text();
		const long long universe_id = id_edit->text().trimmed().toLongLong();
		if (const std::shared_ptr<UniverseProfile> universe = attached_universe.lock())
		{
			if (universe->set_details(name, universe_id))
			{
				close();
			}
			else
			{
				QMessageBox* msg_box = new QMessageBox{ this };
				msg_box->setWindowTitle("Update Failed");
				msg_box->setText("Failed to update universe. A universe with that name or id already exists.");
				msg_box->exec();
			}
		}
		else
		{
			const std::optional<UniverseProfile::Id> new_id = UserProfile::get_active_api_key()->add_universe(name, universe_id);
			if (new_id)
			{
				close();
			}
			else
			{
				QMessageBox* msg_box = new QMessageBox{ this };
				msg_box->setWindowTitle("Add Failed");
				msg_box->setText("Failed to add new universe. A universe with that name or id already exists.");
				msg_box->exec();
			}
		}
	}
}

void AddUniverseWindow::pressed_fetch()
{
	if (id_is_valid())
	{
		const long long universe_id = id_edit->text().trimmed().toLongLong();
		const auto req = std::make_shared<UniverseGetDetailsRequest>(api_key, universe_id);
		OperationInProgressDialog diag{ this, req };
		diag.exec();

		const std::optional<QString> display_name = req->get_display_name();
		if (display_name)
		{
			name_edit->setText(*display_name);
		}
	}
}
