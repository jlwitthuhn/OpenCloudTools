#include "panel_datastore_explore.h"

#include <cstddef>

#include <memory>
#include <optional>
#include <set>
#include <vector>

#include <Qt>
#include <QAbstractItemModel>
#include <QAction>
#include <QCheckBox>
#include <QClipboard>
#include <QFrame>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMargins>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSizePolicy>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

#include "api_key.h"
#include "api_response.h"
#include "data_request.h"
#include "datastore_model.h"
#include "diag_confirm_change.h"
#include "diag_operation_in_progress.h"
#include "user_settings.h"
#include "util_enum.h"
#include "window_datastore_entry_versions_view.h"
#include "window_datastore_entry_view.h"

ExploreDatastorePanel::ExploreDatastorePanel(QWidget* parent, const QString& api_key) :
	QWidget{ parent },
	api_key{ api_key }
{
	connect(UserSettings::get().get(), &UserSettings::hidden_datastores_changed, this, &ExploreDatastorePanel::handle_show_hidden_datastores_toggled);

	QWidget* left_bar_widget = new QWidget{ this };
	{
		QSizePolicy left_size_policy{ QSizePolicy::Preferred, QSizePolicy::Preferred };
		left_size_policy.setHorizontalStretch(5);
		left_bar_widget->setSizePolicy(left_size_policy);

		QGroupBox* select_datastore_widget = new QGroupBox{ "Datastores", left_bar_widget };
		{
			select_datastore_list = new QListWidget{ select_datastore_widget };
			select_datastore_list->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
			connect(select_datastore_list, &QListWidget::customContextMenuRequested, this, &ExploreDatastorePanel::pressed_right_click_datastore_list);
			connect(select_datastore_list, &QListWidget::itemSelectionChanged, this, &ExploreDatastorePanel::handle_selected_datastore_changed);

			select_datastore_show_hidden_check = new QCheckBox{ "Show hidden", select_datastore_widget};
			connect(select_datastore_show_hidden_check, &QCheckBox::stateChanged, this, &ExploreDatastorePanel::handle_show_hidden_datastores_toggled);

			select_datastore_fetch_button = new QPushButton{ "Fetch datastores", select_datastore_widget };
			connect(select_datastore_fetch_button, &QPushButton::clicked, this, &ExploreDatastorePanel::pressed_fetch_datastores);

			QVBoxLayout* select_datastore_layout = new QVBoxLayout{ select_datastore_widget };
			select_datastore_layout->addWidget(select_datastore_list);
			select_datastore_layout->addWidget(select_datastore_show_hidden_check);
			select_datastore_layout->addWidget(select_datastore_fetch_button);
		}

		QVBoxLayout* left_bar_layout = new QVBoxLayout{ left_bar_widget };
		left_bar_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		left_bar_layout->addWidget(select_datastore_widget);
	}

	QWidget* right_bar_widget = new QWidget{ this };
	{
		QSizePolicy right_size_policy{ QSizePolicy::Preferred, QSizePolicy::Preferred };
		right_size_policy.setHorizontalStretch(7);
		right_bar_widget->setSizePolicy(right_size_policy);

		QGroupBox* right_group_box = new QGroupBox{ "Search", right_bar_widget };
		{
			QWidget* right_top_widget = new QWidget{ right_group_box };
			{
				QLabel* datastore_name_label = new QLabel{ "Datastore:", right_top_widget };

				datastore_name_edit = new QLineEdit{ right_top_widget };
				connect(datastore_name_edit, &QLineEdit::textChanged, this, &ExploreDatastorePanel::handle_search_text_changed);

				QLabel* datastore_scope_label = new QLabel{ "Scope:", right_top_widget };

				datastore_scope_edit = new QLineEdit{ right_top_widget };
				connect(datastore_scope_edit, &QLineEdit::textChanged, this, &ExploreDatastorePanel::handle_search_text_changed);

				QLabel* datastore_key_name_label = new QLabel{ "Key prefix:", right_top_widget };

				datastore_key_name_edit = new QLineEdit{ right_top_widget };
				connect(datastore_key_name_edit, &QLineEdit::textChanged, this, &ExploreDatastorePanel::handle_search_text_changed);

				QHBoxLayout* right_top_layout = new QHBoxLayout{ right_top_widget };
				right_top_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
				right_top_layout->addWidget(datastore_name_label);
				right_top_layout->addWidget(datastore_name_edit);
				right_top_layout->addWidget(datastore_scope_label);
				right_top_layout->addWidget(datastore_scope_edit);
				right_top_layout->addWidget(datastore_key_name_label);
				right_top_layout->addWidget(datastore_key_name_edit);
			}

			QWidget* right_top_button_widget = new QWidget{ right_group_box };
			{
				find_all_button = new QPushButton{ "Find all", right_top_button_widget };
				connect(find_all_button, &QPushButton::clicked, this, &ExploreDatastorePanel::pressed_find_all);

				find_prefix_button = new QPushButton{ "Find prefix match", right_top_button_widget };
				connect(find_prefix_button, &QPushButton::clicked, this, &ExploreDatastorePanel::pressed_find_prefix);

				QLabel* find_limit_label = new QLabel{ "Limit:", right_top_button_widget };
				find_limit_label->setSizePolicy(QSizePolicy{ QSizePolicy::Fixed, QSizePolicy::Fixed });

				find_limit_edit = new QLineEdit{ right_top_button_widget };
				find_limit_edit->setText("1200");
				find_limit_edit->setFixedWidth(60);

				QHBoxLayout* right_top_button_layout = new QHBoxLayout{ right_top_button_widget };
				right_top_button_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
				right_top_button_layout->addWidget(find_all_button);
				right_top_button_layout->addWidget(find_prefix_button);
				right_top_button_layout->addWidget(find_limit_label);
				right_top_button_layout->addWidget(find_limit_edit);
			}

			datastore_entry_tree = new QTreeView{ right_group_box };
			connect(datastore_entry_tree, &QTreeView::doubleClicked, this, &ExploreDatastorePanel::handle_datastore_entry_double_clicked);
			connect(datastore_entry_tree, &QTreeView::pressed, this, &ExploreDatastorePanel::handle_selected_datastore_entry_changed);

			QWidget* right_read_buttons = new QWidget{ right_group_box };
			{
				view_entry_button = new QPushButton{ "View entry...", right_read_buttons };
				connect(view_entry_button, &QPushButton::clicked, this, &ExploreDatastorePanel::pressed_view_entry);

				view_versions_button = new QPushButton{ "View versions...", right_read_buttons };
				connect(view_versions_button, &QPushButton::clicked, this, &ExploreDatastorePanel::pressed_view_versions);

				QHBoxLayout* right_read_layout = new QHBoxLayout{ right_read_buttons };
				right_read_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
				right_read_layout->addWidget(view_entry_button);
				right_read_layout->addWidget(view_versions_button);
			}

			QFrame* right_separator = new QFrame{ right_group_box };
			right_separator->setFrameShape(QFrame::HLine);
			right_separator->setFrameShadow(QFrame::Sunken);

			QWidget* right_edit_buttons = new QWidget{ right_group_box };
			{
				edit_entry_button = new QPushButton{ "Edit entry...", right_edit_buttons };
				connect(edit_entry_button, &QPushButton::clicked, this, &ExploreDatastorePanel::pressed_edit_entry);

				delete_entry_button = new QPushButton{ "Delete entry", right_edit_buttons };
				connect(delete_entry_button, &QPushButton::clicked, this, &ExploreDatastorePanel::pressed_delete_entry);

				QHBoxLayout* right_edit_layout = new QHBoxLayout{ right_edit_buttons };
				right_edit_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
				right_edit_layout->addWidget(edit_entry_button);
				right_edit_layout->addWidget(delete_entry_button);
			}

			QVBoxLayout* right_box_layout = new QVBoxLayout{ right_group_box };
			right_box_layout->addWidget(right_top_widget);
			right_box_layout->addWidget(right_top_button_widget);
			right_box_layout->addWidget(datastore_entry_tree);
			right_box_layout->addWidget(right_read_buttons);
			right_box_layout->addWidget(right_separator);
			right_box_layout->addWidget(right_edit_buttons);
		}

		QVBoxLayout* right_bar_layout = new QVBoxLayout{ right_bar_widget };
		right_bar_layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
		right_bar_layout->addWidget(right_group_box);
	}

	QSplitter* splitter = new QSplitter{ this };
	splitter->setChildrenCollapsible(false);
	splitter->addWidget(left_bar_widget);
	splitter->addWidget(right_bar_widget);

	QHBoxLayout* layout = new QHBoxLayout{ this };
	layout->addWidget(splitter);

	handle_search_text_changed();
	handle_selected_datastore_changed();
	handle_selected_datastore_entry_changed();
}

