#include "window_ban_view.h"

#include <memory>
#include <optional>

#include <Qt>
#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "assert.h"
#include "data_request.h"
#include "diag_confirm_change.h"
#include "diag_operation_in_progress.h"
#include "model_common.h"

ViewBanWindow::ViewBanWindow(const ViewEditMode view_edit_mode, const QString& api_key, const BanListUserRestriction& user_restriction, QWidget* const parent) :
	QWidget{ parent, Qt::Window },
	api_key{ api_key }
{
	setAttribute(Qt::WA_DeleteOnClose);

	OCTASSERT(parent != nullptr);
	setWindowModality(Qt::WindowModality::ApplicationModal);

	switch (view_edit_mode)
	{
	case ViewEditMode::Edit:
		setWindowTitle("Edit Ban");
		break;
	case ViewEditMode::View:
		setWindowTitle("View Ban");
		break;
	}

	const BanListGameJoinRestriction& game_join_restriction = user_restriction.get_game_join_restriction();

	QGroupBox* const ban_details_group = new QGroupBox("Ban Details");
	{
		path_edit = new QLineEdit{ ban_details_group };
		path_edit->setReadOnly(true);
		path_edit->setText(user_restriction.get_path());

		QLineEdit* const update_time_edit = new QLineEdit{ ban_details_group };
		update_time_edit->setReadOnly(true);
		if (user_restriction.get_update_time())
		{
			update_time_edit->setText(*user_restriction.get_update_time());
		}

		QLineEdit* const user_edit = new QLineEdit{ ban_details_group };
		user_edit->setReadOnly(true);
		user_edit->setText(user_restriction.get_user());

		QCheckBox* const active_check = new QCheckBox{ ban_details_group };
		active_check->setEnabled(false);
		active_check->setChecked(game_join_restriction.get_active());

		QLineEdit* const start_time_edit = new QLineEdit{ ban_details_group };
		start_time_edit->setReadOnly(true);
		start_time_edit->setText(game_join_restriction.get_start_time());

		QLineEdit* const duration_edit = new QLineEdit{ ban_details_group };
		duration_edit->setReadOnly(true);
		if (game_join_restriction.get_duration())
		{
			duration_edit->setText(*game_join_restriction.get_duration());
		}

		QLineEdit* const private_reason_edit = new QLineEdit{ ban_details_group };
		private_reason_edit->setReadOnly(true);
		private_reason_edit->setText(game_join_restriction.get_private_reason());

		QLineEdit* const display_reason_edit = new QLineEdit{ ban_details_group };
		display_reason_edit->setReadOnly(true);
		display_reason_edit->setText(game_join_restriction.get_display_reason());

		QCheckBox* const exclude_alt_accounts_check = new QCheckBox{ ban_details_group };
		exclude_alt_accounts_check->setEnabled(false);
		exclude_alt_accounts_check->setChecked(game_join_restriction.get_exclude_alt_accounts());

		QCheckBox* const inherited_check = new QCheckBox{ ban_details_group };
		inherited_check->setEnabled(false);
		inherited_check->setChecked(game_join_restriction.get_inherited());

		QFormLayout* const details_layout = new QFormLayout{ ban_details_group };
		details_layout->addRow("Path", path_edit);
		details_layout->addRow("Update Time", update_time_edit);
		details_layout->addRow("User", user_edit);
		details_layout->addRow("Active", active_check);
		details_layout->addRow("Start Time", start_time_edit);
		details_layout->addRow("Duration", duration_edit);
		details_layout->addRow("Private Reason", private_reason_edit);
		details_layout->addRow("Display Reason", display_reason_edit);
		details_layout->addRow("Exclude Alts", exclude_alt_accounts_check);
		details_layout->addRow("Inherited", inherited_check);
	}

	QGroupBox* update_group = nullptr;
	if (view_edit_mode == ViewEditMode::Edit)
	{
		update_group = new QGroupBox{ "Updated", this };

		QWidget* const update_form_widget = new QWidget{ update_group };
		{
			update_active_check = new QCheckBox{ update_form_widget };
			update_active_check->setChecked(game_join_restriction.get_active());

			update_duration_edit = new QLineEdit{ update_form_widget };
			if (game_join_restriction.get_duration())
			{
				update_duration_edit->setText(*game_join_restriction.get_duration());
			}

			update_private_reason_edit = new QLineEdit{ update_form_widget };
			update_private_reason_edit->setText(game_join_restriction.get_private_reason());

			update_display_reason_edit = new QLineEdit{ update_form_widget };
			update_display_reason_edit->setText(game_join_restriction.get_display_reason());

			update_exclude_alt_accounts_check = new QCheckBox{ update_form_widget };
			update_exclude_alt_accounts_check->setChecked(game_join_restriction.get_exclude_alt_accounts());

			QFormLayout* const update_form_layout = new QFormLayout{ update_form_widget };
			update_form_layout->setContentsMargins(0, 0, 0, 0);
			update_form_layout->addRow("Active", update_active_check);
			update_form_layout->addRow("Duration", update_duration_edit);
			update_form_layout->addRow("Private Reason", update_private_reason_edit);
			update_form_layout->addRow("Display Reason", update_display_reason_edit);
			update_form_layout->addRow("Exclude Alts", update_exclude_alt_accounts_check);
		}

		QPushButton* const submit_button = new QPushButton{ "Submit", update_group };
		connect(submit_button, &QPushButton::clicked, this, &ViewBanWindow::pressed_submit);

		QVBoxLayout* const update_group_layout = new QVBoxLayout{ update_group };
		update_group_layout->addWidget(update_form_widget);
		//update_group_layout->addStretch();
		update_group_layout->addWidget(submit_button);
	}

	QHBoxLayout* const layout = new QHBoxLayout{ this };
	layout->addWidget(ban_details_group);
	if (update_group)
	{
		layout->addWidget(update_group);
	}

	if (view_edit_mode == ViewEditMode::Edit)
	{
		setMinimumWidth(820);
	}
	else
	{
		setMinimumWidth(450);
	}
}

