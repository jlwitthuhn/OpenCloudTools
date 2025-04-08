#include "panel_ban_list.h"

#include <optional>
#include <vector>

#include <QAbstractItemModel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>

#include "assert.h"
#include "data_request.h"
#include "diag_confirm_change.h"
#include "diag_operation_in_progress.h"
#include "model_common.h"
#include "model_qt.h"
#include "profile.h"
#include "util_enum.h"
#include "window_ban_view.h"

BanListPanel::BanListPanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe) :
	QWidget{ parent },
	api_key{ api_key },
	attached_universe{ universe }
{
	setMinimumWidth(450);

	QWidget* const top_bar = new QWidget{ this };
	{
		QPushButton* const refresh_button = new QPushButton{ "Refresh", top_bar };
		connect(refresh_button, &QPushButton::clicked, this, &BanListPanel::pressed_refresh);

		active_only_check = new QCheckBox{ "Active bans only", top_bar };
		active_only_check->setChecked(true);

		QHBoxLayout* const top_layout = new QHBoxLayout{ top_bar };
		top_layout->setContentsMargins(0, 0, 0, 0);
		top_layout->addWidget(refresh_button);
		top_layout->addWidget(active_only_check);
		top_layout->addStretch();
	}

	tree_view = new QTreeView{ this };
	tree_view->setMinimumSize(400, 200);

	QWidget* const button_bar = new QWidget{ this };
	{
		unban_button = new QPushButton{ "Unban...", button_bar };
		connect(unban_button, &QPushButton::clicked, this, &BanListPanel::pressed_unban);

		edit_button = new QPushButton{ "Edit...", button_bar };
		connect(edit_button, &QPushButton::clicked, this, &BanListPanel::pressed_edit);

		details_button = new QPushButton{ "Details...", button_bar };
		connect(details_button, &QPushButton::clicked, this, &BanListPanel::pressed_details);

		QHBoxLayout* const button_layout = new QHBoxLayout{ button_bar };
		button_layout->setContentsMargins(0, 0, 0, 0);
		button_layout->addStretch();
		button_layout->addWidget(unban_button);
		button_layout->addWidget(edit_button);
		button_layout->addWidget(details_button);
	}

	QVBoxLayout* const layout = new QVBoxLayout{ this };
	layout->addWidget(top_bar);
	layout->addWidget(tree_view);
	layout->addWidget(button_bar);

	set_table_model(nullptr);
	gui_refresh();
}

void BanListPanel::gui_refresh()
{
	const bool enabled = get_selected_single_index().isValid();
	unban_button->setEnabled(enabled);
	details_button->setEnabled(enabled);
	edit_button->setEnabled(enabled);
}

QModelIndex BanListPanel::get_selected_single_index() const
{
	const QItemSelectionModel* const select_model = tree_view->selectionModel();
	if (select_model == nullptr)
	{
		return QModelIndex{};
	}

	if (select_model->selectedRows().count() != 1)
	{
		return QModelIndex{};
	}

	return select_model->selectedRows().front();
}

std::optional<BanListUserRestriction> BanListPanel::get_selected_restriction() const
{
	const QModelIndex selected_index = get_selected_single_index();

	if (selected_index.isValid() == false)
	{
		return std::nullopt;
	}

	BanListQTableModel* const table_model = dynamic_cast<BanListQTableModel*>(tree_view->model());
	if (table_model == nullptr)
	{
		return std::nullopt;
	}

	return table_model->get_restriction(selected_index.row());
}

void BanListPanel::set_table_model(BanListQTableModel* entry_model)
{
	if (entry_model == nullptr)
	{
		entry_model = new BanListQTableModel{ this, {} };
	}
	tree_view->setModel(entry_model);
	connect(tree_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &BanListPanel::handle_selected_ban_changed);
	for (int i = 0; i < entry_model->columnCount(); i++)
	{
		tree_view->resizeColumnToContents(i);
	}
	gui_refresh();
}

void BanListPanel::handle_selected_ban_changed()
{
	gui_refresh();
}

void BanListPanel::pressed_details()
{
	const std::optional<BanListUserRestriction> opt_restriction = get_selected_restriction();

	ViewBanWindow* const ban_window = new ViewBanWindow{ ViewEditMode::View, "", *opt_restriction, this };
	ban_window->show();
}

void BanListPanel::pressed_edit()
{
	const std::optional<BanListUserRestriction> opt_restriction = get_selected_restriction();

	ViewBanWindow* const ban_window = new ViewBanWindow{ ViewEditMode::Edit, api_key, *opt_restriction, this };
	ban_window->show();
}

void BanListPanel::pressed_refresh()
{
	auto universe = attached_universe.lock();
	OCTASSERT(universe);

	const bool active_only = active_only_check->isChecked();
	auto req = std::make_shared<UserRestrictionGetListV2Request>(api_key, universe->get_universe_id(), active_only);
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	BanListQTableModel* const new_model = new BanListQTableModel{ this, req->get_restrictions() };
	set_table_model(new_model);
}

void BanListPanel::pressed_unban()
{
	const std::optional<BanListUserRestriction> opt_restriction = get_selected_restriction();
	if (!opt_restriction)
	{
		return;
	}

	ConfirmChangeDialog* const confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::BanListUnbanUser, opt_restriction->get_user()};
	bool confirmed = static_cast<bool>(confirm_dialog->exec());
	if (!confirmed)
	{
		return;
	}

	constexpr bool active = false;
	const std::optional<QString> duration;
	const QString reason = "Manually unbanned";
	constexpr bool exclude_alts = false;
	const BanListGameJoinRestrictionUpdate update{ active, duration, reason, reason, exclude_alts };

	const auto update_req = std::make_shared<UserRestrictionPatchUpdateV2Request>(api_key, opt_restriction->get_path(), update);
	OperationInProgressDialog diag{ this, update_req };
	diag.exec();
}
