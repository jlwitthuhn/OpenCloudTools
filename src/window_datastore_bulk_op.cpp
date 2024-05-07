#include "window_datastore_bulk_op.h"

#include <memory>
#include <optional>
#include <utility>
#include <set>

#include <Qt>
#include <QCheckBox>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFlags>
#include <QFrame>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMargins>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "assert.h"
#include "diag_confirm_change.h"
#include "profile.h"
#include "roblox_time.h"
#include "sqlite_wrapper.h"
#include "window_datastore_bulk_op_progress.h"

DatastoreBulkOperationWindow::DatastoreBulkOperationWindow(QWidget* parent, const QString& api_key, const long long universe_id, const std::vector<QString>& datastore_names) :
	QWidget{ parent, Qt::Window },
	api_key{ api_key },
	universe_id{ universe_id }
{
	setAttribute(Qt::WA_DeleteOnClose);

	OCTASSERT(parent != nullptr);
	setWindowModality(Qt::WindowModality::WindowModal);

	QWidget* main_panel = new QWidget{ this };
	{
		QWidget* left_bar = new QWidget{ main_panel };
		{
			QGroupBox* datastore_group = new QGroupBox{ "Datastores", left_bar };
			{
				datastore_list = new QListWidget{ datastore_group };
				for (const QString& this_datastore_name : datastore_names)
				{
					QListWidgetItem* this_item = new QListWidgetItem{ datastore_list };
					this_item->setText(this_datastore_name);
					this_item->setFlags(this_item->flags() | Qt::ItemIsUserCheckable);
					this_item->setCheckState(Qt::Checked);
					datastore_list->addItem(this_item);
				}

				datastore_list_show_hidden_check = new QCheckBox{ "Show hidden", datastore_group };
				connect(datastore_list_show_hidden_check, &QCheckBox::stateChanged, this, &DatastoreBulkOperationWindow::handle_show_hidden_toggled);

				QWidget* select_buttons_widget = new QWidget{ left_bar };
				{
					QPushButton* select_all_button = new QPushButton{ "Select all", select_buttons_widget };
					connect(select_all_button, &QPushButton::clicked, this, &DatastoreBulkOperationWindow::pressed_select_all);

					QPushButton* select_none_button = new QPushButton{ "Select none", select_buttons_widget };
					connect(select_none_button, &QPushButton::clicked, this, &DatastoreBulkOperationWindow::pressed_select_none);

					QHBoxLayout* select_buttons_layout = new QHBoxLayout{ select_buttons_widget };
					select_buttons_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
					select_buttons_layout->addWidget(select_all_button);
					select_buttons_layout->addWidget(select_none_button);
				}

				QVBoxLayout* datastore_group_layout = new QVBoxLayout{ datastore_group };
				datastore_group_layout->addWidget(datastore_list);
				datastore_group_layout->addWidget(datastore_list_show_hidden_check);
				datastore_group_layout->addWidget(select_buttons_widget);
			}

			QVBoxLayout* left_bar_layout = new QVBoxLayout{ left_bar };
			left_bar_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
			left_bar_layout->addWidget(datastore_group);
		}

		right_bar = new QWidget{ main_panel };
		{
			QGroupBox* filter_box = new QGroupBox{ "Filter", right_bar };
			{
				filter_enabled_check = new QCheckBox{ "Filter Enabled", filter_box};
				connect(filter_enabled_check, &QCheckBox::stateChanged, this, &DatastoreBulkOperationWindow::pressed_toggle_filter);

				QWidget* filter_form = new QWidget{ filter_box };
				filter_form->setMinimumWidth(220);
				{
					filter_scope_edit = new QLineEdit{ filter_form };
					filter_key_prefix_edit = new QLineEdit{ filter_form };

					QFormLayout* form_layout = new QFormLayout{ filter_form };
					form_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
					form_layout->addRow("Scope:", filter_scope_edit);
					form_layout->addRow("Key prefix:", filter_key_prefix_edit);
				}

				QVBoxLayout* filter_layout = new QVBoxLayout{ filter_box };
				filter_layout->addWidget(filter_enabled_check);
				filter_layout->addWidget(filter_form);
			}

			right_bar_layout = new QVBoxLayout{ right_bar };
			right_bar_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
			right_bar_layout->addWidget(filter_box);
		}

		QHBoxLayout* main_panel_layout = new QHBoxLayout{ main_panel };
		main_panel_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		main_panel_layout->addWidget(left_bar);
		main_panel_layout->addWidget(right_bar);
	}

	submit_button = new QPushButton{ "Submit", this };
	connect(submit_button, &QPushButton::clicked, this, &DatastoreBulkOperationWindow::pressed_submit);

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->addWidget(main_panel);
	layout->addWidget(submit_button);

	handle_show_hidden_toggled();
	pressed_toggle_filter();
}

