#include "panel_datastore_standard.h"

#include <cstddef>

#include <memory>
#include <optional>
#include <set>

#include <Qt>
#include <QtGlobal>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QAction>
#include <QCheckBox>
#include <QClipboard>
#include <QFrame>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMargins>
#include <QMenu>
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
#include "util_enum.h"
#include "window_datastore_entry_versions_view.h"
#include "window_datastore_entry_view.h"

StandardDatastorePanel::StandardDatastorePanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe) :
	QWidget{ parent },
	api_key{ api_key },
	attached_universe{ universe }
{
	connect(&(UserProfile::get()), &UserProfile::show_datastore_filter_changed, this, &StandardDatastorePanel::handle_show_datastore_filter_changed);

	QSplitter* splitter = new QSplitter{ this };
	{
		QGroupBox* const group_index = new QGroupBox{ "Data store list", splitter };
		{
			list_datastore_index = new QListWidget{ group_index };
			list_datastore_index->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
			connect(list_datastore_index, &QListWidget::customContextMenuRequested, this, &StandardDatastorePanel::pressed_right_click_datastore_list);
			connect(list_datastore_index, &QListWidget::itemSelectionChanged, this, &StandardDatastorePanel::handle_selected_datastore_changed);

			edit_datastore_index_filter = new QLineEdit{ group_index };
			edit_datastore_index_filter->setPlaceholderText("filter");
			edit_datastore_index_filter->setToolTip("Only data store names matching this text box will be displayed.");
			connect(edit_datastore_index_filter, &QLineEdit::textChanged, this, &StandardDatastorePanel::refresh_datastore_list);

			check_datastore_index_show_hidden = new QCheckBox{ "Show hidden", group_index };
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
			connect(check_datastore_index_show_hidden, &QCheckBox::checkStateChanged, this, &StandardDatastorePanel::handle_show_hidden_datastores_toggled);
#else
			connect(check_datastore_index_show_hidden, &QCheckBox::stateChanged, this, &StandardDatastorePanel::handle_show_hidden_datastores_toggled);
#endif

			button_datastore_index_fetch = new QPushButton{ "Fetch data stores", group_index };
			connect(button_datastore_index_fetch, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_fetch_datastores);

			QVBoxLayout* const layout_group = new QVBoxLayout{ group_index };
			layout_group->addWidget(list_datastore_index);
			layout_group->addWidget(edit_datastore_index_filter);
			layout_group->addWidget(check_datastore_index_show_hidden);
			layout_group->addWidget(button_datastore_index_fetch);
		}

		QGroupBox* const group_search = new QGroupBox{ "Search", splitter };
		{
			QWidget* const panel_search = new QWidget{ group_search };
			{
				QWidget* const panel_search_params = new QWidget{ panel_search };
				{
					QLabel* const label_search_datastore_name = new QLabel{ "Data store:", panel_search_params };

					edit_search_datastore_name = new QLineEdit{ panel_search_params };
					connect(edit_search_datastore_name, &QLineEdit::textChanged, this, &StandardDatastorePanel::handle_search_text_changed);

					QLabel* const label_search_datastore_scope = new QLabel{ "Scope:", panel_search_params };

					edit_search_datastore_scope = new QLineEdit{ panel_search_params };
					edit_search_datastore_scope->setPlaceholderText("global");
					connect(edit_search_datastore_scope, &QLineEdit::textChanged, this, &StandardDatastorePanel::handle_search_text_changed);

					QLabel* const label_search_datastore_key_name = new QLabel{ "Key prefix:", panel_search_params };

					edit_search_datastore_key_prefix = new QLineEdit{ panel_search_params };
					connect(edit_search_datastore_key_prefix, &QLineEdit::textChanged, this, &StandardDatastorePanel::handle_search_text_changed);

					QHBoxLayout* const layout = new QHBoxLayout{ panel_search_params };
					layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
					layout->addWidget(label_search_datastore_name);
					layout->addWidget(edit_search_datastore_name);
					layout->addWidget(label_search_datastore_scope);
					layout->addWidget(edit_search_datastore_scope);
					layout->addWidget(label_search_datastore_key_name);
					layout->addWidget(edit_search_datastore_key_prefix);
				}

				QWidget* const panel_search_submit = new QWidget{ panel_search };
				{
					button_search_find_all = new QPushButton{ "Find all", panel_search_submit };
					connect(button_search_find_all, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_find_all);

					button_search_find_prefix = new QPushButton{ "Find prefix match", panel_search_submit };
					connect(button_search_find_prefix, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_find_prefix);

					QLabel* const label_find_limit = new QLabel{ "Limit:", panel_search_submit };
					label_find_limit->setSizePolicy(QSizePolicy{ QSizePolicy::Fixed, QSizePolicy::Fixed });

					edit_search_find_limit = new QLineEdit{ panel_search_submit };
					edit_search_find_limit->setText("1200");
					edit_search_find_limit->setFixedWidth(60);

					QHBoxLayout* const layout = new QHBoxLayout{ panel_search_submit };
					layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
					layout->addWidget(button_search_find_all);
					layout->addWidget(button_search_find_prefix);
					layout->addWidget(label_find_limit);
					layout->addWidget(edit_search_find_limit);
				}

				tree_view_main = new QTreeView{ panel_search };
				tree_view_main->setSelectionMode(QAbstractItemView::ExtendedSelection);
				tree_view_main->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
				connect(tree_view_main, &QListWidget::customContextMenuRequested, this, &StandardDatastorePanel::pressed_right_click_entry_list);
				connect(tree_view_main, &QTreeView::doubleClicked, this, &StandardDatastorePanel::handle_datastore_entry_double_clicked);

				QWidget* const panel_read = new QWidget{ panel_search };
				{
					button_entry_view = new QPushButton{ "View entry...", panel_read };
					connect(button_entry_view, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_view_entry);

					button_entry_view_version = new QPushButton{ "View versions...", panel_read };
					connect(button_entry_view_version, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_view_versions);

					QHBoxLayout* const layout = new QHBoxLayout{ panel_read };
					layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
					layout->addWidget(button_entry_view);
					layout->addWidget(button_entry_view_version);
				}

				QFrame* const horizontal_bar = new QFrame{ panel_search };
				horizontal_bar->setFrameShape(QFrame::HLine);
				horizontal_bar->setFrameShadow(QFrame::Sunken);

				QWidget* const panel_edit = new QWidget{ panel_search };
				{
					button_entry_edit = new QPushButton{ "Edit entry...", panel_edit };
					connect(button_entry_edit, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_edit_entry);

					button_entry_delete = new QPushButton{ "Delete entry", panel_edit };
					connect(button_entry_delete, &QPushButton::clicked, this, &StandardDatastorePanel::pressed_delete_entry);

					QHBoxLayout* const layout = new QHBoxLayout{ panel_edit };
					layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });
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

			QVBoxLayout* const layout_search = new QVBoxLayout{ group_search };
			layout_search->setContentsMargins(QMargins{ 0, 0, 0, 0 });
			layout_search->addWidget(panel_search);
		}

		splitter->addWidget(group_index);
		splitter->addWidget(group_search);
		splitter->setSizes({ OCT_LIST_WIDGET_LIST_WIDTH, OCT_LIST_WIDGET_MAIN_WIDTH });
	}

	QHBoxLayout* const layout = new QHBoxLayout{ this };
	layout->addWidget(splitter);

	set_table_model(nullptr);

	conn_universe_hidden_datastores_changed = connect(universe.get(), &UniverseProfile::hidden_datastore_list_changed, this, &StandardDatastorePanel::refresh_datastore_list);
	check_datastore_index_show_hidden->setChecked(universe->get_show_hidden_standard_datastores());

	gui_refresh();
}

