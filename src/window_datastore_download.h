#pragma once

#include <vector>

#include <QString>
#include <QWidget>

class QListWidget;

class DownloadDatastoreWindow : public QWidget
{
	Q_OBJECT
public:
	DownloadDatastoreWindow(QWidget* parent, const QString& api_key, long long universe_id, const std::vector<QString>& datastore_names);

private:
	std::vector<QString> get_selected_datastores() const;

	void pressed_save();
	void pressed_select_all();
	void pressed_select_none();

	QString api_key;
	long long universe_id = 0;
	QListWidget* datastore_list = nullptr;
};
