#include "panel_datastore_standard.h"

#include <cstddef>

#include <memory>
#include <optional>
#include <set>

#include <Qt>
#include <QAbstractItemModel>
#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMargins>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QPushButton>
#include <QSizePolicy>
#include <QTabWidget>
#include <QTextEdit>
#include <QTreeView>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "data_request.h"
#include "diag_confirm_change.h"
#include "diag_operation_in_progress.h"
#include "model_common.h"
#include "model_qt.h"
#include "profile.h"
#include "util_enum.h"
#include "util_json.h"
#include "util_validator.h"
#include "window_datastore_entry_versions_view.h"
#include "window_datastore_entry_view.h"

static void show_validation_error(QWidget* const parent, const QString& message)
{
	QMessageBox* message_box = new QMessageBox{ parent };
	message_box->setWindowTitle("Validation Error");
	message_box->setIcon(QMessageBox::Critical);
	message_box->setText(message);
	message_box->exec();
}

StandardDatastorePanel::StandardDatastorePanel(QWidget* parent, const QString& api_key) : BaseDatastorePanel(parent, api_key)
{
	// Left bar
	{
		select_datastore_show_hidden_check = new QCheckBox{ "Show hidden", select_datastore_group };
		connect(select_datastore_show_hidden_check, &QCheckBox::stateChanged, this, &StandardDatastorePanel::refresh_datastore_list);

		select_datastore_fetch_button = new QPushButton{ "Fetch datastores", select_datastore_group };
		connect(select_datastore_fetch_button, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_fetch_datastores);

		QLayout* group_layout = select_datastore_group->layout();
		group_layout->addWidget(select_datastore_show_hidden_check);
		group_layout->addWidget(select_datastore_fetch_button);
	}

	// Main panel
	{
		// Search params
		{
			QLabel* const search_datastore_key_name_label = new QLabel{ "Key prefix:", search_params_widget };

			search_datastore_key_prefix_edit = new QLineEdit{ search_params_widget };
			connect(search_datastore_key_prefix_edit, &QLineEdit::textChanged, this, &StandardDatastorePanel::handle_search_text_changed);

			QLayout* main_top_layout = search_params_widget->layout();

			main_top_layout->addWidget(search_datastore_key_name_label);
			main_top_layout->addWidget(search_datastore_key_prefix_edit);
		}

		// Search submit buttons
		{
			find_all_button = new QPushButton{ "Find all", search_submit_widget };
			connect(find_all_button, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_find_all);

			find_prefix_button = new QPushButton{ "Find prefix match", search_submit_widget };
			connect(find_prefix_button, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_find_prefix);

			QBoxLayout* const search_submit_layout = dynamic_cast<QBoxLayout*>(search_submit_widget->layout());
			search_submit_layout->insertWidget(0, find_all_button);
			search_submit_layout->insertWidget(1, find_prefix_button);
		}

		{
			QWidget* right_read_buttons = new QWidget{ search_panel };
			{
				view_entry_button = new QPushButton{ "View entry...", right_read_buttons };
				connect(view_entry_button, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_view_entry);

				view_versions_button = new QPushButton{ "View versions...", right_read_buttons };
				connect(view_versions_button, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_view_versions);

				QHBoxLayout* right_read_layout = new QHBoxLayout{ right_read_buttons };
				right_read_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
				right_read_layout->addWidget(view_entry_button);
				right_read_layout->addWidget(view_versions_button);
			}

			QFrame* right_separator = new QFrame{ search_panel };
			right_separator->setFrameShape(QFrame::HLine);
			right_separator->setFrameShadow(QFrame::Sunken);

			QWidget* right_edit_buttons = new QWidget{ search_panel };
			{
				edit_entry_button = new QPushButton{ "Edit entry...", right_edit_buttons };
				connect(edit_entry_button, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_edit_entry);

				delete_entry_button = new QPushButton{ "Delete entry", right_edit_buttons };
				connect(delete_entry_button, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_delete_entry);

				QHBoxLayout* right_edit_layout = new QHBoxLayout{ right_edit_buttons };
				right_edit_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
				right_edit_layout->addWidget(edit_entry_button);
				right_edit_layout->addWidget(delete_entry_button);
			}

			QLayout* search_layout = search_panel->layout();
			search_layout->addWidget(right_read_buttons);
			search_layout->addWidget(right_separator);
			search_layout->addWidget(right_edit_buttons);
		}

		QWidget* add_entry_panel = new QWidget{ main_tab_widget };
		{
			QWidget* add_entry_form = new QWidget{ add_entry_panel };
			{
				add_datastore_name_edit = new QLineEdit{ add_entry_form };
				connect(add_datastore_name_edit, &QLineEdit::textChanged, this, &StandardDatastorePanel::handle_add_entry_text_changed);

				add_datastore_scope_edit = new QLineEdit{ add_entry_form };
				add_datastore_scope_edit->setPlaceholderText("global");
				connect(add_datastore_scope_edit, &QLineEdit::textChanged, this, &StandardDatastorePanel::handle_add_entry_text_changed);

				add_datastore_key_name_edit = new QLineEdit{ add_entry_form };
				connect(add_datastore_key_name_edit, &QLineEdit::textChanged, this, &StandardDatastorePanel::handle_add_entry_text_changed);

				add_entry_type_combo = new QComboBox{ add_entry_form };
				add_entry_type_combo->setSizePolicy(QSizePolicy{ QSizePolicy::Expanding, QSizePolicy::Preferred });
				add_entry_type_combo->addItem(get_enum_string(DatastoreEntryType::Json), static_cast<int>(DatastoreEntryType::Json));
				add_entry_type_combo->addItem(get_enum_string(DatastoreEntryType::String), static_cast<int>(DatastoreEntryType::String));
				add_entry_type_combo->addItem(get_enum_string(DatastoreEntryType::Number), static_cast<int>(DatastoreEntryType::Number));
				add_entry_type_combo->addItem(get_enum_string(DatastoreEntryType::Bool), static_cast<int>(DatastoreEntryType::Bool));

				QTabWidget* data_tab_widget = new QTabWidget{ add_entry_form };
				{
					add_entry_data_edit = new QTextEdit{ data_tab_widget };
					add_entry_data_edit->setAcceptRichText(false);
					connect(add_entry_data_edit, &QTextEdit::textChanged, this, &StandardDatastorePanel::handle_add_entry_text_changed);

					add_entry_userids_edit = new QTextEdit{ data_tab_widget };
					add_entry_userids_edit->setAcceptRichText(false);

					add_entry_attributes_edit = new QTextEdit{ data_tab_widget };
					add_entry_attributes_edit->setAcceptRichText(false);

					data_tab_widget->addTab(add_entry_data_edit, "Data");
					data_tab_widget->addTab(add_entry_userids_edit, "User IDs");
					data_tab_widget->addTab(add_entry_attributes_edit, "Attributes");
				}

				add_entry_submit_button = new QPushButton{ "Submit", add_entry_form };
				add_entry_submit_button->setSizePolicy(QSizePolicy{ QSizePolicy::Expanding, QSizePolicy::Preferred });
				connect(add_entry_submit_button, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_submit_new_entry);

				QFormLayout* add_entry_form_layout = new QFormLayout{ add_entry_form };
				add_entry_form_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
				add_entry_form_layout->addRow("Datastore", add_datastore_name_edit);
				add_entry_form_layout->addRow("Scope", add_datastore_scope_edit);
				add_entry_form_layout->addRow("Key", add_datastore_key_name_edit);
				add_entry_form_layout->addRow("Type", add_entry_type_combo);
				add_entry_form_layout->addRow("Data", data_tab_widget);
				add_entry_form_layout->addRow("", add_entry_submit_button);
			}

			QVBoxLayout* add_entry_layout = new QVBoxLayout{ add_entry_panel };
			add_entry_layout->addWidget(add_entry_form);
		}
		main_tab_widget->addTab(add_entry_panel, "Add Entry");
	}

	clear_model();
	handle_add_entry_text_changed();
}

