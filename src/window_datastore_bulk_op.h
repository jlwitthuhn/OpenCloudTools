#pragma once

#include <vector>

#include <QObject>
#include <QString>
#include <QWidget>

class QCheckBox;
class QLineEdit;
class QListWidget;

class DatastoreBulkOperationWindow : public QWidget
{
	Q_OBJECT
public:
	DatastoreBulkOperationWindow(QWidget* parent, const QString& api_key, long long universe_id, const std::vector<QString>& datastore_names);

private:
	std::vector<QString> get_selected_datastores() const;

	void pressed_save();
	void pressed_select_all();
	void pressed_select_none();
	void pressed_toggle_filter();

	QString api_key;
	long long universe_id = 0;

	QListWidget* datastore_list = nullptr;

	QCheckBox* filter_enabled_check = nullptr;

	QLineEdit* filter_scope_edit = nullptr;
	QLineEdit* filter_key_prefix_edit = nullptr;
};