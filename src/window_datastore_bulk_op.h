#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <QObject>
#include <QString>
#include <QWidget>

class QCheckBox;
class QDateTime;
class QLineEdit;
class QListWidget;
class QPushButton;
class QVBoxLayout;

class UniverseProfile;

class DatastoreBulkOperationWindow : public QWidget
{
	Q_OBJECT
protected:
	DatastoreBulkOperationWindow(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe, const std::vector<QString>& datastore_names);

	virtual void pressed_submit() = 0;

	std::vector<QString> get_selected_datastores() const;

	void handle_show_hidden_toggled();

	void pressed_select_all();
	void pressed_select_none();
	void pressed_toggle_filter();

	QString api_key;
	std::weak_ptr<UniverseProfile> attached_universe;

	QWidget* right_bar = nullptr;
	QVBoxLayout* right_bar_layout = nullptr;

	QListWidget* datastore_list = nullptr;
	QCheckBox* datastore_list_show_hidden_check = nullptr;

	QCheckBox* filter_enabled_check = nullptr;

	QLineEdit* filter_scope_edit = nullptr;
	QLineEdit* filter_key_prefix_edit = nullptr;

	QPushButton* submit_button = nullptr;
};

class DatastoreBulkDeleteWindow : public DatastoreBulkOperationWindow
{
	Q_OBJECT
public:
	DatastoreBulkDeleteWindow(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe, const std::vector<QString>& datastore_names);

private:
	virtual void pressed_submit() override;

	QCheckBox* confirm_count_before_delete_check = nullptr;
	QCheckBox* rewrite_before_delete_check = nullptr;
	QCheckBox* hide_after_delete_check = nullptr;
};

class DatastoreBulkDownloadWindow : public DatastoreBulkOperationWindow
{
	Q_OBJECT
public:
	DatastoreBulkDownloadWindow(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe, const std::vector<QString>& datastore_names);

private:
	virtual void pressed_submit() override;
};

class DatastoreBulkUndeleteWindow : public DatastoreBulkOperationWindow
{
	Q_OBJECT
public:
	DatastoreBulkUndeleteWindow(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe, const std::vector<QString>& datastore_names);

private:
	virtual void pressed_submit() override;

	void pressed_toggle_time_filter();

	std::optional<QDateTime> get_undelete_after_time() const;

	QCheckBox* time_filter_check = nullptr;
	QLineEdit* day_edit = nullptr;
	QLineEdit* hour_edit = nullptr;
	QLineEdit* min_edit = nullptr;
};
