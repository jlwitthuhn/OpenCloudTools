#pragma once

#include <memory>
#include <optional>

#include <QMainWindow>
#include <QMetaObject>
#include <QObject>

#include "profile.h"

class QLineEdit;
class QPushButton;
class QTreeWidget;

class MyNewMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MyNewMainWindow();

private:
	void gui_refresh();

	std::optional<UniverseProfile::Id> get_selected_universe_id();

	void handle_active_api_key_changed();
	void handle_active_api_key_details_changed();
	void handle_universe_details_changed(UniverseProfile::Id changed_id);
	void handle_universe_list_changed();

	void pressed_change_key();

	void pressed_add_universe();
	void pressed_edit_universe();
	void pressed_delete_universe();

	void rebuild_universe_tree();

	std::weak_ptr<ApiKeyProfile> attached_profile;
	QMetaObject::Connection conn_attached_profile_details_changed;
	QMetaObject::Connection conn_attached_profile_universe_details_changed;
	QMetaObject::Connection conn_attached_profile_universe_list_changed;

	QLineEdit* edit_api_key_name = nullptr;

	QTreeWidget* tree_universe = nullptr;

	QPushButton* button_edit_universe = nullptr;
	QPushButton* button_delete_universe = nullptr;
};
