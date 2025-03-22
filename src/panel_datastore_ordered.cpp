#include "panel_datastore_ordered.h"

#include <cstddef>

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include <Qt>
#include <QtGlobal>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMargins>
#include <QMessageBox>
#include <QModelIndex>
#include <QPushButton>
#include <QSizePolicy>
#include <QSplitter>
#include <QTabWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

#include "assert.h"
#include "data_request.h"
#include "diag_confirm_change.h"
#include "diag_operation_in_progress.h"
#include "gui_constants.h"
#include "model_common.h"
#include "model_qt.h"
#include "profile.h"
#include "util_alert.h"
#include "util_validator.h"

OrderedDatastorePanel::OrderedDatastorePanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe) :
	QWidget{ parent },
	api_key { api_key },
	attached_universe{ universe }
{
	connect(&(UserProfile::get()), &UserProfile::show_datastore_filter_changed, this, &OrderedDatastorePanel::handle_show_datastore_filter_changed);

	QSplitter* const splitter = new QSplitter{ this };
	{
		QWidget* const panel_index = new QWidget{ splitter };
		{
			QGroupBox* const group_index = new QGroupBox{ "Datastores", panel_index };
			{
				list_datastore_index = new QListWidget{ group_index };
				list_datastore_index->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
				connect(list_datastore_index, &QListWidget::itemSelectionChanged, this, &OrderedDatastorePanel::handle_selected_datastore_changed);

				edit_datastore_index_filter = new QLineEdit{ group_index };
				edit_datastore_index_filter->setPlaceholderText("filter");
				edit_datastore_index_filter->setToolTip("Only datastore names matching this text box will be displayed.");
				connect(edit_datastore_index_filter, &QLineEdit::textChanged, this, &OrderedDatastorePanel::refresh_datastore_list);

				check_save_recent_datastores = new QCheckBox{ "Add used datastores", group_index };
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
				connect(check_save_recent_datastores, &QCheckBox::checkStateChanged, this, &OrderedDatastorePanel::handle_save_recent_datastores_toggled);
#else
				connect(check_save_recent_datastores, &QCheckBox::stateChanged, this, &OrderedDatastorePanel::handle_save_recent_datastores_toggled);
#endif

				button_remove_datastore = new QPushButton{ "Remove", group_index };
				connect(button_remove_datastore, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_remove_datastore);

				QVBoxLayout* const layout_group = new QVBoxLayout{ group_index };
				layout_group->addWidget(list_datastore_index);
				layout_group->addWidget(edit_datastore_index_filter);
				layout_group->addWidget(check_save_recent_datastores);
				layout_group->addWidget(button_remove_datastore);
			}
			QVBoxLayout* const layout_index = new QVBoxLayout{ panel_index };
			layout_index->setContentsMargins(QMargins{ 0, 0, 0, 0 });
			layout_index->addWidget(group_index);
		}

		QWidget* const panel_main = new QWidget{ splitter };
		{
			QTabWidget* const tab_widget_main = new QTabWidget{ panel_main };
			{
				QWidget* const panel_search = new QWidget{ tab_widget_main };
				{
					QWidget* const panel_search_params = new QWidget{ panel_search };
					{
						QLabel* const label_search_datastore_name = new QLabel{ "Datastore:", panel_search_params };

						edit_search_datastore_name = new QLineEdit{ panel_search_params };
						connect(edit_search_datastore_name, &QLineEdit::textChanged, this, &OrderedDatastorePanel::handle_search_text_changed);

						QLabel* const label_search_datastore_scope = new QLabel{ "Scope:", panel_search_params };

						edit_search_datastore_scope = new QLineEdit{ panel_search_params };
						edit_search_datastore_scope->setPlaceholderText("global");
						connect(edit_search_datastore_scope, &QLineEdit::textChanged, this, &OrderedDatastorePanel::handle_search_text_changed);

						QHBoxLayout* const layout = new QHBoxLayout{ panel_search_params };
						layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
						layout->addWidget(label_search_datastore_name);
						layout->addWidget(edit_search_datastore_name);
						layout->addWidget(label_search_datastore_scope);
						layout->addWidget(edit_search_datastore_scope);
					}

					QWidget* const panel_search_submit = new QWidget{ panel_search };
					{
						button_search_find_ascending = new QPushButton{ "Find all (ascending)", panel_search_submit };
						connect(button_search_find_ascending, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_find_ascending);

						button_search_find_descending = new QPushButton{ "Find prefix match", panel_search_submit };
						connect(button_search_find_descending, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_find_descending);

						QLabel* const label_find_limit = new QLabel{ "Limit:", panel_search_submit };
						label_find_limit->setSizePolicy(QSizePolicy{ QSizePolicy::Fixed, QSizePolicy::Fixed });

						edit_search_find_limit = new QLineEdit{ panel_search_submit };
						edit_search_find_limit->setText("1200");
						edit_search_find_limit->setFixedWidth(60);

						QHBoxLayout* const layout = new QHBoxLayout{ panel_search_submit };
						layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
						layout->addWidget(button_search_find_ascending);
						layout->addWidget(button_search_find_descending);
						layout->addWidget(label_find_limit);
						layout->addWidget(edit_search_find_limit);
					}

					tree_view_main = new QTreeView{ panel_search };
					tree_view_main->setSelectionMode(QAbstractItemView::ExtendedSelection);
					tree_view_main->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
					connect(tree_view_main, &QTreeView::doubleClicked, this, &OrderedDatastorePanel::handle_datastore_entry_double_clicked);

					QWidget* const panel_read = new QWidget{ panel_search };
					{
						button_entry_view = new QPushButton{ "View entry...", panel_read };
						connect(button_entry_view, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_view_entry);

						QHBoxLayout* const layout = new QHBoxLayout{ panel_read };
						layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
						layout->addWidget(button_entry_view);
					}

					QFrame* const horizontal_bar = new QFrame{ panel_search };
					horizontal_bar->setFrameShape(QFrame::HLine);
					horizontal_bar->setFrameShadow(QFrame::Sunken);

					QWidget* const panel_edit = new QWidget{ panel_search };
					{
						button_entry_increment = new QPushButton{ "Increment entry...", panel_edit };
						connect(button_entry_increment, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_entry_increment);

						button_entry_edit = new QPushButton{ "Edit entry...", panel_edit };
						connect(button_entry_edit, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_entry_edit);

						button_entry_delete = new QPushButton{ "Delete entry", panel_edit };
						connect(button_entry_delete, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_entry_delete);

						QHBoxLayout* const layout = new QHBoxLayout{ panel_edit };
						layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
						layout->addWidget(button_entry_increment);
						layout->addWidget(button_entry_edit);
						layout->addWidget(button_entry_delete);
					}

					QVBoxLayout* const layout_search = new QVBoxLayout{ panel_search };
					layout_search->addWidget(panel_search_params);
					layout_search->addWidget(panel_search_submit);
					layout_search->addWidget(tree_view_main);
					layout_search->addWidget(panel_read);
					layout_search->addWidget(horizontal_bar);
					layout_search->addWidget(panel_edit);
				}

				QWidget* const panel_add = new QWidget{ tab_widget_main };
				{
					edit_add_entry_datastore_name = new QLineEdit{ panel_add };
					connect(edit_add_entry_datastore_name, &QLineEdit::textChanged, this, &OrderedDatastorePanel::handle_add_entry_text_changed);

					edit_add_entry_scope = new QLineEdit{ panel_add };
					edit_add_entry_scope->setPlaceholderText("global");
					connect(edit_add_entry_scope, &QLineEdit::textChanged, this, &OrderedDatastorePanel::handle_add_entry_text_changed);

					edit_add_entry_key_name = new QLineEdit{ panel_add };
					connect(edit_add_entry_key_name, &QLineEdit::textChanged, this, &OrderedDatastorePanel::handle_add_entry_text_changed);

					edit_add_entry_value = new QLineEdit{ panel_add };
					connect(edit_add_entry_value, &QLineEdit::textChanged, this, &OrderedDatastorePanel::handle_add_entry_text_changed);

					button_add_entry_submit = new QPushButton{ "Submit", panel_add };
					button_add_entry_submit->setSizePolicy(QSizePolicy{ QSizePolicy::Expanding, QSizePolicy::Preferred });
					connect(button_add_entry_submit, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_submit_new_entry);

					QFormLayout* const layout = new QFormLayout{ panel_add };
					layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
					layout->addRow("Datastore", edit_add_entry_datastore_name);
					layout->addRow("Scope", edit_add_entry_scope);
					layout->addRow("Key", edit_add_entry_key_name);
					layout->addRow("Value", edit_add_entry_value);
					layout->addRow("", button_add_entry_submit);
				}

				tab_widget_main->addTab(panel_search, "Search");
				tab_widget_main->addTab(panel_add, "Add Entry");
			}

			QVBoxLayout* const layout_main = new QVBoxLayout{ panel_main };
			layout_main->setContentsMargins(QMargins{ 0, 0, 0, 0 });
			layout_main->addWidget(tab_widget_main);
		}

		splitter->addWidget(panel_index);
		splitter->addWidget(panel_main);
		splitter->setSizes({ OCT_LIST_WIDGET_LIST_WIDTH, OCT_LIST_WIDGET_MAIN_WIDTH });
	}

	QHBoxLayout* const layout = new QHBoxLayout{ this };
	layout->addWidget(splitter);

	conn_universe_ordered_datastores_changed = connect(universe.get(), &UniverseProfile::recent_ordered_datastore_list_changed, this, &OrderedDatastorePanel::handle_recent_datastores_changed);

	set_table_model(nullptr);
	handle_recent_datastores_changed();
	gui_refresh();
}

