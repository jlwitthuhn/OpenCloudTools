#pragma once

#include <map>
#include <memory>
#include <optional>

#include <QMainWindow>
#include <QMetaObject>
#include <QObject>
#include <QPointer>

#include "profile.h"
#include "subwindow.h"

class QLineEdit;
class QMdiArea;
class QMdiSubWindow;
class QPoint;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

class MyMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MyMainWindow();

private:
	void gui_refresh();

	std::optional<UniverseProfile::Id> get_selected_universe_id();

	void do_universe_delete(UniverseProfile::Id id);
	void do_universe_edit(UniverseProfile::Id id);

	void handle_active_api_key_changed();
	void handle_active_api_key_details_changed();
	void handle_universe_details_changed(UniverseProfile::Id changed_id);
	void handle_universe_list_changed();

	void pressed_change_key();

	void pressed_add_universe();
	void pressed_edit_universe();
	void pressed_delete_universe();

	void rebuild_universe_tree();

	void close_all_subwindows();
	void close_universe_subwindows(const UniverseProfile::Id& id);

	void show_http_log();
	void show_subwindow(const SubwindowId& id);
	void show_subwindow_from_item(QTreeWidgetItem* item);
	void show_universe_context_menu(QPoint pos);

	std::weak_ptr<ApiKeyProfile> attached_profile;
	QMetaObject::Connection conn_attached_profile_details_changed;
	QMetaObject::Connection conn_attached_profile_universe_details_changed;
	QMetaObject::Connection conn_attached_profile_universe_hidden_operations_changed;
	QMetaObject::Connection conn_attached_profile_universe_list_changed;

	QLineEdit* edit_api_key_name = nullptr;

	QTreeWidget* tree_universe = nullptr;

	QPushButton* button_edit_universe = nullptr;
	QPushButton* button_delete_universe = nullptr;

	QMdiArea* center_mdi_widget = nullptr;

	std::map<SubwindowId, QPointer<QMdiSubWindow>> subwindows;

	QPointer<QMdiSubWindow> subwindow_http_log;
};