void ExploreDatastorePanel::selected_universe_changed()
{
	select_datastore_list->clear();
	select_datastore_fetch_button->setEnabled(UserSettings::get()->get_selected_universe().has_value());
	handle_search_text_changed();
	datastore_entry_tree->setModel(nullptr);
	handle_selected_datastore_entry_changed();
}

void ExploreDatastorePanel::view_entry(const QModelIndex& index)
{
	if (index.isValid())
	{
		if (DatastoreEntryModel* model = dynamic_cast<DatastoreEntryModel*>(datastore_entry_tree->model()))
		{
			std::optional<StandardDatastoreEntry> opt_entry = model->get_entry(index.row());
			if (opt_entry)
			{
				GetStandardDatastoreEntryRequest req{ nullptr, api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key() };
				OperationInProgressDialog diag{ this, &req };
				req.send_request();
				diag.exec();

				std::optional<GetStandardDatastoreEntryDetailsResponse> opt_response = req.get_response();
				if (opt_response)
				{
					ViewDatastoreEntryWindow* view_entry_window = new ViewDatastoreEntryWindow{ this, api_key, opt_response->get_details() };
					view_entry_window->setWindowModality(Qt::WindowModality::ApplicationModal);
					view_entry_window->show();
				}
				else
				{
					QMessageBox* msg_box = new QMessageBox{ this };
					msg_box->setWindowTitle("Not Found");
					msg_box->setText("This entry does not exist or has been deleted.");
					msg_box->exec();
				}
			}
		}
	}
}