void OrderedDatastorePanel::gui_refresh()
{
	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		setEnabled(false);
		return;
	}

	setEnabled(true);

	const bool find_enabled = edit_search_datastore_name->text().size() > 0;
	button_search_find_ascending->setEnabled(find_enabled);
	button_search_find_descending->setEnabled(find_enabled);

	bool single_selected = false;
	if (const QItemSelectionModel* const select_model = tree_view_main->selectionModel())
	{
		single_selected = select_model->selectedRows().count() == 1;
	}
	button_entry_view->setEnabled(single_selected);
	button_entry_increment->setEnabled(single_selected);
	button_entry_edit->setEnabled(single_selected);
	button_entry_delete->setEnabled(single_selected);

	const bool filter_active = UserProfile::get().get_show_datastore_name_filter();
	edit_datastore_index_filter->setVisible(filter_active);

	const bool add_submit_enabled =
		edit_add_entry_datastore_name->text().size() > 0 &&
		edit_add_entry_key_name->text().size() > 0 &&
		edit_add_entry_value->text().size() > 0;
	button_add_entry_submit->setEnabled(add_submit_enabled);
}

void OrderedDatastorePanel::set_table_model(OrderedDatastoreEntryQTableModel* entry_model)
{
	if (entry_model)
	{
		tree_view_main->setModel(entry_model);
	}
	else
	{
		tree_view_main->setModel(new OrderedDatastoreEntryQTableModel{ tree_view_main, std::vector<OrderedDatastoreEntryFull>{} });
	}
	tree_view_main->setColumnWidth(0, 140);
	tree_view_main->setColumnWidth(1, 140);
	connect(tree_view_main->selectionModel(), &QItemSelectionModel::selectionChanged, this, &OrderedDatastorePanel::handle_selected_datastore_entry_changed);
	handle_selected_datastore_entry_changed();
}

