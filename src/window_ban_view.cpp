#include "window_ban_view.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>

#include "assert.h"
#include "model_common.h"

ViewBanWindow::ViewBanWindow(const BanListUserRestriction& user_restriction, const ViewEditMode view_edit_mode, QWidget* const parent) : QWidget{ parent, Qt::Window }
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

	QLineEdit* const path_edit = new QLineEdit{ this };
	path_edit->setReadOnly(true);
	path_edit->setText(user_restriction.get_path());

	QLineEdit* const update_time_edit = new QLineEdit{ this };
	update_time_edit->setReadOnly(true);
	if (user_restriction.get_update_time())
	{
		update_time_edit->setText(*user_restriction.get_update_time());
	}

	QLineEdit* const user_edit = new QLineEdit{ this };
	user_edit->setReadOnly(true);
	user_edit->setText(user_restriction.get_user());

	const BanListGameJoinRestriction& game_join_restriction = user_restriction.get_game_join_restriction();

	QCheckBox* const active_check = new QCheckBox{ this };
	active_check->setEnabled(false);
	active_check->setChecked(game_join_restriction.get_active());

	QLineEdit* const start_time_edit = new QLineEdit{ this };
	start_time_edit->setReadOnly(true);
	start_time_edit->setText(game_join_restriction.get_start_time());

	QLineEdit* const duration_edit = new QLineEdit{ this };
	duration_edit->setReadOnly(true);
	if (game_join_restriction.get_duration())
	{
		duration_edit->setText(*game_join_restriction.get_duration());
	}

	QLineEdit* const private_reason_edit = new QLineEdit{ this };
	private_reason_edit->setReadOnly(true);
	private_reason_edit->setText(game_join_restriction.get_private_reason());

	QLineEdit* const display_reason_edit = new QLineEdit{ this };
	display_reason_edit->setReadOnly(true);
	display_reason_edit->setText(game_join_restriction.get_display_reason());

	QCheckBox* const exclude_alt_accounts_check = new QCheckBox{ this };
	exclude_alt_accounts_check->setEnabled(false);
	exclude_alt_accounts_check->setChecked(game_join_restriction.get_exclude_alt_accounts());

	QCheckBox* const inherited_check = new QCheckBox{ this };
	inherited_check->setEnabled(false);
	inherited_check->setChecked(game_join_restriction.get_inherited());

	QFormLayout* const layout = new QFormLayout{ this };
	layout->addRow("Path", path_edit);
	layout->addRow("Update Time", update_time_edit);
	layout->addRow("User", user_edit);
	layout->addRow("Active", active_check);
	layout->addRow("Start Time", start_time_edit);
	layout->addRow("Duration", duration_edit);
	layout->addRow("Private Reason", private_reason_edit);
	layout->addRow("Display Reason", display_reason_edit);
	layout->addRow("Exclude Alts", exclude_alt_accounts_check);
	layout->addRow("Inherited", inherited_check);

	setMinimumWidth(400);
}