std::vector<QString> DatastoreBulkOperationWindow::get_selected_datastores() const
{
	std::vector<QString> result;
	for (int i = 0; i < datastore_list->count(); i++)
	{
		if (datastore_list->item(i)->checkState() == Qt::Checked)
		{
			result.push_back(datastore_list->item(i)->text());
		}
	}

	return result;
}

void DatastoreBulkOperationWindow::handle_show_hidden_toggled()
{
	const UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
	if (selected_universe)
	{
		const bool show_hidden = datastore_list_show_hidden_check->isChecked();
		for (int i = 0; i < datastore_list->count(); i++)
		{
			QListWidgetItem* this_item = datastore_list->item(i);
			const bool hide = show_hidden ? false : static_cast<bool>(selected_universe->get_hidden_datastore_set().count(this_item->text()));
			this_item->setHidden(hide);
			if (hide)
			{
				this_item->setCheckState(Qt::Unchecked);
			}
		}
	}
}

void DatastoreBulkOperationWindow::pressed_select_all()
{
	for (int i = 0; i < datastore_list->count(); i++)
	{
		QListWidgetItem* this_item = datastore_list->item(i);
		if (this_item->isHidden() == false)
		{
			this_item->setCheckState(Qt::Checked);
		}
	}
}

void DatastoreBulkOperationWindow::pressed_select_none()
{
	for (int i = 0; i < datastore_list->count(); i++)
	{
		datastore_list->item(i)->setCheckState(Qt::Unchecked);
	}
}

void DatastoreBulkOperationWindow::pressed_toggle_filter()
{
	const bool filter_enabled = filter_enabled_check->isChecked();
	filter_scope_edit->setEnabled(filter_enabled);
	filter_key_prefix_edit->setEnabled(filter_enabled);
}

DatastoreBulkDeleteWindow::DatastoreBulkDeleteWindow(QWidget* parent, const QString& api_key, const long long universe_id, const std::vector<QString>& datastore_names) :
	DatastoreBulkOperationWindow{ parent, api_key, universe_id, datastore_names }
{
	setWindowTitle("Delete Datastores");

	submit_button->setText("Delete");

	QGroupBox* options_box = new QGroupBox{ "Delete Options", right_bar };
	{
		confirm_count_before_delete_check = new QCheckBox{ "Confirm entry count before deletion", options_box };
		confirm_count_before_delete_check->setCheckState(Qt::Checked);
		if (UserProfile::get_selected_api_key() && UserProfile::get_selected_api_key()->get_production())
		{
			confirm_count_before_delete_check->setEnabled(false);
		}

		rewrite_before_delete_check = new QCheckBox{ "Rewrite entries before deletion", options_box };

		hide_after_delete_check = new QCheckBox{ "Hide datastore after deletion", options_box };

		QVBoxLayout* options_layout = new QVBoxLayout{ options_box };
		options_layout->addWidget(confirm_count_before_delete_check);
		options_layout->addWidget(rewrite_before_delete_check);
		options_layout->addWidget(hide_after_delete_check);
	};

	right_bar_layout->addWidget(options_box);
	right_bar_layout->addStretch();

	pressed_select_none();
}

void DatastoreBulkDeleteWindow::pressed_submit()
{
	const std::vector<QString> selected_datastores = get_selected_datastores();
	if (selected_datastores.size() > 0)
	{
		ConfirmChangeDialog* confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::StandardDatastoreBulkDelete };
		bool confirmed = static_cast<bool>(confirm_dialog->exec());
		if (confirmed)
		{
			const QString scope = filter_enabled_check->isChecked() ? filter_scope_edit->text().trimmed() : "";
			const QString key_prefix = filter_enabled_check->isChecked() ? filter_key_prefix_edit->text().trimmed() : "";
			const bool confirm_count_before_delete = confirm_count_before_delete_check->isChecked();
			const bool rewrite_before_delete = rewrite_before_delete_check->isChecked();
			const bool hide_datastores_after = hide_after_delete_check->isChecked();
			DatastoreBulkDeleteProgressWindow* progress_window = new DatastoreBulkDeleteProgressWindow{ dynamic_cast<QWidget*>(parent()), api_key, universe_id, scope, key_prefix, selected_datastores, confirm_count_before_delete, rewrite_before_delete, hide_datastores_after };
			close();
			progress_window->show();
			progress_window->start();
		}
	}
	else
	{
		QMessageBox* msg_box = new QMessageBox{ this };
		msg_box->setWindowTitle("Error");
		msg_box->setText("You must select at least one datastore.");
		msg_box->exec();
	}
}

