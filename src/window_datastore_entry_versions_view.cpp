#include "window_datastore_entry_versions_view.h"

#include <memory>
#include <optional>

#include <Qt>
#include <QAbstractItemModel>
#include <QAction>
#include <QClipboard>
#include <QFormLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMargins>
#include <QMenu>
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
#include "window_datastore_entry_view.h"

ViewDatastoreEntryVersionsWindow::ViewDatastoreEntryVersionsWindow(QWidget* parent, const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const std::vector<StandardDatastoreEntryVersion>& versions)
	: QWidget{ parent, Qt::Window }, api_key{ api_key }
{
	setAttribute(Qt::WA_DeleteOnClose);

	OCTASSERT(parent != nullptr);
	setWindowModality(Qt::WindowModality::WindowModal);

	setWindowTitle("View Versions");
	setMinimumWidth(725);

	QWidget* info_panel = new QWidget{ this };
	{
		universe_id_edit = new QLineEdit{ info_panel };
		universe_id_edit->setReadOnly(true);
		universe_id_edit->setText(QString::number(universe_id));

		datastore_name_edit = new QLineEdit{ info_panel };
		datastore_name_edit->setReadOnly(true);
		datastore_name_edit->setText(datastore_name);

		scope_edit = new QLineEdit{ info_panel };
		scope_edit->setReadOnly(true);
		scope_edit->setText(scope);

		key_name_edit = new QLineEdit{ info_panel };
		key_name_edit->setReadOnly(true);
		key_name_edit->setText(key_name);

		versions_tree = new QTreeView{ info_panel };
		versions_tree->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
		connect(versions_tree, &QTreeView::customContextMenuRequested, this, &ViewDatastoreEntryVersionsWindow::pressed_right_click);
		StandardDatastoreEntryVersionQTableModel* version_model = new StandardDatastoreEntryVersionQTableModel{ versions_tree, versions };
		versions_tree->setModel(version_model);
		for (int i = 0; i < version_model->columnCount(); i++)
		{
			versions_tree->resizeColumnToContents(i);
		}
		connect(versions_tree, &QTreeView::doubleClicked, this, &ViewDatastoreEntryVersionsWindow::handle_version_double_clicked);
		connect(versions_tree, &QTreeView::pressed, this, &ViewDatastoreEntryVersionsWindow::handle_selected_version_changed);

		QFormLayout* info_layout = new QFormLayout{ info_panel };
		info_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
        info_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		info_layout->addRow("Universe", universe_id_edit);
		info_layout->addRow("Datastore", datastore_name_edit);
		info_layout->addRow("Scope", scope_edit);
		info_layout->addRow("Key", key_name_edit);
		info_layout->addRow("Versions", versions_tree);
	}

	refresh_button = new QPushButton{ "Refresh list", this };
	connect(refresh_button, &QPushButton::clicked, this, &ViewDatastoreEntryVersionsWindow::pressed_refresh);

	QWidget* button_panel = new QWidget{ this };
	{
		view_button = new QPushButton{ "View...", button_panel };
		connect(view_button, &QPushButton::clicked, this, &ViewDatastoreEntryVersionsWindow::pressed_view);

		revert_button = new QPushButton{ "Revert to", button_panel };
		connect(revert_button, &QPushButton::clicked, this, &ViewDatastoreEntryVersionsWindow::pressed_revert);

		QHBoxLayout* read_button_layout = new QHBoxLayout{ button_panel };
		read_button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		read_button_layout->addWidget(view_button);
		read_button_layout->addWidget(revert_button);
	}

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->addWidget(info_panel);
	layout->addWidget(refresh_button);
	layout->addWidget(button_panel);

	handle_selected_version_changed();
}

void ViewDatastoreEntryVersionsWindow::revert_to_version(const QModelIndex& index)
{
	if (index.isValid())
	{
		if (StandardDatastoreEntryVersionQTableModel* version_model = dynamic_cast<StandardDatastoreEntryVersionQTableModel*>(versions_tree->model()))
		{
			if (std::optional<StandardDatastoreEntryVersion> opt_version = version_model->get_version(index.row()))
			{
				const long long universe_id = universe_id_edit->text().toLongLong();
				const QString datastore_name = datastore_name_edit->text();
				const QString scope = scope_edit->text();
				const QString key_name = key_name_edit->text();
				const QString version = opt_version->get_version();

				ConfirmChangeDialog* confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::StandardDatastoreRevert };
				bool confirmed = static_cast<bool>(confirm_dialog->exec());
				if (confirmed)
				{
					const auto req = std::make_shared<GetStandardDatastoreEntryAtVersionRequest>(api_key, universe_id, datastore_name, scope, key_name, version);
					OperationInProgressDialog get_diag{ this, req };
					get_diag.exec();

					const std::optional<StandardDatastoreEntryFull> opt_details = req->get_details();
					if (opt_details)
					{
						const std::optional<QString> userids = opt_details->get_userids();
						const std::optional<QString> attributes = opt_details->get_attributes();
						const QString body = opt_details->get_data_raw();

						const auto post_req = std::make_shared<PostStandardDatastoreEntryRequest>(api_key, universe_id, datastore_name, scope, key_name, userids, attributes, body);
						OperationInProgressDialog post_diag{ this, post_req };
						post_diag.exec();

						if (post_req->req_success())
						{
							pressed_refresh();
						}
					}
				}
			}
		}
	}
}