void StandardDatastorePanel::gui_refresh()
{
	const std::shared_ptr<UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		setEnabled(false);
		return;
	}

	setEnabled(true);

	{
		const bool find_all_enabled = edit_search_datastore_name->text().size() > 0;
		const bool find_prefix_enabled = find_all_enabled && edit_search_datastore_key_prefix->text().size() > 0;
		button_search_find_all->setEnabled(find_all_enabled);
		button_search_find_prefix->setEnabled(find_prefix_enabled);
	}

	{
		qsizetype count = 0;
		bool single_selected = false;
		bool multi_selected = false;
		if (const QItemSelectionModel* select_model = tree_view_main->selectionModel())
		{
			count = select_model->selectedRows().count();
			single_selected = count == 1;
			multi_selected = count > 1;
		}
		button_entry_view->setEnabled(single_selected);
		button_entry_view_version->setEnabled(single_selected);
		button_entry_edit->setEnabled(single_selected);
		button_entry_delete->setEnabled(single_selected || multi_selected);

		if (multi_selected)
		{
			button_entry_delete->setText(QString{ "Delete %1 entries" }.arg(count));
		}
		else
		{
			button_entry_delete->setText("Delete entry");
		}
	}

	{
		const bool filter_visible = UserProfile::get().get_show_datastore_name_filter();
		if (!filter_visible)
		{
			edit_datastore_index_filter->setText("");
		}
		edit_datastore_index_filter->setVisible(filter_visible);
	}
}