DatastoreBulkDownloadWindow::DatastoreBulkDownloadWindow(QWidget* parent, const QString& api_key, const long long universe_id, const std::vector<QString>& datastore_names) :
	DatastoreBulkOperationWindow{ parent, api_key, universe_id, datastore_names }
{
	setWindowTitle("Download Datastores");

	submit_button->setText("Save as...");
	right_bar_layout->addStretch();
}

void DatastoreBulkDownloadWindow::pressed_submit()
{
	const std::vector<QString> selected_datastores = get_selected_datastores();
	if (selected_datastores.size() > 0)
	{
		QString file_name = QFileDialog::getSaveFileName(this, "Save as...", "datastore.sqlite3", "sqlite3 databases (*.sqlite3)");
		if (file_name.trimmed().length() > 0)
		{
			{
				QFile existing_file{ file_name };
				if (existing_file.exists())
				{
					const QMessageBox::StandardButton response =
						QMessageBox::warning(nullptr, "File already exists", "The existing sqlite database will be deleted prior to download, proceed?", QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);

					if (response != QMessageBox::StandardButton::Yes)
					{
						return;
					}

					if (existing_file.remove() == false)
					{
						QMessageBox* msg_box = new QMessageBox{ this };
						msg_box->setWindowTitle("Error");
						msg_box->setText("Failed to delete existing file.");
						msg_box->exec();
						return;
					}
				}
			}

			std::unique_ptr<SqliteDatastoreWrapper> writer = SqliteDatastoreWrapper::new_from_path(file_name.toStdString());
			if (writer)
			{
				const QString scope = filter_enabled_check->isChecked() ? filter_scope_edit->text().trimmed() : "";
				const QString key_prefix = filter_enabled_check->isChecked() ? filter_key_prefix_edit->text().trimmed() : "";
				DatastoreBulkDownloadProgressWindow* progress_window = new DatastoreBulkDownloadProgressWindow{ dynamic_cast<QWidget*>(parent()), api_key, universe_id, scope, key_prefix, selected_datastores, std::move(writer) };
				close();
				progress_window->show();
				progress_window->start();
			}
			else
			{
				QMessageBox* msg_box = new QMessageBox{ this };
				msg_box->setWindowTitle("Error");
				msg_box->setText("Failed to open file for writing.");
				msg_box->exec();
			}
		}
	}
	else
	{
		QMessageBox* msg_box = new QMessageBox{ this };
		msg_box->setWindowTitle("Error");
		msg_box->setText("You must select at least one datastore.");
		msg_box->exec();
	}
}

