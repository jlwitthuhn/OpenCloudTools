#pragma once

#include <memory>

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QWidget>

class QCheckBox;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTreeView;

class MemoryStoreSortedMapQTableModel;

class UniverseProfile;

class MemoryStoreSortedMapPanel : public QWidget
{
	Q_OBJECT
public:
	MemoryStoreSortedMapPanel(QWidget* parent, const QString& api_key);

	void change_universe(const std::shared_ptr<UniverseProfile>& universe);

private:
	void gui_refresh();

	void set_table_model(MemoryStoreSortedMapQTableModel* table_model);

	void handle_recent_maps_changed();
	void handle_save_recent_maps_toggled();
	void handle_search_name_changed();
	void handle_selected_map_changed();

	void pressed_list_all(bool ascending);
	void pressed_list_all_asc() { return pressed_list_all(true); };
	void pressed_list_all_desc() { return pressed_list_all(false); }
	void pressed_remove_recent_map();

	QString api_key;
	std::weak_ptr<UniverseProfile> attached_universe;
	QMetaObject::Connection conn_universe_mem_sorted_map_list_changed;

	// Index panel
	QListWidget* list_maps = nullptr;
	QCheckBox* check_save_recent_maps = nullptr;
	QPushButton* button_remove_recent_map = nullptr;

	// Search panel
	QLineEdit* edit_map_name = nullptr;
	QPushButton* button_list_all_asc = nullptr;
	QPushButton* button_list_all_desc = nullptr;
	QLineEdit* edit_list_limit = nullptr;
	QTreeView* tree_view = nullptr;
};
