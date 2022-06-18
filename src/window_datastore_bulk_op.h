#pragma once

#include <vector>

#include <QObject>
#include <QString>
#include <QWidget>

class QCheckBox;
class QLineEdit;
class QListWidget;
class QPushButton;

class DatastoreBulkOperationWindow : public QWidget
{
	Q_OBJECT
protected:
	DatastoreBulkOperationWindow(QWidget* parent, const QString& api_key, long long universe_id, const std::vector<QString>& datastore_names);

	virtual void pressed_submit() = 0;

	std::vector<QString> get_selected_datastores() const;

	void handle_show_hidden_toggled();

	void pressed_select_all();
	void pressed_select_none();
	void pressed_toggle_filter();

	QString api_key;
	long long universe_id = 0;

	QListWidget* datastore_list = nullptr;
	QCheckBox* datastore_list_show_hidden_check = nullptr;

	QCheckBox* filter_enabled_check = nullptr;

	QLineEdit* filter_scope_edit = nullptr;
	QLineEdit* filter_key_prefix_edit = nullptr;

	QPushButton* submit_button = nullptr;
};

class DatastoreBulkDownloadWindow : public DatastoreBulkOperationWindow
{
	Q_OBJECT
public:
	DatastoreBulkDownloadWindow(QWidget* parent, const QString& api_key, long long universe_id, const std::vector<QString>& datastore_names);

private:
	virtual void pressed_submit() override;
};
