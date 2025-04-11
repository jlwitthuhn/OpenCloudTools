#include "panel_ban_list_add.h"

#include <optional>

#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "assert.h"
#include "data_request.h"
#include "diag_operation_in_progress.h"
#include "model_common.h"
#include "profile.h"

BanAddPanel::BanAddPanel(QWidget* const parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe) :
	QWidget{ parent },
	api_key{ api_key },
	attached_universe{ universe }
{
	setMinimumWidth(300);

	QWidget* const form_widget = new QWidget{ this };
	{
		// TODO: Add numeric validator here
		user_id_edit = new QLineEdit{ form_widget };
		duration_edit = new QLineEdit{ form_widget };
		private_reason_edit = new QLineEdit{ form_widget };
		display_reason_edit = new QLineEdit{ form_widget };
		exclude_alts_check = new QCheckBox{ form_widget };

		QFormLayout* const form_layout = new QFormLayout{ form_widget };
		form_layout->setContentsMargins(0, 0, 0, 0);
		form_layout->addRow("User ID", user_id_edit);
		form_layout->addRow("Duration", duration_edit);
		form_layout->addRow("Private Reason", private_reason_edit);
		form_layout->addRow("Display Reason", display_reason_edit);
		form_layout->addRow("Exclude Alts", exclude_alts_check);
	}

	QPushButton* const ban_button = new QPushButton{ "Ban", this };
	connect(ban_button, &QPushButton::clicked, this, &BanAddPanel::pressed_ban);

	QVBoxLayout* const layout = new QVBoxLayout{ this };
	layout->addWidget(form_widget);
	layout->addWidget(ban_button);
}

void BanAddPanel::pressed_ban()
{
	const auto universe = attached_universe.lock();
	OCTASSERT(universe);
	const QString path = QString{ "universes/" } + QString::number(universe->get_universe_id()) + "/user-restrictions/" + user_id_edit->text();
	const bool has_duration = duration_edit->text().trimmed().size() > 0;
	const std::optional<QString> duration = has_duration ? std::make_optional(duration_edit->text()) : std::nullopt;
	const QString private_reason = private_reason_edit->text();
	const QString display_reason = display_reason_edit->text();
	const bool exclude_alts = exclude_alts_check->isChecked();

	const BanListGameJoinRestrictionUpdate update{ true, duration, private_reason, display_reason, exclude_alts };

	const auto req = std::make_shared<UserRestrictionPatchUpdateV2Request>(api_key, path, update);
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	if (req->req_success())
	{
		duration_edit->setText("");
		private_reason_edit->setText("");
		display_reason_edit->setText("");
		exclude_alts_check->setChecked(false);
	}
}
