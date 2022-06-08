#pragma once

#include <optional>

#include <QObject>
#include <QString>
#include <QMainWindow>

class QAction;
class QComboBox;
class QPushButton;
class QTabWidget;
class QWidget;

class BulkDataPanel;
class ExploreDatastorePanel;
class HttpLogPanel;

class MyMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MyMainWindow(QWidget* parent, QString title, QString api_key);

private:
	void selected_universe_changed();
	void universe_list_changed();

	void pressed_add_universe();
	void pressed_remove_universe();
	void pressed_change_key();
	void pressed_toggle_autoclose();

	void handle_autoclose_changed();
	void handle_tab_changed(int index);

	QString api_key;
	std::optional<long long> selected_universe_id;

	QAction* action_toggle_autoclose = nullptr;

	QComboBox* select_universe_combo = nullptr;

	QPushButton* del_universe_button = nullptr;

	QTabWidget* panel_tabs = nullptr;

	ExploreDatastorePanel* explore_datastore_panel = nullptr;
	BulkDataPanel* bulk_data_panel = nullptr;
	HttpLogPanel* http_log_panel = nullptr;
};