void StandardDatastorePanel::selected_universe_changed()
{
	BaseDatastorePanel::selected_universe_changed();

	const UniverseProfile* const profile = UserProfile::get_selected_universe();
	const bool enabled = profile != nullptr;
	select_datastore_fetch_button->setEnabled(enabled);
	select_datastore_show_hidden_check->setEnabled(enabled);
	if (profile != nullptr)
	{
		select_datastore_show_hidden_check->setChecked(profile->get_show_hidden_standard_datastores());
	}
	else
	{
		select_datastore_show_hidden_check->setChecked(false);
	}
	handle_add_entry_text_changed();
	handle_search_text_changed();
}

void StandardDatastorePanel::set_datastore_entry_model(StandardDatastoreEntryQTableModel* const entry_model)
{
	if (entry_model)
	{
		datastore_entry_tree->setModel(entry_model);
	}
	else
	{
		datastore_entry_tree->setModel(new StandardDatastoreEntryQTableModel{ datastore_entry_tree, std::vector<StandardDatastoreEntryName>{} });
	}
	datastore_entry_tree->setColumnWidth(0, 140);
	connect(datastore_entry_tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &StandardDatastorePanel::handle_selected_datastore_entry_changed);
	handle_selected_datastore_entry_changed();
}

std::vector<StandardDatastoreEntryName> StandardDatastorePanel::get_selected_entries() const
{
	if (StandardDatastoreEntryQTableModel* const entry_model = dynamic_cast<StandardDatastoreEntryQTableModel*>(datastore_entry_tree->model()))
	{
		if (QItemSelectionModel* const select_model = datastore_entry_tree->selectionModel())
		{
			if (select_model->selectedRows().count() > 0)
			{
				std::vector<StandardDatastoreEntryName> result;
				QList<QModelIndex> indices{ select_model->selectedRows() };
				for (const QModelIndex& this_index : indices)
				{
					if (this_index.isValid())
					{
						if (std::optional<StandardDatastoreEntryName> opt_entry = entry_model->get_entry(this_index.row()))
						{
							result.push_back(*opt_entry);
						}
					}
				}
				return result;
			}
		}
	}

	return std::vector<StandardDatastoreEntryName>{};
}

