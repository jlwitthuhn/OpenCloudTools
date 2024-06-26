#include "panel_bulk_data.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <Qt>
#include <QCheckBox>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "data_request.h"
#include "diag_confirm_change.h"
#include "diag_operation_in_progress.h"
#include "model_common.h"
#include "profile.h"
#include "sqlite_wrapper.h"
#include "tooltip_text.h"
#include "window_datastore_bulk_op.h"
#include "window_datastore_bulk_op_progress.h"

BulkDataPanel::BulkDataPanel(QWidget* const parent, const QString& api_key) :
	QWidget{ parent },
	api_key{ api_key }
{
	QWidget* container_widget = new QWidget{ this };
	{
		QGroupBox* datastore_group = new QGroupBox{ "Datastore", container_widget};
		{
			datastore_download_button = new QPushButton{ "Bulk download...", datastore_group };
			datastore_download_button->setToolTip(ToolTip::BulkDataPanel_Download);
			datastore_download_button->setMinimumWidth(150);
			connect(datastore_download_button, &QPushButton::clicked, this, &BulkDataPanel::pressed_download);

			datastore_download_resume_button = new QPushButton{ "Resume download...", datastore_group };
			datastore_download_resume_button->setToolTip(ToolTip::BulkDataPanel_ResumeDownload);
			datastore_download_resume_button->setMinimumWidth(150);
			connect(datastore_download_resume_button, &QPushButton::clicked, this, &BulkDataPanel::pressed_download_resume);

			QFrame* separator = new QFrame{ datastore_group };
			separator->setFrameShape(QFrame::HLine);
			separator->setFrameShadow(QFrame::Sunken);

			danger_buttons_check = new QCheckBox{ "Enable danger buttons", datastore_group };
			connect(danger_buttons_check, &QCheckBox::stateChanged, this, &BulkDataPanel::handle_datastore_danger_toggle);

			datastore_delete_button = new QPushButton{ "Bulk delete...", datastore_group };
			datastore_delete_button->setToolTip(ToolTip::BulkDataPanel_Delete);
			datastore_delete_button->setMinimumWidth(150);
			connect(datastore_delete_button, &QPushButton::clicked, this, &BulkDataPanel::pressed_delete);

			datastore_undelete_button = new QPushButton{ "Bulk undelete...", datastore_group };
			datastore_undelete_button->setToolTip(ToolTip::BulkDataPanel_Undelete);
			datastore_undelete_button->setMinimumWidth(150);
			connect(datastore_undelete_button, &QPushButton::clicked, this, &BulkDataPanel::pressed_undelete);

			datastore_upload_button = new QPushButton{ "Bulk upload...", datastore_group };
			datastore_upload_button->setToolTip(ToolTip::BulkDataPanel_Upload);
			datastore_upload_button->setMinimumWidth(150);
			connect(datastore_upload_button, &QPushButton::clicked, this, &BulkDataPanel::pressed_upload);

			QVBoxLayout* group_layout = new QVBoxLayout{ datastore_group };
			group_layout->addWidget(datastore_download_button);
			group_layout->addWidget(datastore_download_resume_button);
			group_layout->addWidget(separator);
			group_layout->addWidget(danger_buttons_check);
			group_layout->addWidget(datastore_delete_button);
			group_layout->addWidget(datastore_undelete_button);
			group_layout->addWidget(datastore_upload_button);
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

	selected_universe_changed();
	handle_datastore_danger_toggle();
}

void BulkDataPanel::selected_universe_changed()
{
	const bool enabled = UserProfile::get_selected_universe() != nullptr;
	datastore_download_button->setEnabled(enabled);
	danger_buttons_check->setEnabled(enabled);
	danger_buttons_check->setCheckState(Qt::Unchecked);
	handle_datastore_danger_toggle();
}

void BulkDataPanel::handle_datastore_danger_toggle()
{
	const bool enabled = danger_buttons_check->isEnabled() && danger_buttons_check->isChecked();
	datastore_delete_button->setEnabled(enabled);
	datastore_undelete_button->setEnabled(enabled);
	datastore_upload_button->setEnabled(enabled);
}

void BulkDataPanel::pressed_delete()
{
	if (UserProfile::get_selected_universe() && danger_buttons_check->isChecked())
	{
		const long long universe_id = UserProfile::get_selected_universe()->get_universe_id();

		const auto req = std::make_shared<StandardDatastoreGetListRequest>(api_key, universe_id);
		OperationInProgressDialog diag{ this, req };
		diag.exec();

		const std::vector<QString> datastores = req->get_datastore_names();
		if (datastores.size() > 0)
		{
			DatastoreBulkDeleteWindow* delete_window = new DatastoreBulkDeleteWindow{ this, api_key, universe_id, datastores };
			delete_window->show();
		}
	}
}

void BulkDataPanel::pressed_download()
{
	if (UserProfile::get_selected_universe())
	{
		const long long universe_id = UserProfile::get_selected_universe()->get_universe_id();

		const auto req = std::make_shared<StandardDatastoreGetListRequest>(api_key, universe_id);
		OperationInProgressDialog diag{ this, req };
		diag.exec();

		const std::vector<QString> datastores = req->get_datastore_names();
		if (datastores.size() > 0)
		{
			DatastoreBulkDownloadWindow* download_window = new DatastoreBulkDownloadWindow{ this, api_key, universe_id, datastores };
			download_window->show();
		}
	}
}

void BulkDataPanel::pressed_download_resume()
{
	if (UserProfile::get_selected_universe())
	{
		const QString file_name = QFileDialog::getOpenFileName(this, "Resume download", "", "sqlite3 databases (*.sqlite3)");
		if (file_name.trimmed().length() == 0)
		{
			// User selected nothing, return silently
			return;
		}

		{
			const QFile existing_file{ file_name };
			if (existing_file.exists() == false)
			{
				QMessageBox::critical(nullptr, "Error", "Unable to open file");
				return;
			}
		}

		const long long universe_id = UserProfile::get_selected_universe()->get_universe_id();

		std::unique_ptr<SqliteDatastoreWrapper> writer = SqliteDatastoreWrapper::open_from_path(file_name.toStdString());
		if (writer->is_correct_schema() == false)
		{
			QMessageBox::critical(nullptr, "Error", "Selected file has unexpected database schema, unable to proceed");
			return;
		}
		if (writer->is_resumable(universe_id) == false)
		{
			QMessageBox::critical(nullptr, "Error", "Selected file cannot be resumed for the selected universe");
			return;
		}

		DatastoreBulkDownloadProgressWindow* progress_window = new DatastoreBulkDownloadProgressWindow{ this, api_key, universe_id, std::move(writer) };
		progress_window->show();
		progress_window->start();
	}
}

void BulkDataPanel::pressed_undelete()
{
	if (UserProfile::get_selected_universe() && danger_buttons_check->isChecked())
	{
		const long long universe_id = UserProfile::get_selected_universe()->get_universe_id();

		const auto req = std::make_shared<StandardDatastoreGetListRequest>(api_key, universe_id);
		OperationInProgressDialog diag{ this, req };
		diag.exec();

		const std::vector<QString> datastores = req->get_datastore_names();
		if (datastores.size() > 0)
		{
			DatastoreBulkUndeleteWindow* undelete_window = new DatastoreBulkUndeleteWindow{ this, api_key, universe_id, datastores };
			undelete_window->show();
		}
	}
}

void BulkDataPanel::pressed_upload()
{
	if (UserProfile::get_selected_universe() && danger_buttons_check->isChecked())
	{
		ConfirmChangeDialog* confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::StandardDatastoreBulkUpload };
		bool confirmed = static_cast<bool>(confirm_dialog->exec());
		if (confirmed)
		{
			const QString load_file_path = QFileDialog::getOpenFileName(this, "Select dump to upload...", "", "sqlite3 databases (*.sqlite3)");
			if (load_file_path.trimmed().size() > 0)
			{
				std::optional<std::vector<StandardDatastoreEntryFull>> loaded_data = SqliteDatastoreReader::read_all(load_file_path.toStdString());
				if (loaded_data)
				{
					std::vector<std::shared_ptr<DataRequest>> shared_requests;

					for (const StandardDatastoreEntryFull& this_entry : *loaded_data)
					{
						shared_requests.push_back(std::make_shared<StandardDatastoreEntryPostSetRequest>(
							api_key,
							this_entry.get_universe_id(),
							this_entry.get_datastore_name(),
							this_entry.get_scope(),
							this_entry.get_key_name(),
							this_entry.get_userids(),
							this_entry.get_attributes(),
							this_entry.get_data_raw()
						));
					}

					OperationInProgressDialog diag{ this, shared_requests };
					diag.exec();
				}
				else
				{
					QMessageBox* msg_box = new QMessageBox{ this };
					msg_box->setWindowTitle("Error");
					msg_box->setText("Failed to open database.");
					msg_box->exec();
				}
			}
		}
	}
}