void StandardDatastorePanel::set_table_model(StandardDatastoreEntryQTableModel* const entry_model)
{
	if (entry_model)
	{
		tree_view_main->setModel(entry_model);
	}
	else
	{
		tree_view_main->setModel(new StandardDatastoreEntryQTableModel{ tree_view_main, std::vector<StandardDatastoreEntryName>{} });
	}
	tree_view_main->setColumnWidth(0, 140);
	connect(tree_view_main->selectionModel(), &QItemSelectionModel::selectionChanged, this, &StandardDatastorePanel::handle_selected_datastore_entry_changed);
	handle_selected_datastore_entry_changed();
}

std::vector<StandardDatastoreEntryName> StandardDatastorePanel::get_selected_entries() const
{
	const StandardDatastoreEntryQTableModel* const entry_model = dynamic_cast<StandardDatastoreEntryQTableModel*>(tree_view_main->model());
	if (!entry_model)
	{
		return std::vector<StandardDatastoreEntryName>{};
	}

	const QItemSelectionModel* const select_model = tree_view_main->selectionModel();
	if (select_model == nullptr)
	{
		return std::vector<StandardDatastoreEntryName>{};
	}

	if (select_model->selectedRows().count() == 0)
	{
		return std::vector<StandardDatastoreEntryName>{};
	}

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

QModelIndex StandardDatastorePanel::get_selected_single_index() const
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

void StandardDatastorePanel::view_entry(const QModelIndex& index)
{
	if (index.isValid() == false)
	{
		return;
	}

	const StandardDatastoreEntryQTableModel* model = dynamic_cast<StandardDatastoreEntryQTableModel*>(tree_view_main->model());
	if (model == nullptr)
	{
		return;
	}

	const std::optional<StandardDatastoreEntryName> opt_entry = model->get_entry(index.row());
	if (!opt_entry)
	{
		return;
	}

	const auto req = std::make_shared<StandardDatastoreEntryGetDetailsRequest>(api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key());
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	const std::optional<StandardDatastoreEntryFull> opt_details = req->get_details();
	if (opt_details)
	{
		ViewDatastoreEntryWindow* const view_entry_window = new ViewDatastoreEntryWindow{ this, api_key, *opt_details };
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

void StandardDatastorePanel::view_versions(const QModelIndex& index)
{
	if (index.isValid() == false)
	{
		return;
	}

	const StandardDatastoreEntryQTableModel* const model = dynamic_cast<StandardDatastoreEntryQTableModel*>(tree_view_main->model());
	if (model == nullptr)
	{
		return;
	}

	const std::optional<StandardDatastoreEntryName> opt_entry = model->get_entry(index.row());
	if (!opt_entry)
	{
		return;
	}

	const auto req = std::make_shared<StandardDatastoreEntryGetVersionListRequest>(api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key());
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	if (req->get_versions().size() == 0)
	{
		return;
	}

	ViewDatastoreEntryVersionsWindow* const view_versions_window = new ViewDatastoreEntryVersionsWindow{ this, api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key(), req->get_versions() };
	view_versions_window->show();
}

void StandardDatastorePanel::edit_entry(const QModelIndex& index)
{
	if (index.isValid() == false)
	{
		return;
	}

	const StandardDatastoreEntryQTableModel* const model = dynamic_cast<StandardDatastoreEntryQTableModel*>(tree_view_main->model());
	if (model == nullptr)
	{
		return;
	}

	const std::optional<StandardDatastoreEntryName> opt_entry = model->get_entry(index.row());
	if (opt_entry.has_value() == false)
	{
		return;
	}

	const auto req = std::make_shared<StandardDatastoreEntryGetDetailsRequest>(api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key());
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	const std::optional<StandardDatastoreEntryFull> opt_details = req->get_details();
	if (opt_details)
	{
		ViewDatastoreEntryWindow* const edit_entry_window = new ViewDatastoreEntryWindow{ this, api_key, *opt_details, ViewEditMode::Edit };
		edit_entry_window->show();
	}
}

void StandardDatastorePanel::delete_entry(const QModelIndex& index)
{
	if (index.isValid() == false)
	{
		return;
	}

	const StandardDatastoreEntryQTableModel* model = dynamic_cast<StandardDatastoreEntryQTableModel*>(tree_view_main->model());
	if (model == nullptr)
	{
		return;
	}

	std::optional<StandardDatastoreEntryName> opt_entry = model->get_entry(index.row());
	if (opt_entry.has_value() == false)
	{
		return;
	}

	ConfirmChangeDialog* confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::StandardDatastoreDelete };
	bool confirmed = static_cast<bool>(confirm_dialog->exec());
	if (confirmed == false)
	{
		return;
	}

	const auto req = std::make_shared<StandardDatastoreEntryDeleteRequest>(api_key, opt_entry->get_universe_id(), opt_entry->get_datastore_name(), opt_entry->get_scope(), opt_entry->get_key());
	OperationInProgressDialog diag{ this, req };
	diag.exec();
}


void StandardDatastorePanel::delete_entry_list(const std::vector<StandardDatastoreEntryName>& entry_list)
{
	OCTASSERT(entry_list.size() > 1);
	if (entry_list.size() <= 1)
	{
		return;
	}

	ConfirmChangeDialog* const confirm_dialog = new ConfirmChangeDialog{ this, ChangeType::StandardDatastoreMultiDelete };
	bool confirmed = static_cast<bool>(confirm_dialog->exec());
	if (confirmed == false)
	{
		return;
	}

	std::vector<std::shared_ptr<DataRequest>> request_list;
	for (const StandardDatastoreEntryName& this_entry : entry_list)
	{
		std::shared_ptr<StandardDatastoreEntryDeleteRequest> this_request =
			std::make_shared<StandardDatastoreEntryDeleteRequest>(api_key, this_entry.get_universe_id(), this_entry.get_datastore_name(), this_entry.get_scope(), this_entry.get_key());

		request_list.push_back(this_request);
	}

	OperationInProgressDialog diag{ this, request_list };
	diag.exec();
}

void StandardDatastorePanel::handle_datastore_entry_double_clicked(const QModelIndex& index)
{
	view_entry(index);
}

void StandardDatastorePanel::handle_search_text_changed()
{
	gui_refresh();
}

void StandardDatastorePanel::handle_selected_datastore_changed()
{
	const QList<QListWidgetItem*> selected = list_datastore_index->selectedItems();
	if (selected.size() == 1)
	{
		edit_search_datastore_name->setText(selected.first()->text());
	}
	gui_refresh();
}

void StandardDatastorePanel::handle_selected_datastore_entry_changed()
{
	gui_refresh();
}

void StandardDatastorePanel::handle_show_datastore_filter_changed()
{
	gui_refresh();
}

void StandardDatastorePanel::handle_show_hidden_datastores_toggled()
{
	gui_refresh();
	if (const std::shared_ptr<UniverseProfile> universe = attached_universe.lock())
	{
		const bool checked = check_datastore_index_show_hidden->isChecked();
		universe->set_show_hidden_standard_datastores(checked);
	}
}

void StandardDatastorePanel::pressed_right_click_datastore_list(const QPoint& pos)
{
	const QModelIndex the_index = list_datastore_index->indexAt(pos);
	if (the_index.isValid() == false)
	{
		return;
	}

	const std::shared_ptr<const UniverseProfile> this_universe = attached_universe.lock();
	if (!this_universe)
	{
		return;
	}

	const QListWidgetItem* const the_item = list_datastore_index->item(the_index.row());
		QString the_datastore_name = the_item->text();

	QMenu* const context_menu = new QMenu{ list_datastore_index };
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
			connect(hide_unhide_action, &QAction::triggered, [the_datastore_name, attached_universe = this->attached_universe]() {
				if (const std::shared_ptr<UniverseProfile> universe_profile = attached_universe.lock())
				{
					universe_profile->remove_hidden_datastore(the_datastore_name);
				}
			});
		}
		else
		{
			hide_unhide_action = new QAction{ "Hide datastore", context_menu };
			connect(hide_unhide_action, &QAction::triggered, [the_datastore_name, attached_universe = this->attached_universe]() {
				if (const std::shared_ptr<UniverseProfile> universe_profile = attached_universe.lock())
				{
					universe_profile->add_hidden_datastore(the_datastore_name);
				}
			});
		}

		context_menu->addAction(copy_name);
		context_menu->addSeparator();
		context_menu->addAction(hide_unhide_action);
	}

	context_menu->exec(list_datastore_index->mapToGlobal(pos));
	context_menu->deleteLater();
}

