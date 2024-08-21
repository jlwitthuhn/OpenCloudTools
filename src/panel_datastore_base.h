#pragma once

#include <memory>

#include <QObject>
#include <QString>
#include <QWidget>

class QGroupBox;
class QLineEdit;
class QListWidget;
class QModelIndex;
class QPoint;
class QTabWidget;
class QTreeView;

class UniverseProfile;

class BaseDatastorePanel : public QWidget
{
	Q_OBJECT
public:
	BaseDatastorePanel(QWidget* parent, const QString& api_key);

	virtual void change_universe(const std::shared_ptr<UniverseProfile>& universe);

protected:
	void show_blocking_error(const QString& title, const QString& message);

	QModelIndex get_selected_single_index() const;

	virtual void handle_datastore_entry_double_clicked(const QModelIndex& /*index*/ ) {}
	virtual void handle_selected_datastore_changed();
	virtual void handle_selected_datastore_entry_changed() {}
	virtual void handle_search_text_changed() {}
	virtual void handle_show_datastore_filter_changed();

	virtual void pressed_right_click_datastore_list(const QPoint& /*pos*/) {}
	virtual void pressed_right_click_entry_list(const QPoint& /*pos*/) {}

	virtual void clear_model() = 0;
	virtual void refresh_datastore_list() = 0;

	QString api_key;

	// Left panel
	QWidget* panel_left = nullptr;
	QGroupBox* select_datastore_group = nullptr;
	QListWidget* select_datastore_list = nullptr;
	QLineEdit* select_datastore_name_filter_edit = nullptr;

	// Main panel
	QTabWidget* main_tab_widget = nullptr;

	// Search panel
	QWidget* search_panel = nullptr;

	QWidget* search_params_widget = nullptr;
	QLineEdit* search_datastore_name_edit = nullptr;
	QLineEdit* search_datastore_scope_edit = nullptr;

	QWidget* search_submit_widget = nullptr;

	QLineEdit* find_limit_edit = nullptr;

	QTreeView* datastore_entry_tree = nullptr;
};