void ExploreDatastorePanel::handle_datastore_entry_double_clicked(const QModelIndex& index)
{
	view_entry(index);
}

void ExploreDatastorePanel::handle_search_text_changed()
{
	const std::optional<UniverseProfile> selected_universe = UserSettings::get()->get_selected_universe();
	const bool find_all_enabled = datastore_name_edit->text().size() > 0 && selected_universe.has_value();
	const bool find_prefix_enabled = find_all_enabled && datastore_key_name_edit->text().size() > 0 && datastore_key_name_edit->text().size() > 0;
	find_all_button->setEnabled(find_all_enabled);
	find_prefix_button->setEnabled(find_prefix_enabled);
}

void ExploreDatastorePanel::handle_selected_datastore_changed()
{
	QList<QListWidgetItem*> selected = select_datastore_list->selectedItems();
	if (selected.size() == 1)
	{
		datastore_name_edit->setText(selected.first()->text());
	}
}

void ExploreDatastorePanel::handle_selected_datastore_entry_changed()
{
	const bool item_selected = datastore_entry_tree->model() != nullptr && datastore_entry_tree->currentIndex().isValid();
	view_entry_button->setEnabled(item_selected);
	view_versions_button->setEnabled(item_selected);
	edit_entry_button->setEnabled(item_selected);
	delete_entry_button->setEnabled(item_selected);
}

void ExploreDatastorePanel::handle_show_hidden_datastores_toggled()
{
	const std::optional<UniverseProfile> selected_universe = UserSettings::get()->get_selected_universe();
	if (selected_universe)
	{
		const bool show_hidden = select_datastore_show_hidden_check->isChecked();
		for (int i = 0; i < select_datastore_list->count(); i++)
		{
			QListWidgetItem* this_item = select_datastore_list->item(i);
			const bool hide = show_hidden ? false : static_cast<bool>( selected_universe->hidden_datastores().count(this_item->text()) );
			this_item->setHidden(hide);
		}
	}
}