QModelIndex OrderedDatastorePanel::get_selected_single_index() const
{
	const QItemSelectionModel* const select_model = tree_view_main->selectionModel();
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

void OrderedDatastorePanel::view_entry(const QModelIndex& index, ViewOrderedDatastoreEntryWindow::EditMode edit_mode)
{
	if (index.isValid() == false)
	{
		return;
	}

	OrderedDatastoreEntryQTableModel* const model = dynamic_cast<OrderedDatastoreEntryQTableModel*>(tree_view_main->model());
	if (model == nullptr)
	{
		return;
	}

	std::optional<OrderedDatastoreEntryFull> opt_entry = model->get_entry(index.row());
	if (opt_entry.has_value() == false)
	{
		return;
	}

	const auto req = std::make_shared<OrderedDatastoreEntryGetDetailsRequest>(
		api_key,
		opt_entry->get_universe_id(),
		opt_entry->get_datastore_name(),
		opt_entry->get_scope(),
		opt_entry->get_key_name()
	);

	OperationInProgressDialog diag{ this, req };
	diag.exec();

	const std::optional<OrderedDatastoreEntryFull> opt_details = req->get_details();
	if (opt_details)
	{
		ViewOrderedDatastoreEntryWindow* const view_entry_window = new ViewOrderedDatastoreEntryWindow{ this, api_key, *opt_details, edit_mode};
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

void OrderedDatastorePanel::handle_datastore_entry_double_clicked(const QModelIndex& index)
{
	view_entry(index, ViewOrderedDatastoreEntryWindow::EditMode::View);
}

void OrderedDatastorePanel::handle_search_text_changed()
{
	gui_refresh();
}

void OrderedDatastorePanel::handle_selected_datastore_changed()
{
	const QList<QListWidgetItem*> selected = list_datastore_index->selectedItems();
	if (selected.size() == 1)
	{
		edit_search_datastore_name->setText(selected.first()->text());
		edit_add_entry_datastore_name->setText(selected.first()->text());
	}
	gui_refresh();
}

void OrderedDatastorePanel::handle_selected_datastore_entry_changed()
{
	gui_refresh();
}

void OrderedDatastorePanel::handle_show_datastore_filter_changed()
{
	edit_datastore_index_filter->setText("");
	gui_refresh();
}

void OrderedDatastorePanel::handle_add_entry_text_changed()
{
	gui_refresh();
}

void OrderedDatastorePanel::handle_recent_datastores_changed()
{
	list_datastore_index->clear();
	const std::shared_ptr<const UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		return;
	}

	for (const QString& this_topic : universe->get_recent_ordered_datastore_set())
	{
		list_datastore_index->addItem(this_topic);
	}
}

void OrderedDatastorePanel::handle_save_recent_datastores_toggled()
{
	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		return;
	}

	universe->set_save_recent_ordered_datastores(check_save_recent_datastores->isChecked());
}

void OrderedDatastorePanel::refresh_datastore_list()
{
	for (int i = 0; i < list_datastore_index->count(); i++)
	{
		QListWidgetItem* const this_item = list_datastore_index->item(i);
		const bool matches_filter = this_item->text().contains(edit_datastore_index_filter->text());
		this_item->setHidden(!matches_filter);
	}
}

void OrderedDatastorePanel::pressed_find(const bool ascending)
{
	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		OCTASSERT(false);
		return;
	}

	if (edit_search_datastore_name->text().trimmed().size() == 0)
	{
		return;
	}

	const long long universe_id = universe->get_universe_id();
	const QString datastore_name = edit_search_datastore_name->text().trimmed();
	QString scope = edit_search_datastore_scope->text().trimmed();

	if (scope.size() == 0)
	{
		scope = "global";
	}

	const size_t result_limit = edit_search_find_limit->text().trimmed().toULongLong();

	const auto req = std::make_shared<OrderedDatastoreEntryGetListRequest>(api_key, universe_id, datastore_name, scope, ascending);
	if (result_limit > 0)
	{
		req->set_result_limit(result_limit);
	}
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	if (check_save_recent_datastores->isChecked() && req->get_entries().size() > 0)
	{
		universe->add_recent_ordered_datastore(datastore_name);
	}

	OrderedDatastoreEntryQTableModel* const qt_model = new OrderedDatastoreEntryQTableModel{ tree_view_main, req->get_entries() };
	set_table_model(qt_model);
}