void StandardDatastorePanel::view_entry(const QModelIndex& index)
{
	if (index.isValid())
	{
		if (StandardDatastoreEntryQTableModel* model = dynamic_cast<StandardDatastoreEntryQTableModel*>(datastore_entry_tree->model()))
		{
			std::optional<StandardDatastoreEntryName> opt_entry = model->get_entry(index.row());
			if (opt_entry)
			{
				const auto req = std::make_shared<GetStandardDatastoreEntryDetailsRequest>(api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key());
				OperationInProgressDialog diag{ this, req };
				diag.exec();

				const std::optional<StandardDatastoreEntryFull> opt_details = req->get_details();
				if (opt_details)
				{
					ViewDatastoreEntryWindow* const view_entry_window = new ViewDatastoreEntryWindow{ this, api_key, *opt_details };
					view_entry_window->setWindowModality(Qt::WindowModality::ApplicationModal);
					view_entry_window->show();
				}
				else
				{
					QMessageBox* const msg_box = new QMessageBox{ this };
					msg_box->setWindowTitle("Not Found");
					msg_box->setText("This entry does not exist or has been deleted.");
					msg_box->exec();
				}
			}
		}
	}
}

void StandardDatastorePanel::view_versions(const QModelIndex& index)
{
	if (index.isValid())
	{
		if (StandardDatastoreEntryQTableModel* model = dynamic_cast<StandardDatastoreEntryQTableModel*>(datastore_entry_tree->model()))
		{
			std::optional<StandardDatastoreEntryName> opt_entry = model->get_entry(index.row());
			if (opt_entry)
			{
				const auto req = std::make_shared<GetStandardDatastoreEntryVersionListRequest>(api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key());
				OperationInProgressDialog diag{ this, req };
				diag.exec();

				if (req->get_versions().size() > 0)
				{
					ViewDatastoreEntryVersionsWindow* view_versions_window = new ViewDatastoreEntryVersionsWindow{ this, api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key(), req->get_versions() };
					view_versions_window->setWindowModality(Qt::WindowModality::ApplicationModal);
					view_versions_window->show();
				}
			}
		}
	}
}