void ExploreDatastorePanel::pressed_delete_entry()
{
	QModelIndex selected_index = datastore_entry_tree->currentIndex();
	if (selected_index.isValid())
	{
		if (DatastoreEntryModel* model = dynamic_cast<DatastoreEntryModel*>(datastore_entry_tree->model()))
		{
			std::optional<StandardDatastoreEntry> opt_entry = model->get_entry(selected_index.row());
			if (opt_entry)
			{
				ConfirmChangeDialog* confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::Delete };
				bool confirmed = static_cast<bool>(confirm_dialog->exec());
				if (confirmed == false)
				{
					return;
				}

				DeleteStandardDatastoreEntryRequest req{ nullptr, api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key() };
				OperationInProgressDialog diag{ this, &req };
				req.send_request();
				diag.exec();
			}
		}
	}
}

void ExploreDatastorePanel::pressed_edit_entry()
{
	QModelIndex selected_index = datastore_entry_tree->currentIndex();
	if (selected_index.isValid())
	{
		if (DatastoreEntryModel* model = dynamic_cast<DatastoreEntryModel*>(datastore_entry_tree->model()))
		{
			std::optional<StandardDatastoreEntry> opt_entry = model->get_entry(selected_index.row());
			if (opt_entry)
			{
				GetStandardDatastoreEntryRequest req{ nullptr, api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key() };
				OperationInProgressDialog diag{ this, &req };
				req.send_request();
				diag.exec();

				std::optional<GetStandardDatastoreEntryDetailsResponse> opt_response = req.get_response();
				if (opt_response)
				{
					ViewDatastoreEntryWindow* edit_entry_window = new ViewDatastoreEntryWindow{ this, api_key, opt_response->get_details(), ViewEditMode::Edit };
					edit_entry_window->setWindowModality(Qt::WindowModality::ApplicationModal);
					edit_entry_window->show();
				}
			}
		}
	}
}

void ExploreDatastorePanel::pressed_fetch_datastores()
{
	const std::optional<UniverseProfile> selected_universe = UserSettings::get()->get_selected_universe();
	if (selected_universe)
	{
		const long long universe_id = selected_universe->universe_id();
		if (universe_id > 0)
		{
			GetStandardDatastoresDataRequest req{ nullptr, api_key, universe_id };
			OperationInProgressDialog diag{ this, &req };
			req.send_request();
			diag.exec();

			select_datastore_list->clear();
			for (QString this_name : req.get_datastore_names())
			{
				QListWidgetItem* this_item = new QListWidgetItem(select_datastore_list);
				this_item->setText(this_name);
				select_datastore_list->addItem(this_item);
			}
			handle_show_hidden_datastores_toggled();
		}
	}
}

void ExploreDatastorePanel::pressed_find_all()
{
	const std::optional<UniverseProfile> selected_universe = UserSettings::get()->get_selected_universe();
	if (selected_universe && datastore_name_edit->text().trimmed().size() > 0)
	{
		const long long universe_id = selected_universe->universe_id();
		if (universe_id > 0)
		{
			QString datastore_name = datastore_name_edit->text().trimmed();
			QString scope = datastore_scope_edit->text().trimmed();
			QString key_name; // This is only used for find by prefix

			const size_t result_limit = find_limit_edit->text().trimmed().toULongLong();

			GetStandardDatastoreEntriesRequest req{ nullptr, api_key, universe_id, datastore_name, scope, key_name };
			if (result_limit > 0)
			{
				req.set_result_limit(result_limit);
			}
			OperationInProgressDialog diag{ this, &req };
			req.send_request();
			diag.exec();

			DatastoreEntryModel* datastore_model = new DatastoreEntryModel{ datastore_entry_tree, req.get_datastore_entries() };
			datastore_entry_tree->setModel(datastore_model);
			datastore_entry_tree->setColumnWidth(0, 280);

			handle_selected_datastore_entry_changed();
		}
	}
}

