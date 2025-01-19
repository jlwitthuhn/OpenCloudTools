#include "panel_bulk_data.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <Qt>
#include <QtGlobal>
#include <QCheckBox>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "assert.h"
#include "data_request.h"
#include "diag_confirm_change.h"
#include "diag_operation_in_progress.h"
#include "gui_constants.h"
#include "model_common.h"
#include "profile.h"
#include "sqlite_wrapper.h"
#include "tooltip_text.h"
#include "util_alert.h"
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

			datastore_snapshot_button = new QPushButton{ "Make snapshot...", datastore_group };
			datastore_snapshot_button->setToolTip(ToolTip::BulkDataPanel_Snapshot);
			datastore_snapshot_button->setMinimumWidth(150);
			connect(datastore_snapshot_button, &QPushButton::clicked, this, &BulkDataPanel::pressed_snapshot);

			QFrame* separator = new QFrame{ datastore_group };
			separator->setFrameShape(QFrame::HLine);
			separator->setFrameShadow(QFrame::Sunken);

			danger_buttons_check = new QCheckBox{ "Enable danger buttons", datastore_group };
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
			connect(danger_buttons_check, &QCheckBox::checkStateChanged, this, &BulkDataPanel::handle_datastore_danger_toggle);
#else
			connect(danger_buttons_check, &QCheckBox::stateChanged, this, &BulkDataPanel::handle_datastore_danger_toggle);
#endif

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
			group_layout->addWidget(datastore_snapshot_button);
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

#ifdef OCT_NEW_GUI
	// Increase minimum width so the full window title can be seen
	setMinimumWidth(OCT_SUBWINDOW_MIN_WIDTH);
#endif

	change_universe(nullptr);
}

void BulkDataPanel::change_universe(const std::shared_ptr<UniverseProfile>& universe)
{
	if (universe && universe == attached_universe.lock())
	{
		gui_refresh();
		return;
	}
	attached_universe = universe;

	danger_buttons_check->setCheckState(Qt::Unchecked);
	gui_refresh();
}

void BulkDataPanel::gui_refresh()
{
	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		danger_buttons_check->setCheckState(Qt::Unchecked);
		setEnabled(false);
		return;
	}

	setEnabled(true);
	const bool enable_danger = (danger_buttons_check->checkState() == Qt::Checked);
	datastore_delete_button->setEnabled(enable_danger);
	datastore_undelete_button->setEnabled(enable_danger);
	datastore_upload_button->setEnabled(enable_danger);
}

void BulkDataPanel::handle_datastore_danger_toggle()
{
	gui_refresh();
}

void BulkDataPanel::pressed_delete()
{
	const std::shared_ptr<UniverseProfile> universe_profile = attached_universe.lock();
	if (!universe_profile)
	{
		OCTASSERT(false);
		return;
	}

	if (danger_buttons_check->isChecked() == false)
	{
		OCTASSERT(false);
		return;
	}

	const long long universe_id = universe_profile->get_universe_id();

	const auto req = std::make_shared<StandardDatastoreGetListRequest>(api_key, universe_id);
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	const std::vector<QString> datastores = req->get_datastore_names();
	if (datastores.size() == 0)
	{
		return;
	}

	DatastoreBulkDeleteWindow* delete_window = new DatastoreBulkDeleteWindow{ this, api_key, universe_profile, datastores };
	delete_window->show();
}

void BulkDataPanel::pressed_download()
{
	const std::shared_ptr<UniverseProfile> universe_profile = attached_universe.lock();
	if (!universe_profile)
	{
		OCTASSERT(false);
		return;
	}

	const long long universe_id = universe_profile->get_universe_id();

	const auto req = std::make_shared<StandardDatastoreGetListRequest>(api_key, universe_id);
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	const std::vector<QString> datastores = req->get_datastore_names();
	if (datastores.size() == 0)
	{
		return;
	}

	DatastoreBulkDownloadWindow* download_window = new DatastoreBulkDownloadWindow{ this, api_key, universe_profile, datastores };
	download_window->show();
}