void StandardDatastorePanel::edit_entry(const QModelIndex& index)
{
	if (index.isValid())
	{
		if (StandardDatastoreEntryQTableModel* model = dynamic_cast<StandardDatastoreEntryQTableModel*>(datastore_entry_tree->model()))
		{
			std::optional<StandardDatastoreEntryName> opt_entry = model->get_entry(index.row());
			if (opt_entry)
			{
				const auto req = std::make_shared<GetStandardDatastoreEntryDetailsRequest>(api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key());
				OperationInProgressDialog diag{ this, req };
				diag.exec();

				const std::optional<StandardDatastoreEntryFull> opt_details = req->get_details();
				if (opt_details)
				{
					ViewDatastoreEntryWindow* edit_entry_window = new ViewDatastoreEntryWindow{ this, api_key, *opt_details, ViewEditMode::Edit };
					edit_entry_window->setWindowModality(Qt::WindowModality::ApplicationModal);
					edit_entry_window->show();
				}
			}
		}
	}
}

void StandardDatastorePanel::delete_entry(const QModelIndex& index)
{
	if (index.isValid())
	{
		if (StandardDatastoreEntryQTableModel* model = dynamic_cast<StandardDatastoreEntryQTableModel*>(datastore_entry_tree->model()))
		{
			std::optional<StandardDatastoreEntryName> opt_entry = model->get_entry(index.row());
			if (opt_entry)
			{
				ConfirmChangeDialog* confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::Delete };
				bool confirmed = static_cast<bool>(confirm_dialog->exec());
				if (confirmed == false)
				{
					return;
				}

				const auto req = std::make_shared<DeleteStandardDatastoreEntryRequest>(api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key());
				OperationInProgressDialog diag{ this, req };
				diag.exec();
			}
		}
	}
}


void StandardDatastorePanel::delete_entry_list(const std::vector<StandardDatastoreEntryName>& entry_list)
{
	if (entry_list.size() > 1)
	{
		ConfirmChangeDialog* confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::MultiDelete };
		bool confirmed = static_cast<bool>(confirm_dialog->exec());
		if (confirmed == false)
		{
			return;
		}

		std::vector<std::shared_ptr<DataRequest>> request_list;
		for (const StandardDatastoreEntryName& this_entry : entry_list)
		{
			std::shared_ptr<DeleteStandardDatastoreEntryRequest> this_request =
				std::make_shared<DeleteStandardDatastoreEntryRequest>(api_key, this_entry.get_universe_id(), this_entry.get_datastore_name(), this_entry.get_scope(), this_entry.get_key());

			request_list.push_back(this_request);
		}

		OperationInProgressDialog diag{ this, request_list };
		diag.exec();
	}
}

void StandardDatastorePanel::handle_datastore_entry_double_clicked(const QModelIndex& index)
{
	view_entry(index);
}

void StandardDatastorePanel::handle_search_text_changed()
{
	BaseDatastorePanel::handle_search_text_changed();
	const UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
	const bool find_all_enabled = search_datastore_name_edit->text().size() > 0 && selected_universe;
	const bool find_prefix_enabled = find_all_enabled && search_datastore_key_prefix_edit->text().size() > 0;
	find_all_button->setEnabled(find_all_enabled);
	find_prefix_button->setEnabled(find_prefix_enabled);
}

void StandardDatastorePanel::handle_selected_datastore_changed()
{
	QList<QListWidgetItem*> selected = select_datastore_list->selectedItems();
	if (selected.size() == 1)
	{
		add_datastore_name_edit->setText(selected.first()->text());
	}
	BaseDatastorePanel::handle_selected_datastore_changed();
}