void StandardDatastorePanel::pressed_right_click_entry_list(const QPoint& pos)
{
	const QModelIndex the_index = tree_view_main->indexAt(pos);
	if (the_index.isValid() == false)
	{
		return;
	}

	StandardDatastoreEntryQTableModel* const the_model = dynamic_cast<StandardDatastoreEntryQTableModel*>(tree_view_main->model());
	if (the_model == nullptr)
	{
		return;
	}

	std::optional<StandardDatastoreEntryName> the_entry = the_model->get_entry(the_index.row());
	if (the_entry.has_value() == false)
	{
		return;
	}

	const QString the_key_name = the_entry->get_key();
	QMenu* const context_menu = new QMenu{ tree_view_main };
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

	context_menu->exec(tree_view_main->mapToGlobal(pos));
	context_menu->deleteLater();
}

void StandardDatastorePanel::pressed_delete_entry()
{
	const QModelIndex single_entry = get_selected_single_index();
	if (single_entry.isValid())
	{
		delete_entry(single_entry);
	}

	const std::vector<StandardDatastoreEntryName> entry_list = get_selected_entries();
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
	const std::shared_ptr<const UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		return;
	}

	const long long universe_id = universe->get_universe_id();
	OCTASSERT(universe_id != 0);

	const auto req = std::make_shared<StandardDatastoreGetListRequest>(api_key, universe_id);
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	list_datastore_index->clear();
	for (const QString& this_name : req->get_datastore_names())
	{
		QListWidgetItem* this_item = new QListWidgetItem(list_datastore_index);
		this_item->setText(this_name);
		list_datastore_index->addItem(this_item);
	}
	refresh_datastore_list();
}