void OrderedDatastorePanel::pressed_find_ascending()
{
	pressed_find(true);
}

void OrderedDatastorePanel::pressed_find_descending()
{
	pressed_find(false);
}

void OrderedDatastorePanel::pressed_remove_datastore()
{
	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		OCTASSERT(false);
		return;
	}

	QList<QListWidgetItem*> selected = list_datastore_index->selectedItems();
	if (selected.size() == 1)
	{
		universe->remove_recent_ordered_datastore(selected.front()->text());
	}
}

void OrderedDatastorePanel::pressed_submit_new_entry()
{
	const QString data_raw = edit_add_entry_value->text();

	const bool value_valid = DataValidator::is_number(data_raw);
	if (value_valid == false)
	{
		alert_error_blocking("Validation Error", "New value is not a valid integer.");
		return;
	}

	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		OCTASSERT(false);
		return;
	}

	ConfirmChangeDialog* const confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::OrderedDatastoreCreate };
	const bool confirmed = static_cast<bool>(confirm_dialog->exec());
	if (confirmed == false)
	{
		return;
	}

	const long long universe_id = universe->get_universe_id();
	const QString datastore_name = edit_add_entry_datastore_name->text();
	const QString scope = edit_add_entry_scope->text().size() > 0 ? edit_add_entry_scope->text() : "global";
	const QString key_name = edit_add_entry_key_name->text();
	const long long value = edit_add_entry_value->text().toLongLong();

	const auto post_req = std::make_shared<OrderedDatastoreEntryPostCreateRequest>(api_key, universe_id, datastore_name, scope, key_name, value);
	OperationInProgressDialog diag{ this, post_req };
	diag.exec();

	if (post_req->req_success())
	{
		edit_add_entry_value->clear();
	}
}