void StandardDatastorePanel::handle_selected_datastore_entry_changed()
{
	size_t count = 0;
	bool single_selected = false;
	bool multi_selected = false;
	if (const QItemSelectionModel* select_model = datastore_entry_tree->selectionModel())
	{
		count = select_model->selectedRows().count();
		single_selected = count == 1;
		multi_selected = count > 1;
	}
	view_entry_button->setEnabled(single_selected);
	view_versions_button->setEnabled(single_selected);
	edit_entry_button->setEnabled(single_selected);
	delete_entry_button->setEnabled(single_selected || multi_selected);
	if (multi_selected)
	{
		delete_entry_button->setText(QString{ "Delete %1 entries" }.arg(count));
	}
	else
	{
		delete_entry_button->setText("Delete entry");
	}
}

void StandardDatastorePanel::handle_add_entry_text_changed()
{
	const UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
	const bool submit_enabled = selected_universe && add_datastore_name_edit->text().size() > 0 && add_datastore_key_name_edit->text().size() > 0 && add_entry_data_edit->toPlainText().size() > 0;
	add_entry_submit_button->setEnabled(submit_enabled);
}


void StandardDatastorePanel::pressed_right_click_datastore_list(const QPoint& pos)
{
	const QModelIndex the_index = select_datastore_list->indexAt(pos);
	if (the_index.isValid())
	{
		const UniverseProfile* const this_universe = UserProfile::get_selected_universe();
		if (this_universe)
		{
			if (QListWidgetItem* the_item = select_datastore_list->item(the_index.row()))
			{
				QString the_datastore_name = the_item->text();

				QMenu* context_menu = new QMenu{ select_datastore_list };
				{
					QAction* copy_name = new QAction{ "Copy name", context_menu };
					connect(copy_name, &QAction::triggered, [the_datastore_name]() {
						QClipboard* clipboard = QGuiApplication::clipboard();
						clipboard->setText(the_datastore_name);
					});

					QAction* hide_unhide_action = nullptr;
					if (this_universe->get_hidden_datastore_set().count(the_datastore_name))
					{
						hide_unhide_action = new QAction{ "Unhide datastore", context_menu };
						connect(hide_unhide_action, &QAction::triggered, [the_datastore_name]() {
							if (UniverseProfile* universe_profile = UserProfile::get_selected_universe())
							{
								universe_profile->remove_hidden_datastore(the_datastore_name);
							}
						});
					}
					else
					{
						hide_unhide_action = new QAction{ "Hide datastore", context_menu };
						connect(hide_unhide_action, &QAction::triggered, [the_datastore_name]() {
							if (UniverseProfile* universe_profile = UserProfile::get_selected_universe())
							{
								universe_profile->add_hidden_datastore(the_datastore_name);
							}
						});
					}

					context_menu->addAction(copy_name);
					context_menu->addSeparator();
					context_menu->addAction(hide_unhide_action);
				}

				context_menu->exec(select_datastore_list->mapToGlobal(pos));
				context_menu->deleteLater();
			}
		}
	}
}

void StandardDatastorePanel::pressed_right_click_entry_list(const QPoint& pos)
{
	const QModelIndex the_index = datastore_entry_tree->indexAt(pos);
	if (the_index.isValid())
	{
		StandardDatastoreEntryQTableModel* const the_model = dynamic_cast<StandardDatastoreEntryQTableModel*>(datastore_entry_tree->model());
		if (auto the_entry = the_model->get_entry(the_index.row()))
		{
			QString the_key_name = the_entry->get_key();
			QMenu* context_menu = new QMenu{ datastore_entry_tree };
			{
				QAction* copy_action = new QAction{ "Copy name", context_menu };
				connect(copy_action, &QAction::triggered, [the_key_name]() {
					QClipboard* clipboard = QGuiApplication::clipboard();
					clipboard->setText(the_key_name);
					});

				QAction* view_action = new QAction{ "View entry...", context_menu };
				connect(view_action, &QAction::triggered, [this, the_index]() {
					view_entry(the_index);
					});

				QAction* versions_action = new QAction{ "View versions...", context_menu };
				connect(versions_action, &QAction::triggered, [this, the_index]() {
					view_versions(the_index);
					});

				QAction* edit_action = new QAction{ "Edit entry...", context_menu };
				connect(edit_action, &QAction::triggered, [this, the_index]() {
					edit_entry(the_index);
					});

				QAction* delete_action = new QAction{ "Delete entry", context_menu };
				connect(delete_action, &QAction::triggered, [this, the_index]() {
					delete_entry(the_index);
					});

				context_menu->addAction(copy_action);
				context_menu->addSeparator();
				context_menu->addAction(view_action);
				context_menu->addAction(versions_action);
				context_menu->addSeparator();
				context_menu->addAction(edit_action);
				context_menu->addAction(delete_action);
			}

			context_menu->exec(datastore_entry_tree->mapToGlobal(pos));
			context_menu->deleteLater();
		}
	}
}