void ViewBanWindow::pressed_submit()
{
	const bool active = update_active_check->isChecked();
	const bool has_duration_set = update_duration_edit->text().trimmed().size() > 0;
	const std::optional<QString> duration = has_duration_set ? std::make_optional(update_duration_edit->text()) : std::nullopt;
	const QString private_reason = update_private_reason_edit->text();
	const QString display_reason = update_display_reason_edit->text();
	const bool exclude_alt_accounts = update_exclude_alt_accounts_check->isChecked();
	const BanListGameJoinRestrictionUpdate update{ active, duration, private_reason, display_reason, exclude_alt_accounts };

	if (duration)
	{
		auto show_error_dialog = [this]() {
			QMessageBox* const message_box = new QMessageBox{ this };
			message_box->setWindowTitle("Duration error");
			message_box->setText("Duration must be in the form '{number}s' ex: '600s'");
			message_box->setIcon(QMessageBox::Warning);
			message_box->exec();
		};

		if (duration->size() < 2)
		{
			show_error_dialog();
			return;
		}
		if ((*duration)[duration->size() - 1] != 's')
		{
			show_error_dialog();
			return;
		}

		const QString without_s = duration->left(duration->size() - 1);
		bool valid_number = false;
		without_s.toInt(&valid_number);
		if (!valid_number)
		{
			show_error_dialog();
			return;
		}
	}

	ConfirmChangeDialog* const confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::BanListUpdateRestriction };
	const bool confirmed = static_cast<bool>(confirm_dialog->exec());
	if (!confirmed)
	{
		return;
	}

	const QString path = path_edit->text();
	const std::shared_ptr<UserRestrictionPatchUpdateV2Request> req = std::make_shared<UserRestrictionPatchUpdateV2Request>(api_key, path, update);
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	if (req->req_status() == DataRequestStatus::Success)
	{
		close();
	}
}
