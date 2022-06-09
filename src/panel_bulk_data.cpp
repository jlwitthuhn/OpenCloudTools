#include "panel_bulk_data.h"

#include <vector>

#include <Qt>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "data_request.h"
#include "diag_operation_in_progress.h"
#include "window_datastore_download.h"

BulkDataPanel::BulkDataPanel(QWidget* const parent, const QString& api_key, const std::optional<long long> selected_universe_id) :
	QWidget{ parent },
	api_key{ api_key },
	selected_universe_id{ selected_universe_id }
{
	QWidget* container_widget = new QWidget{ this };
	{
		QGroupBox* datastore_group = new QGroupBox{ "Datastore", container_widget};
		{
			datastore_download_button = new QPushButton{ "Bulk download...", datastore_group };
			datastore_download_button->setMinimumWidth(150);
			connect(datastore_download_button, &QPushButton::clicked, this, &BulkDataPanel::pressed_download);

			QVBoxLayout* group_layout = new QVBoxLayout{ datastore_group };
			group_layout->addWidget(datastore_download_button);
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
}

void BulkDataPanel::selected_universe_changed(std::optional<long long> new_universe)
{
	selected_universe_id = new_universe;
	datastore_download_button->setEnabled(selected_universe_id.has_value());
}

void BulkDataPanel::pressed_download()
{
	if (selected_universe_id)
	{
		QMessageBox* msg_box = new QMessageBox{ this };
		msg_box->setWindowTitle("Alert");
		msg_box->setText("The Bulk Download feature may change in the near future. The format of the sqlite database is not final, so don't depend on it too much.");
		msg_box->exec();

		const long long universe_id = *selected_universe_id;

		GetStandardDatastoresDataRequest req{ nullptr, api_key, universe_id };
		OperationInProgressDialog diag{ this, &req };
		req.send_request();
		diag.exec();

		const std::vector<QString> datastores = req.get_datastore_names();
		if (datastores.size() > 0)
		{
			DownloadDatastoreWindow* download_window = new DownloadDatastoreWindow{ this, api_key, universe_id, datastores };
			download_window->setWindowModality(Qt::WindowModality::ApplicationModal);
			download_window->show();
		}
	}
}
