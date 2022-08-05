#pragma once

#include <cstddef>

#include <optional>
#include <vector>

#include <QMainWindow>
#include <QObject>
#include <QString>
#include <QWidget>

class QAction;
class QComboBox;
class QLineEdit;
class QPushButton;
class QTabWidget;

class BulkDataPanel;
class ExploreDatastorePanel;
class HttpLogPanel;
class MessagingServicePanel;
class UniversePreferencesPanel;

class MyMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MyMainWindow(QWidget* parent, QString title, QString api_key);

private:
	void selected_universe_combo_changed();

	void pressed_add_universe();
	void pressed_edit_universe();
	void pressed_remove_universe();
	void pressed_change_key();
	void pressed_toggle_autoclose();
	void pressed_toggle_less_verbose_bulk();

	void handle_autoclose_changed();
	void handle_qt_theme_changed();
	void handle_tab_changed(int index);
	void handle_universe_list_changed(std::optional<size_t> universe_index);

	QString api_key;

	std::vector<QAction*> theme_actions;
	QAction* action_toggle_autoclose = nullptr;
	QAction* action_toggle_less_verbose_bulk = nullptr;

	QComboBox* select_universe_combo = nullptr;

	QPushButton* edit_universe_button = nullptr;
	QPushButton* del_universe_button = nullptr;

	QTabWidget* panel_tabs = nullptr;

	ExploreDatastorePanel* explore_datastore_panel = nullptr;
	BulkDataPanel* bulk_data_panel = nullptr;
	HttpLogPanel* http_log_panel = nullptr;
	MessagingServicePanel* messaging_service_panel = nullptr;
	UniversePreferencesPanel* universe_preferences_panel = nullptr;
};

class MainWindowAddUniverseWindow : public QWidget
{
	Q_OBJECT
public:
	explicit MainWindowAddUniverseWindow(QWidget* parent = nullptr, bool edit_current = false);

private:
	bool input_is_valid() const;

	void text_changed();
	void pressed_add();

	bool edit_mode = false;

	QLineEdit* name_edit = nullptr;
	QLineEdit* id_edit = nullptr;

	QPushButton* add_button = nullptr;
};