void StandardDatastorePanel::pressed_find_all()
{
	const std::shared_ptr<const UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		return;
	}

	if (edit_search_datastore_name->text().trimmed().size() == 0)
	{
		return;
	}

	const long long universe_id = universe->get_universe_id();
	OCTASSERT(universe_id != 0);

	const QString datastore_name = edit_search_datastore_name->text().trimmed();
	QString scope = edit_search_datastore_scope->text().trimmed();

	if (scope.size() == 0)
	{
		scope = "global";
	}

	const size_t result_limit = edit_search_find_limit->text().trimmed().toULongLong();

	const auto req = std::make_shared<StandardDatastoreEntryGetListRequest>(api_key, universe_id, datastore_name, scope, "");
	if (result_limit > 0)
	{
		req->set_result_limit(result_limit);
	}
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	StandardDatastoreEntryQTableModel* const datastore_model = new StandardDatastoreEntryQTableModel{ tree_view_main, req->get_datastore_entries() };
	set_table_model(datastore_model);
}

void StandardDatastorePanel::pressed_find_prefix()
{
	const std::shared_ptr<const UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		return;
	}

	if (edit_search_datastore_name->text().trimmed().size() == 0)
	{
		return;
	}

	const long long universe_id = universe->get_universe_id();
	OCTASSERT(universe_id > 0);

	const QString datastore_name = edit_search_datastore_name->text().trimmed();
	QString scope = edit_search_datastore_scope->text().trimmed();
	const QString key_name = edit_search_datastore_key_prefix->text().trimmed();

	if (scope.size() == 0)
	{
		scope = "global";
	}

	const size_t result_limit = edit_search_find_limit->text().trimmed().toULongLong();

	const auto req = std::make_shared<StandardDatastoreEntryGetListRequest>(api_key, universe_id, datastore_name, scope, key_name);
	if (result_limit > 0)
	{
		req->set_result_limit(result_limit);
	}
	OperationInProgressDialog diag{ this, req };
	diag.exec();

	StandardDatastoreEntryQTableModel* datastore_model = new StandardDatastoreEntryQTableModel{ tree_view_main, req->get_datastore_entries() };
	set_table_model(datastore_model);
}

void StandardDatastorePanel::pressed_view_entry()
{
	view_entry(get_selected_single_index());
}

void StandardDatastorePanel::pressed_view_versions()
{
	view_versions(get_selected_single_index());
}

void StandardDatastorePanel::refresh_datastore_list()
{
	const std::shared_ptr<const UniverseProfile> universe = attached_universe.lock();
	if (!universe)
	{
		return;
	}

	const bool show_hidden = check_datastore_index_show_hidden->isChecked();
	for (int i = 0; i < list_datastore_index->count(); i++)
	{
		QListWidgetItem* const this_item = list_datastore_index->item(i);
		const bool is_hidden = static_cast<bool>(universe->get_hidden_datastore_set().count(this_item->text()));
		const bool matches_filter = this_item->text().contains(edit_datastore_index_filter->text());
		this_item->setHidden((!show_hidden && is_hidden) || !matches_filter);
	}
}
