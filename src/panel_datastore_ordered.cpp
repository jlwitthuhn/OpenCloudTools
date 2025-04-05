#include "panel_datastore_ordered.h"

#include <cstddef>

#include <memory>
#include <optional>
#include <set>
#include <vector>

#include <Qt>
#include <QtGlobal>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QCheckBox>
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

OrderedDatastorePanel::OrderedDatastorePanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe) :
	QWidget{ parent },
	api_key { api_key },
	attached_universe{ universe }
{
	OCTASSERT(universe);
	connect(&(UserProfile::get()), &UserProfile::show_datastore_filter_changed, this, &OrderedDatastorePanel::handle_show_datastore_filter_changed);

	QSplitter* const splitter = new QSplitter{ this };
	{
		QGroupBox* const group_index = new QGroupBox{ "Data store list", splitter };
		{
			list_datastore_index = new QListWidget{ group_index };
			list_datastore_index->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
			connect(list_datastore_index, &QListWidget::itemSelectionChanged, this, &OrderedDatastorePanel::handle_selected_datastore_changed);

			edit_datastore_index_filter = new QLineEdit{ group_index };
			edit_datastore_index_filter->setPlaceholderText("filter");
			edit_datastore_index_filter->setToolTip("Only data store names matching this text box will be displayed.");
			connect(edit_datastore_index_filter, &QLineEdit::textChanged, this, &OrderedDatastorePanel::refresh_datastore_list);

			check_save_recent_datastores = new QCheckBox{ "Add used data stores", group_index };
			check_save_recent_datastores->setChecked(universe->get_save_recent_ordered_datastores());
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

		QGroupBox* const group_main = new QGroupBox{ "Search", splitter };
		{
			QWidget* const panel_search_params = new QWidget{ group_main };
			{
				QLabel* const label_search_datastore_name = new QLabel{ "Data store:", panel_search_params };

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

			QWidget* const panel_search_submit = new QWidget{ group_main };
			{
				button_search_find_ascending = new QPushButton{ "Find all (ascending)", panel_search_submit };
				connect(button_search_find_ascending, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_find_ascending);

				button_search_find_descending = new QPushButton{ "Find all (descending)", panel_search_submit };
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

			tree_view_main = new QTreeView{ group_main };
			tree_view_main->setSelectionMode(QAbstractItemView::ExtendedSelection);
			tree_view_main->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
			connect(tree_view_main, &QTreeView::doubleClicked, this, &OrderedDatastorePanel::handle_datastore_entry_double_clicked);

			QWidget* const panel_read = new QWidget{ group_main };
			{
				button_entry_view = new QPushButton{ "View entry...", panel_read };
				connect(button_entry_view, &QPushButton::clicked, this, &OrderedDatastorePanel::pressed_view_entry);

				QHBoxLayout* const layout = new QHBoxLayout{ panel_read };
				layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
				layout->addWidget(button_entry_view);
			}

			QFrame* const horizontal_bar = new QFrame{ group_main };
			horizontal_bar->setFrameShape(QFrame::HLine);
			horizontal_bar->setFrameShadow(QFrame::Sunken);

			QWidget* const panel_edit = new QWidget{ group_main };
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

			QVBoxLayout* const layout_search = new QVBoxLayout{ group_main };
			layout_search->addWidget(panel_search_params);
			layout_search->addWidget(panel_search_submit);
			layout_search->addWidget(tree_view_main);
			layout_search->addWidget(panel_read);
			layout_search->addWidget(horizontal_bar);
			layout_search->addWidget(panel_edit);
		}

		splitter->addWidget(group_index);
		splitter->addWidget(group_main);
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

	const auto req = std::make_shared<OrderedDatastoreEntryGetDetailsV2Request>(
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

	const auto req = std::make_shared<OrderedDatastoreEntryGetListV2Request>(api_key, universe_id, datastore_name, scope, ascending);
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

	const auto req = std::make_shared<OrderedDatastoreEntryDeleteV2Request>(
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