void StandardDatastorePanel::pressed_delete_entry()
{
	QModelIndex single_entry = get_selected_single_index();
	if (single_entry.isValid())
	{
		delete_entry(single_entry);
	}

	std::vector<StandardDatastoreEntryName> entry_list = get_selected_entries();
	if (entry_list.size() > 1)
	{
		delete_entry_list(entry_list);
	}
}

void StandardDatastorePanel::pressed_edit_entry()
{
	edit_entry(get_selected_single_index());
}

void StandardDatastorePanel::pressed_fetch_datastores()
{
	const UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
	if (selected_universe)
	{
		const long long universe_id = selected_universe->get_universe_id();
		if (universe_id > 0)
		{
			const auto req = std::make_shared<GetStandardDatastoreListRequest>(api_key, universe_id);
			OperationInProgressDialog diag{ this, req };
			diag.exec();

			select_datastore_list->clear();
			for (QString this_name : req->get_datastore_names())
			{
				QListWidgetItem* this_item = new QListWidgetItem(select_datastore_list);
				this_item->setText(this_name);
				select_datastore_list->addItem(this_item);
			}
			refresh_datastore_list();

			// TODO: This should store the names in a complete list, then populate the widget based on the filter
		}
	}
}

void StandardDatastorePanel::pressed_find_all()
{
	const UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
	if (selected_universe && search_datastore_name_edit->text().trimmed().size() > 0)
	{
		const long long universe_id = selected_universe->get_universe_id();
		if (universe_id > 0)
		{
			QString datastore_name = search_datastore_name_edit->text().trimmed();
			QString scope = search_datastore_scope_edit->text().trimmed();

			if (scope.size() == 0)
			{
				scope = "global";
			}

			const size_t result_limit = find_limit_edit->text().trimmed().toULongLong();

			const auto req = std::make_shared<GetStandardDatastoreEntryListRequest>(api_key, universe_id, datastore_name, scope, "");
			if (result_limit > 0)
			{
				req->set_result_limit(result_limit);
			}
			OperationInProgressDialog diag{ this, req };
			diag.exec();

			StandardDatastoreEntryQTableModel* datastore_model = new StandardDatastoreEntryQTableModel{ datastore_entry_tree, req->get_datastore_entries() };
			set_datastore_entry_model(datastore_model);
		}
	}
}

void StandardDatastorePanel::pressed_find_prefix()
{
	const UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
	if (selected_universe && search_datastore_name_edit->text().trimmed().size() > 0)
	{
		const long long universe_id = selected_universe->get_universe_id();
		if (universe_id > 0)
		{
			QString datastore_name = search_datastore_name_edit->text().trimmed();
			QString scope = search_datastore_scope_edit->text().trimmed();
			QString key_name = search_datastore_key_prefix_edit->text().trimmed();

			if (scope.size() == 0)
			{
				scope = "global";
			}

			const size_t result_limit = find_limit_edit->text().trimmed().toULongLong();

			const auto req = std::make_shared<GetStandardDatastoreEntryListRequest>(api_key, universe_id, datastore_name, scope, key_name);
			if (result_limit > 0)
			{
				req->set_result_limit(result_limit);
			}
			OperationInProgressDialog diag{ this, req };
			diag.exec();

			StandardDatastoreEntryQTableModel* datastore_model = new StandardDatastoreEntryQTableModel{ datastore_entry_tree, req->get_datastore_entries() };
			set_datastore_entry_model(datastore_model);
		}
	}
}