DatastoreBulkUndeleteWindow::DatastoreBulkUndeleteWindow(QWidget* parent, const QString& api_key, const long long universe_id, const std::vector<QString>& datastore_names) :
	DatastoreBulkOperationWindow{ parent, api_key, universe_id, datastore_names }
{
	setWindowTitle("Undelete");
	submit_button->setText("Undelete");

	QGroupBox* options_box = new QGroupBox{ "Undelete Options", right_bar };
	{
		time_filter_check = new QCheckBox{ "Only entries deleted in the last", options_box };
		connect(time_filter_check, &QCheckBox::stateChanged, this, &DatastoreBulkUndeleteWindow::pressed_toggle_time_filter);

		if (RobloxTime::is_initialized() == false)
		{
			time_filter_check->setEnabled(false);
		}

		QWidget* time_filter_bar = new QWidget{ options_box };
		{
			day_edit = new QLineEdit{ time_filter_bar };
			day_edit->setFixedWidth(32);
			day_edit->setText("0");
			day_edit->setValidator(new QIntValidator{ 0, 999, day_edit });

			QLabel* day_label = new QLabel{ "days", time_filter_bar };

			QFrame* separator1 = new QFrame{ time_filter_bar };
			separator1->setFrameShape(QFrame::VLine);
			separator1->setFrameShadow(QFrame::Sunken);

			hour_edit = new QLineEdit{ time_filter_bar };
			hour_edit->setFixedWidth(32);
			hour_edit->setText("0");
			hour_edit->setValidator(new QIntValidator{ 0, 99, hour_edit });

			QLabel* hour_label = new QLabel{ "hours", time_filter_bar };

			QFrame* separator2 = new QFrame{ time_filter_bar };
			separator2->setFrameShape(QFrame::VLine);
			separator2->setFrameShadow(QFrame::Sunken);

			min_edit = new QLineEdit{ time_filter_bar };
			min_edit->setFixedWidth(32);
			min_edit->setText("0");
			min_edit->setValidator(new QIntValidator{ 0, 99, min_edit });

			QLabel* min_label = new QLabel{ "min", time_filter_bar };

			QHBoxLayout* time_filter_layout = new QHBoxLayout{ time_filter_bar };
			time_filter_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
			time_filter_layout->addWidget(day_edit);
			time_filter_layout->addWidget(day_label);
			time_filter_layout->addWidget(separator1);
			time_filter_layout->addWidget(hour_edit);
			time_filter_layout->addWidget(hour_label);
			time_filter_layout->addWidget(separator2);
			time_filter_layout->addWidget(min_edit);
			time_filter_layout->addWidget(min_label);
		}

		QVBoxLayout* options_layout = new QVBoxLayout{ options_box };
		options_layout->addWidget(time_filter_check);
		options_layout->addWidget(time_filter_bar);
	};

	right_bar_layout->addWidget(options_box);
	right_bar_layout->addStretch();

	pressed_select_none();
	pressed_toggle_time_filter();
}

void DatastoreBulkUndeleteWindow::pressed_submit()
{
	const std::vector<QString> selected_datastores = get_selected_datastores();
	if (selected_datastores.size() > 0)
	{
		ConfirmChangeDialog* confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::StandardDatastoreBulkUndelete };
		bool confirmed = static_cast<bool>(confirm_dialog->exec());
		if (confirmed)
		{
			const QString scope = filter_enabled_check->isChecked() ? filter_scope_edit->text().trimmed() : "";
			const QString key_prefix = filter_enabled_check->isChecked() ? filter_key_prefix_edit->text().trimmed() : "";
			DatastoreBulkUndeleteProgressWindow* progress_window = new DatastoreBulkUndeleteProgressWindow{ dynamic_cast<QWidget*>(parent()), api_key, universe_id, scope, key_prefix, selected_datastores, get_undelete_after_time() };
			close();
			progress_window->show();
			progress_window->start();
		}
	}
	else
	{
		QMessageBox* msg_box = new QMessageBox{ this };
		msg_box->setWindowTitle("Error");
		msg_box->setText("You must select at least one datastore.");
		msg_box->exec();
	}
}

void DatastoreBulkUndeleteWindow::pressed_toggle_time_filter()
{
	const bool enabled = time_filter_check->isChecked();
	day_edit->setEnabled(enabled);
	hour_edit->setEnabled(enabled);
	min_edit->setEnabled(enabled);
}

std::optional<QDateTime> DatastoreBulkUndeleteWindow::get_undelete_after_time() const
{
	if (time_filter_check->isChecked())
	{
		if (const std::optional<QDateTime> roblox_time = RobloxTime::get_roblox_time())
		{
			QDateTime result{ *roblox_time };

			{
				bool success = false;
				const long long days = day_edit->text().toLongLong(&success);
				if (success)
				{
					result = result.addDays(-1 * days);
				}
			}

			{
				long long seconds_to_subtract = 0;

				{
					bool hours_success = false;
					const long long hours = hour_edit->text().toLongLong(&hours_success);
					if (hours_success)
					{
						seconds_to_subtract += hours * 60 * 60;
					}
				}

				{
					bool minutes_success = false;
					const long long minutes = min_edit->text().toLongLong(&minutes_success);
					if (minutes_success)
					{
						seconds_to_subtract += minutes * 60;
					}
				}

				return result.addSecs(-1 * seconds_to_subtract);
			}
		}
	}

	return std::nullopt;
}