void BulkDataPanel::pressed_download_resume()
{
	const std::shared_ptr<UniverseProfile> universe_profile = attached_universe.lock();

	if (!universe_profile)
	{
		OCTASSERT(false);
		return;
	}

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

	const long long universe_id = universe_profile->get_universe_id();

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

	DatastoreBulkDownloadProgressWindow* const progress_window = new DatastoreBulkDownloadProgressWindow{ this, api_key, universe_id, std::move(writer) };
	progress_window->show();
	progress_window->start();
}

void BulkDataPanel::pressed_snapshot()
{
	const std::shared_ptr<UniverseProfile> universe_profile = attached_universe.lock();
	if (!universe_profile)
	{
		OCTASSERT(false);
		return;
	}

	const long long universe_id = universe_profile->get_universe_id();

	QMessageBox* const confirm_message_box = new QMessageBox{ this };
	confirm_message_box->setWindowTitle("Confirm Snapshot");
	confirm_message_box->setText("You can only take a snapshot once per day (UTC).\nDo you want to snapshow now?");
	confirm_message_box->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	const int confirm_status = confirm_message_box->exec();
	if (confirm_status != QMessageBox::Yes)
	{
		return;
	}

	const auto req = std::make_shared<StandardDatastorePostSnapshotRequest>(api_key, universe_id);
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	if (req->req_status() != DataRequestStatus::Success)
	{
		alert_error_blocking("Failed to Snapshot", "Failed to create snapshot.", this);
		return;
	}

	const std::optional<bool> new_snapshot_taken = req->get_new_snapshot_taken();
	std::optional<QString> latest_snapshot_time = req->get_latest_snapshot_time();
	if (!new_snapshot_taken || !latest_snapshot_time)
	{
		alert_error_blocking("Bad Response", "Received HTTP 200 with invalid data. Snapshot may have failed.", this);
		return;
	}

	const QString status_bool = *new_snapshot_taken ? "true" : "false";
	const QString success_message = QString{ "New snapshot taken: %1\nLatest snapshot time: %2" }.arg(status_bool, *latest_snapshot_time);
	QMessageBox* const success_message_box = new QMessageBox{ this };
	success_message_box->setIcon(QMessageBox::Information);
	success_message_box->setWindowTitle("Snapshot Complete");
	success_message_box->setText(success_message);
	success_message_box->setStandardButtons(QMessageBox::Ok);
	success_message_box->exec();
}

void BulkDataPanel::pressed_undelete()
{
	const std::shared_ptr<UniverseProfile> universe_profile = attached_universe.lock();
	if (!universe_profile)
	{
		OCTASSERT(false);
		return;
	}

	const long long universe_id = universe_profile->get_universe_id();

	const auto req = std::make_shared<StandardDatastoreGetListRequest>(api_key, universe_id);
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	const std::vector<QString> datastores = req->get_datastore_names();
	if (datastores.size() == 0)
	{
		return;
	}

	DatastoreBulkUndeleteWindow* const undelete_window = new DatastoreBulkUndeleteWindow{ this, api_key, universe_profile, datastores };
	undelete_window->show();
}

void BulkDataPanel::pressed_upload()
{
	const std::shared_ptr<UniverseProfile> universe_profile = attached_universe.lock();
	if (!universe_profile)
	{
		OCTASSERT(false);
		return;
	}

	if (danger_buttons_check->isChecked() == false)
	{
		OCTASSERT(false);
		return;
	}

	ConfirmChangeDialog* confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::StandardDatastoreBulkUpload };
	bool confirmed = static_cast<bool>(confirm_dialog->exec());
	if (confirmed == false)
	{
		return;
	}

	const QString load_file_path = QFileDialog::getOpenFileName(this, "Select dump to upload...", "", "sqlite3 databases (*.sqlite3)");
	if (load_file_path.trimmed().size() == 0)
	{
		return;
	}

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
