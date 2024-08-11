#pragma once

#include <optional>

#include <QMainWindow>
#include <QObject>
#include <QString>
#include <QWidget>

#include "profile.h"

class QComboBox;
class QLineEdit;
class QPushButton;
class QTabWidget;

class BulkDataPanel;
class StandardDatastorePanel;
class HttpLogPanel;
class MemoryStoreSortedMapPanel;
class MessagingServicePanel;
class OrderedDatastorePanel;
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

	void handle_tab_changed(int index);
	void handle_universe_list_changed(std::optional<UniverseProfile::Id> universe_id);

	QString api_key;

	QComboBox* select_universe_combo = nullptr;

	QPushButton* edit_universe_button = nullptr;
	QPushButton* del_universe_button = nullptr;

	QTabWidget* panel_tabs = nullptr;

	StandardDatastorePanel* standard_datastore_panel = nullptr;
	OrderedDatastorePanel* ordered_datastore_panel = nullptr;
	MemoryStoreSortedMapPanel* memory_store_panel = nullptr;
	BulkDataPanel* bulk_data_panel = nullptr;
	HttpLogPanel* http_log_panel = nullptr;
	MessagingServicePanel* messaging_service_panel = nullptr;
	UniversePreferencesPanel* universe_preferences_panel = nullptr;
};

class MainWindowAddUniverseWindow : public QWidget
{
	Q_OBJECT
public:
	explicit MainWindowAddUniverseWindow(QWidget* parent, const QString& api_key, bool edit_current);

private:
	bool id_is_valid() const;
	bool name_is_valid() const;

	void text_changed();
	void pressed_add();
	void pressed_fetch();

	bool edit_mode = false;

	QString api_key;

	QLineEdit* name_edit = nullptr;
	QLineEdit* id_edit = nullptr;

	QPushButton* fetch_name_button = nullptr;
	QPushButton* add_button = nullptr;
};