void ExploreDatastorePanel::pressed_find_prefix()
{
	const std::optional<UniverseProfile> selected_universe = UserSettings::get()->get_selected_universe();
	if (selected_universe && datastore_name_edit->text().trimmed().size() > 0)
	{
		const long long universe_id = selected_universe->universe_id();
		if (universe_id > 0)
		{
			QString datastore_name = datastore_name_edit->text().trimmed();
			QString scope = datastore_scope_edit->text().trimmed();
			QString key_name = datastore_key_name_edit->text().trimmed();

			if (scope.size() == 0)
			{
				scope = "global";
			}

			const size_t result_limit = find_limit_edit->text().trimmed().toULongLong();

			GetStandardDatastoreEntriesRequest req{ nullptr, api_key, universe_id, datastore_name, scope, key_name };
			if (result_limit > 0)
			{
				req.set_result_limit(result_limit);
			}
			OperationInProgressDialog diag{ this, &req };
			req.send_request();
			diag.exec();

			DatastoreEntryModel* datastore_model = new DatastoreEntryModel{ datastore_entry_tree, req.get_datastore_entries() };
			datastore_entry_tree->setModel(datastore_model);
			datastore_entry_tree->setColumnWidth(0, 280);

			handle_selected_datastore_entry_changed();
		}
	}
}

void ExploreDatastorePanel::pressed_right_click_datastore_list(const QPoint& pos)
{
	const QModelIndex the_index = select_datastore_list->indexAt(pos);
	if (the_index.isValid())
	{
		const std::optional<UniverseProfile> this_universe = UserSettings::get()->get_selected_universe();
		if (this_universe)
		{
			if (QListWidgetItem* the_item = select_datastore_list->item(the_index.row()))
			{
				QString the_datastore_name = the_item->text();

				QAction* copy_name = new QAction{ "Copy name", select_datastore_list };
				connect(copy_name, &QAction::triggered, [the_datastore_name]() {
					QClipboard* clipboard = QGuiApplication::clipboard();
					clipboard->setText(the_datastore_name);
					});

				QAction* hide_unhide_action = nullptr;
				if (this_universe->hidden_datastores().count(the_datastore_name))
				{
					hide_unhide_action = new QAction{ "Unhide datastore", select_datastore_list };
					connect(hide_unhide_action, &QAction::triggered, [the_datastore_name]() {
						UserSettings::get()->remove_hidden_datastore(the_datastore_name);
					});
				}
				else
				{
					hide_unhide_action = new QAction{ "Hide datastore", select_datastore_list };
					connect(hide_unhide_action, &QAction::triggered, [the_datastore_name]() {
						UserSettings::get()->add_hidden_datastore(the_datastore_name);
					});
				}

				QMenu* context_menu = new QMenu{ select_datastore_list };
				context_menu->addAction(copy_name);
				context_menu->addSeparator();
				context_menu->addAction(hide_unhide_action);

				context_menu->exec(select_datastore_list->mapToGlobal(pos));
			}
		}
	}
}

void ExploreDatastorePanel::pressed_view_entry()
{
	view_entry(datastore_entry_tree->currentIndex());
}

void ExploreDatastorePanel::pressed_view_versions()
{
	QModelIndex selected_index = datastore_entry_tree->currentIndex();
	if (selected_index.isValid())
	{
		if (DatastoreEntryModel* model = dynamic_cast<DatastoreEntryModel*>(datastore_entry_tree->model()))
		{
			std::optional<StandardDatastoreEntry> opt_entry = model->get_entry(selected_index.row());
			if (opt_entry)
			{
				GetStandardDatastoreEntryVersionsRequest req{ nullptr, api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key() };
				OperationInProgressDialog diag{ this, &req };
				req.send_request();
				diag.exec();

				if (req.get_versions().size() > 0)
				{
					ViewDatastoreEntryVersionsWindow* view_versions_window = new ViewDatastoreEntryVersionsWindow{ this, api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key(), req.get_versions() };
					view_versions_window->setWindowModality(Qt::WindowModality::ApplicationModal);
					view_versions_window->show();
				}
			}
		}
	}
}
