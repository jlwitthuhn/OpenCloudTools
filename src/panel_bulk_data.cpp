#include "panel_bulk_data.h"

#include <memory>
#include <optional>
#include <vector>

#include <Qt>
#include <QCheckBox>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#include "api_key.h"
#include "data_request.h"
#include "diag_operation_in_progress.h"
#include "user_settings.h"
#include "window_datastore_bulk_op.h"

BulkDataPanel::BulkDataPanel(QWidget* const parent, const QString& api_key) :
	QWidget{ parent },
	api_key{ api_key }
{
	QWidget* container_widget = new QWidget{ this };
	{
		QGroupBox* datastore_group = new QGroupBox{ "Datastore", container_widget};
		{
			datastore_download_button = new QPushButton{ "Bulk download...", datastore_group };
			datastore_download_button->setMinimumWidth(150);
			connect(datastore_download_button, &QPushButton::clicked, this, &BulkDataPanel::pressed_download);

			QFrame* separator = new QFrame{ datastore_group };
			separator->setFrameShape(QFrame::HLine);
			separator->setFrameShadow(QFrame::Sunken);

			danger_buttons_check = new QCheckBox{ "Enable danger buttons", datastore_group };
			connect(danger_buttons_check, &QCheckBox::stateChanged, this, &BulkDataPanel::handle_datastore_danger_toggle);

			datastore_delete_button = new QPushButton{ "Bulk delete...", datastore_group };
			datastore_delete_button->setMinimumWidth(150);
			connect(datastore_delete_button, &QPushButton::clicked, this, &BulkDataPanel::pressed_delete);

			QVBoxLayout* group_layout = new QVBoxLayout{ datastore_group };
			group_layout->addWidget(datastore_download_button);
			group_layout->addWidget(separator);
			group_layout->addWidget(danger_buttons_check);
			group_layout->addWidget(datastore_delete_button);
		}

		QHBoxLayout* container_layout = new QHBoxLayout{ container_widget };
		container_layout->addStretch();
		container_layout->addWidget(datastore_group);
		container_layout->addStretch();
	}

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->addStretch();
	layout->addWidget(container_widget);
	layout->addStretch();

	handle_datastore_danger_toggle();
}

void BulkDataPanel::selected_universe_changed()
{
	datastore_download_button->setEnabled(UserSettings::get()->get_selected_universe().has_value());
}

void BulkDataPanel::handle_datastore_danger_toggle()
{
	datastore_delete_button->setEnabled(danger_buttons_check->isChecked());
}

void BulkDataPanel::pressed_delete()
{
	if (UserSettings::get()->get_selected_universe())
	{
		const long long universe_id = UserSettings::get()->get_selected_universe()->universe_id();

		GetStandardDatastoresDataRequest req{ nullptr, api_key, universe_id };
		OperationInProgressDialog diag{ this, &req };
		req.send_request();
		diag.exec();

		const std::vector<QString> datastores = req.get_datastore_names();
		if (datastores.size() > 0)
		{
			DatastoreBulkDeleteWindow* delete_window = new DatastoreBulkDeleteWindow{ this, api_key, universe_id, datastores };
			delete_window->setWindowModality(Qt::WindowModality::ApplicationModal);
			delete_window->show();
		}
	}
}

void BulkDataPanel::pressed_download()
{
	if (UserSettings::get()->get_selected_universe())
	{
		const long long universe_id = UserSettings::get()->get_selected_universe()->universe_id();

		GetStandardDatastoresDataRequest req{ nullptr, api_key, universe_id };
		OperationInProgressDialog diag{ this, &req };
		req.send_request();
		diag.exec();

		const std::vector<QString> datastores = req.get_datastore_names();
		if (datastores.size() > 0)
		{
			DatastoreBulkDownloadWindow* download_window = new DatastoreBulkDownloadWindow{ this, api_key, universe_id, datastores };
			download_window->setWindowModality(Qt::WindowModality::ApplicationModal);
			download_window->show();
		}
	}
}
