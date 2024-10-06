#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <QMainWindow>
#include <QMetaObject>
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

class UniverseProfile;

class MyMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MyMainWindow(QWidget* parent, QString title, QString api_key);

private:
	void selected_universe_combo_changed();

	std::shared_ptr<UniverseProfile> get_selected_universe() const;

	void pressed_add_universe();
	void pressed_edit_universe();
	void pressed_remove_universe();
	void pressed_change_key();

	void handle_tab_changed(int index);
	void handle_universe_list_changed(std::optional<UniverseProfile::Id> new_universe);

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

	std::vector<QMetaObject::Connection> universe_details_updated_conns;
};