void ViewDatastoreEntryVersionsWindow::view_version(const QModelIndex& index)
{
	if (index.isValid())
	{
		if (StandardDatastoreEntryVersionQTableModel* version_model = dynamic_cast<StandardDatastoreEntryVersionQTableModel*>(versions_tree->model()))
		{
			if (std::optional<StandardDatastoreEntryVersion> opt_version = version_model->get_version(index.row()))
			{
				const long long universe_id = universe_id_edit->text().toLongLong();
				const QString datastore_name = datastore_name_edit->text();
				const QString scope = scope_edit->text();
				const QString key_name = key_name_edit->text();
				const QString version = opt_version->get_version();

				const auto req = std::make_shared<GetStandardDatastoreEntryAtVersionRequest>(api_key, universe_id, datastore_name, scope, key_name, version);
				OperationInProgressDialog diag{ this, req };
				diag.exec();

				const std::optional<StandardDatastoreEntryFull> opt_details = req->get_details();
				if (opt_details)
				{
					ViewDatastoreEntryWindow* view_entry_window = new ViewDatastoreEntryWindow{ this, api_key, *opt_details };
					view_entry_window->show();
				}
			}
		}
	}
}

void ViewDatastoreEntryVersionsWindow::handle_selected_version_changed()
{
	const bool valid = versions_tree->currentIndex().isValid();
	view_button->setEnabled(valid);
	revert_button->setEnabled(valid);
}

void ViewDatastoreEntryVersionsWindow::handle_version_double_clicked(const QModelIndex& index)
{
	view_version(index);
}

void ViewDatastoreEntryVersionsWindow::pressed_refresh()
{
	const long long universe_id = universe_id_edit->text().toLongLong();
	const QString datastore_name = datastore_name_edit->text();
	const QString scope = scope_edit->text();
	const QString key_name = key_name_edit->text();

	const auto req = std::make_shared<GetStandardDatastoreEntryVersionListRequest>(api_key, universe_id, datastore_name, scope, key_name);
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	if (req->get_versions().size() > 0)
	{
		ViewDatastoreEntryVersionsWindow* view_versions_window = new ViewDatastoreEntryVersionsWindow{ parentWidget(), api_key, universe_id, datastore_name, scope, key_name, req->get_versions()};
		view_versions_window->show();
		close();
	}
}

void ViewDatastoreEntryVersionsWindow::pressed_revert()
{
	QModelIndex current_index = versions_tree->currentIndex();
	if (current_index.isValid())
	{
		if (StandardDatastoreEntryVersionQTableModel* version_model = dynamic_cast<StandardDatastoreEntryVersionQTableModel*>(versions_tree->model()))
		{
			if (std::optional<StandardDatastoreEntryVersion> opt_version = version_model->get_version(current_index.row()))
			{
				const long long universe_id = universe_id_edit->text().toLongLong();
				const QString datastore_name = datastore_name_edit->text();
				const QString scope = scope_edit->text();
				const QString key_name = key_name_edit->text();
				const QString version = opt_version->get_version();

				ConfirmChangeDialog* confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::StandardDatastoreRevert };
				bool confirmed = static_cast<bool>(confirm_dialog->exec());
				if (confirmed)
				{
					const auto req = std::make_shared<GetStandardDatastoreEntryAtVersionRequest>(api_key, universe_id, datastore_name, scope, key_name, version);
					OperationInProgressDialog get_diag{ this, req };
					get_diag.exec();

					const std::optional<StandardDatastoreEntryFull> opt_details = req->get_details();
					if (opt_details)
					{
						const std::optional<QString> userids = opt_details->get_userids();
						const std::optional<QString> attributes = opt_details->get_attributes();
						const QString body = opt_details->get_data_raw();

						const auto post_req = std::make_shared<PostStandardDatastoreEntryRequest>(api_key, universe_id, datastore_name, scope, key_name, userids, attributes, body);
						OperationInProgressDialog post_diag{ this, post_req };
						post_diag.exec();

						if (post_req->req_success())
						{
							pressed_refresh();
						}
					}
				}
			}
		}
	}
}

void ViewDatastoreEntryVersionsWindow::pressed_right_click(const QPoint& pos)
{
	const QModelIndex the_index = versions_tree->indexAt(pos);
	if (the_index.isValid())
	{
		if (StandardDatastoreEntryVersionQTableModel* version_model = dynamic_cast<StandardDatastoreEntryVersionQTableModel*>(versions_tree->model()))
		{
			if (std::optional<StandardDatastoreEntryVersion> opt_version = version_model->get_version(the_index.row()))
			{
				QMenu* context_menu = new QMenu{ versions_tree };
				{
					QAction* act_view = new QAction{ "View...", context_menu };
					connect(act_view, &QAction::triggered, [this, the_index]() {
						view_version(the_index);
					});

					QAction* act_revert = new QAction{ "Revert to", context_menu };
					connect(act_revert, &QAction::triggered, [this, the_index]() {
						revert_to_version(the_index);
					});

					QAction* copy_version = new QAction{ "Copy Version", context_menu };
					connect(copy_version, &QAction::triggered, [opt_version]() {
						QClipboard* clipboard = QGuiApplication::clipboard();
						clipboard->setText(opt_version->get_version());
					});

					QAction* copy_timestamp = new QAction{ "Copy Timestamp", context_menu };
					connect(copy_timestamp, &QAction::triggered, [opt_version]() {
						QClipboard* clipboard = QGuiApplication::clipboard();
						clipboard->setText(opt_version->get_created_time());
					});

					context_menu->addAction(act_view);
					context_menu->addAction(act_revert);
					context_menu->addSeparator();
					context_menu->addAction(copy_version);
					context_menu->addAction(copy_timestamp);
				}

				context_menu->exec(versions_tree->mapToGlobal(pos));
				context_menu->deleteLater();
			}
		}
	}
}

void ViewDatastoreEntryVersionsWindow::pressed_view()
{
	view_version(versions_tree->currentIndex());
}
