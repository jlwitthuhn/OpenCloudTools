#include "window_datastore_entry_versions_view.h"

#include <optional>

#include <Qt>
#include <QAbstractItemModel>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMargins>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>

#include "api_response.h"
#include "data_request.h"
#include "datastore_model.h"
#include "diag_confirm_change.h"
#include "diag_operation_in_progress.h"
#include "window_datastore_entry_view.h"

ViewDatastoreEntryVersionsWindow::ViewDatastoreEntryVersionsWindow(QWidget* parent, const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const std::vector<StandardDatastoreEntryVersion>& versions)
	: QWidget{ parent, Qt::Window }, api_key{ api_key }
{
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
		DatastoreEntryVersionModel* version_model = new DatastoreEntryVersionModel{ versions_tree, versions };
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

void ViewDatastoreEntryVersionsWindow::view_version(const QModelIndex& index)
{
	if (index.isValid())
	{
		if (DatastoreEntryVersionModel* version_model = dynamic_cast<DatastoreEntryVersionModel*>(versions_tree->model()))
		{
			if (std::optional<StandardDatastoreEntryVersion> opt_version = version_model->get_version(index.row()))
			{
				const long long universe_id = universe_id_edit->text().toLongLong();
				const QString datastore_name = datastore_name_edit->text();
				const QString scope = scope_edit->text();
				const QString key_name = key_name_edit->text();
				const QString version = opt_version->get_version();

				GetStandardDatastoreEntryAtVersionRequest req{ nullptr, api_key, universe_id, datastore_name, scope, key_name, version };
				OperationInProgressDialog diag{ this, &req };
				req.send_request();
				diag.exec();

				const std::optional<DatastoreEntryWithDetails> opt_details = req.get_details();
				if (opt_details)
				{
					ViewDatastoreEntryWindow* view_entry_window = new ViewDatastoreEntryWindow{ this, api_key, *opt_details };
					view_entry_window->setWindowModality(Qt::WindowModality::ApplicationModal);
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

	GetStandardDatastoreEntryVersionsRequest req{ nullptr, api_key, universe_id, datastore_name, scope, key_name };
	OperationInProgressDialog diag{ this, &req };
	req.send_request();
	diag.exec();

	if (req.get_versions().size() > 0)
	{
		ViewDatastoreEntryVersionsWindow* view_versions_window = new ViewDatastoreEntryVersionsWindow{ parentWidget(), api_key, universe_id, datastore_name, scope, key_name, req.get_versions()};
		view_versions_window->setWindowModality(Qt::WindowModality::ApplicationModal);
		view_versions_window->show();
		close();
		deleteLater();
	}
}

void ViewDatastoreEntryVersionsWindow::pressed_revert()
{
	QModelIndex current_index = versions_tree->currentIndex();
	if (current_index.isValid())
	{
		if (DatastoreEntryVersionModel* version_model = dynamic_cast<DatastoreEntryVersionModel*>(versions_tree->model()))
		{
			if (std::optional<StandardDatastoreEntryVersion> opt_version = version_model->get_version(current_index.row()))
			{
				const long long universe_id = universe_id_edit->text().toLongLong();
				const QString datastore_name = datastore_name_edit->text();
				const QString scope = scope_edit->text();
				const QString key_name = key_name_edit->text();
				const QString version = opt_version->get_version();

				ConfirmChangeDialog* confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::Revert };
				bool confirmed = static_cast<bool>(confirm_dialog->exec());
				if (confirmed)
				{
					GetStandardDatastoreEntryAtVersionRequest req{ nullptr, api_key, universe_id, datastore_name, scope, key_name, version };
					OperationInProgressDialog diag{ this, &req };
					req.send_request();
					diag.exec();

					const std::optional<DatastoreEntryWithDetails> opt_details = req.get_details();
					if (opt_details)
					{
						const std::optional<QString> userids = opt_details->get_userids();
						const std::optional<QString> attributes = opt_details->get_attributes();
						const QString body = opt_details->get_data_raw();

						PostStandardDatastoreEntryRequest post_req{ nullptr, api_key, universe_id, datastore_name, scope, key_name, userids, attributes, body };
						OperationInProgressDialog diag{ this, &post_req };
						post_req.send_request();
						diag.exec();

						if (post_req.get_success())
						{
							pressed_refresh();
						}
					}
				}
			}
		}
	}
}

void ViewDatastoreEntryVersionsWindow::pressed_view()
{
	view_version(versions_tree->currentIndex());
}
