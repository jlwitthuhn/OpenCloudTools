#include "window_datastore_download.h"

#include <memory>
#include <utility>

#include <Qt>
#include <QCheckBox>
#include <QFileDialog>
#include <QFlags>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMargins>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "sqlite_wrapper.h"
#include "window_datastore_download_progress.h"

DownloadDatastoreWindow::DownloadDatastoreWindow(QWidget* parent, const QString& api_key, const long long universe_id, const std::vector<QString>& datastore_names) :
	QWidget{ parent, Qt::Window },
	api_key{ api_key },
	universe_id{ universe_id }
{
	setWindowTitle("Download Datastores");

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

				QVBoxLayout* datastore_group_layout = new QVBoxLayout{ datastore_group };
				datastore_group_layout->addWidget(datastore_list);
			}

			QWidget* select_buttons_widget = new QWidget{ left_bar };
			{
				QPushButton* select_all_button = new QPushButton{ "Select all", select_buttons_widget };
				connect(select_all_button, &QPushButton::clicked, this, &DownloadDatastoreWindow::pressed_select_all);

				QPushButton* select_none_button = new QPushButton{ "Select none", select_buttons_widget };
				connect(select_none_button, &QPushButton::clicked, this, &DownloadDatastoreWindow::pressed_select_none);

				QHBoxLayout* select_buttons_layout = new QHBoxLayout{ select_buttons_widget };
				select_buttons_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
				select_buttons_layout->addWidget(select_all_button);
				select_buttons_layout->addWidget(select_none_button);
			}

			QVBoxLayout* left_bar_layout = new QVBoxLayout{ left_bar };
			left_bar_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
			left_bar_layout->addWidget(datastore_group);
			left_bar_layout->addWidget(select_buttons_widget);
		}

		QWidget* right_bar = new QWidget{ main_panel };
		{
			QGroupBox* filter_box = new QGroupBox{ "Filter", right_bar };
			{
				filter_enabled_check = new QCheckBox{ "Filter Enabled", filter_box};
				connect(filter_enabled_check, &QCheckBox::stateChanged, this, &DownloadDatastoreWindow::pressed_toggle_filter);

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

			QVBoxLayout* right_bar_layout = new QVBoxLayout{ right_bar };
			right_bar_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
			right_bar_layout->addWidget(filter_box);
		}

		QHBoxLayout* main_panel_layout = new QHBoxLayout{ main_panel };
		main_panel_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		main_panel_layout->addWidget(left_bar);
		main_panel_layout->addWidget(right_bar);
	}

	QPushButton* save_button = new QPushButton{ "Save as...", this };
	connect(save_button, &QPushButton::clicked, this, &DownloadDatastoreWindow::pressed_save);

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->addWidget(main_panel);
	layout->addWidget(save_button);

	pressed_toggle_filter();
}

std::vector<QString> DownloadDatastoreWindow::get_selected_datastores() const
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

void DownloadDatastoreWindow::pressed_save()
{
	const std::vector<QString> selected_datastores = get_selected_datastores();
	if (selected_datastores.size() > 0)
	{
		QString file_name = QFileDialog::getSaveFileName(this, "Save as...", "datastore.sqlite3", "sqlite3 databases (*.sqlite3)");
		if (file_name.trimmed().length() > 0)
		{
			std::unique_ptr<SqliteDatastoreWriter> writer = SqliteDatastoreWriter::from_path(file_name.toStdString());
			if (writer)
			{
				const QString scope = filter_enabled_check->isChecked() ? filter_scope_edit->text().trimmed() : "";
				const QString key_prefix = filter_enabled_check->isChecked() ? filter_key_prefix_edit->text().trimmed() : "";
				DownloadDatastoreProgressWindow* progress_window = new DownloadDatastoreProgressWindow{ dynamic_cast<QWidget*>(parent()), api_key, universe_id, scope, key_prefix, selected_datastores, std::move(writer) };
				close();
				progress_window->setWindowModality(Qt::WindowModality::ApplicationModal);
				progress_window->show();
			}
			else
			{
				QMessageBox* msg_box = new QMessageBox{ this };
				msg_box->setWindowTitle("Error");
				msg_box->setText("Failed to create file.");
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

void DownloadDatastoreWindow::pressed_select_all()
{
	for (int i = 0; i < datastore_list->count(); i++)
	{
		datastore_list->item(i)->setCheckState(Qt::Checked);
	}
}

void DownloadDatastoreWindow::pressed_select_none()
{
	for (int i = 0; i < datastore_list->count(); i++)
	{
		datastore_list->item(i)->setCheckState(Qt::Unchecked);
	}
}

void DownloadDatastoreWindow::pressed_toggle_filter()
{
	const bool filter_enabled = filter_enabled_check->isChecked();
	filter_scope_edit->setEnabled(filter_enabled);
	filter_key_prefix_edit->setEnabled(filter_enabled);
}