void OrderedDatastorePanel::pressed_view_entry()
{
	view_entry(get_selected_single_index(), ViewOrderedDatastoreEntryWindow::EditMode::View);
}

void OrderedDatastorePanel::pressed_entry_delete()
{
	const QModelIndex index = get_selected_single_index();
	if (index.isValid() == false)
	{
		return;
	}

	ConfirmChangeDialog* const confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::OrderedDatastoreDelete };
	const bool confirmed = static_cast<bool>(confirm_dialog->exec());
	if (confirmed == false)
	{
		return;
	}

	OrderedDatastoreEntryQTableModel* const model = dynamic_cast<OrderedDatastoreEntryQTableModel*>(tree_view_main->model());
	OCTASSERT(model != nullptr);
	if (model == nullptr)
	{
		return;
	}

	std::optional<OrderedDatastoreEntryFull> opt_entry = model->get_entry(index.row());
	if (opt_entry.has_value() == false)
	{
		return;
	}

	const auto req = std::make_shared<OrderedDatastoreEntryDeleteRequest>(
		api_key,
		opt_entry->get_universe_id(),
		opt_entry->get_datastore_name(),
		opt_entry->get_scope(),
		opt_entry->get_key_name()
	);

	OperationInProgressDialog diag{ this, req };
	diag.exec();
}

void OrderedDatastorePanel::pressed_entry_edit()
{
	view_entry(get_selected_single_index(), ViewOrderedDatastoreEntryWindow::EditMode::Edit);
}

void OrderedDatastorePanel::pressed_entry_increment()
{
	view_entry(get_selected_single_index(), ViewOrderedDatastoreEntryWindow::EditMode::Increment);
}