void StandardDatastorePanel::pressed_submit_new_entry()
{
	const QString data_raw = add_entry_data_edit->toPlainText();
	const QString userids_raw = add_entry_userids_edit->toPlainText().trimmed();
	const QString attributes_str_raw = add_entry_attributes_edit->toPlainText().trimmed();

	std::optional<QString> attributes;
	if (attributes_str_raw.size() > 0)
	{
		attributes = condense_json(attributes_str_raw);
	}

	const DatastoreEntryType data_type = static_cast<DatastoreEntryType>(add_entry_type_combo->currentData().toInt());
	{
		bool data_valid = false;
		switch (data_type)
		{
		case DatastoreEntryType::Error:
			data_valid = false;
			break;
		case DatastoreEntryType::Bool:
			data_valid = DataValidator::is_bool(data_raw);
			break;
		case DatastoreEntryType::Number:
			data_valid = DataValidator::is_number(data_raw);
			break;
		case DatastoreEntryType::String:
			data_valid = true;
			break;
		case DatastoreEntryType::Json:
			data_valid = DataValidator::is_json(data_raw);
			break;
		}

		if (data_valid == false)
		{
			show_validation_error(this, QString{ "New data is not a valid " } + get_enum_string(data_type) + ".");
			return;
		}
	}

	if (userids_raw != "" && DataValidator::is_json_array(userids_raw) == false)
	{
		show_validation_error(this, "New user list is not empty or a valid Json array.");
		return;
	}

	if (attributes_str_raw != "" && DataValidator::is_json(attributes_str_raw) == false)
	{
		show_validation_error(this, "New attributes is not empty or a valid Json object.");
		return;
	}

	ConfirmChangeDialog* confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::Update };
	bool confirmed = static_cast<bool>(confirm_dialog->exec());
	if (confirmed == false)
	{
		return;
	}

	if (UserProfile::get_selected_universe() == nullptr)
	{
		return;
	}

	const long long universe_id = UserProfile::get_selected_universe()->get_universe_id();
	const QString datastore_name = add_datastore_name_edit->text();
	const QString scope = add_datastore_scope_edit->text();
	const QString key_name = add_datastore_key_name_edit->text();

	const std::optional<QString> userids = condense_json(userids_raw);
	std::optional<QString> data_formatted;
	if (data_type == DatastoreEntryType::Json)
	{
		data_formatted = condense_json(data_raw);
	}
	else if (data_type == DatastoreEntryType::String)
	{
		data_formatted = encode_json_string(data_raw);
	}
	else
	{
		data_formatted = data_raw;
	}

	if (data_formatted.has_value() == false)
	{
		show_validation_error(this, "Failed to format data. This probably shouldn't happen.");
		return;
	}

	const auto post_req = std::make_shared<PostStandardDatastoreEntryRequest>(api_key, universe_id, datastore_name, scope, key_name, userids, attributes, *data_formatted);
	OperationInProgressDialog diag{ this, post_req };
	diag.exec();

	if (post_req->get_success())
	{
		add_entry_data_edit->clear();
		add_entry_userids_edit->clear();
		add_entry_attributes_edit->clear();
	}
}

void StandardDatastorePanel::pressed_view_entry()
{
	view_entry(get_selected_single_index());
}

void StandardDatastorePanel::pressed_view_versions()
{
	view_versions(get_selected_single_index());
}

void StandardDatastorePanel::clear_model()
{
	set_datastore_entry_model(nullptr);
	handle_selected_datastore_entry_changed();
}

void StandardDatastorePanel::refresh_datastore_list()
{
	const UniverseProfile* const selected_universe = UserProfile::get_selected_universe();
	if (selected_universe)
	{
		const bool show_hidden = select_datastore_show_hidden_check->isChecked();
		for (int i = 0; i < select_datastore_list->count(); i++)
		{
			QListWidgetItem* const this_item = select_datastore_list->item(i);
			const bool is_hidden = static_cast<bool>(selected_universe->get_hidden_datastore_set().count(this_item->text()));
			const bool matches_filter = this_item->text().contains(select_datastore_name_filter_edit->text());
			this_item->setHidden((!show_hidden && is_hidden) || !matches_filter);
		}
	}
}
