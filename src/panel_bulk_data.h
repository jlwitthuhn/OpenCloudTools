#pragma once

#include <QObject>
#include <QString>
#include <QWidget>

class QCheckBox;
class QPushButton;

class BulkDataPanel : public QWidget
{
	Q_OBJECT
public:
	BulkDataPanel(QWidget* parent, const QString& api_key);

	void selected_universe_changed();

private:
	void handle_datastore_danger_toggle();

	void pressed_delete();
	void pressed_download();
	void pressed_download_resume();
	void pressed_undelete();
	void pressed_upload();

	QString api_key;

	QPushButton* datastore_download_button = nullptr;
	QPushButton* datastore_download_resume_button = nullptr;

	QCheckBox* danger_buttons_check = nullptr;
	QPushButton* datastore_delete_button = nullptr;
	QPushButton* datastore_undelete_button = nullptr;
	QPushButton* datastore_upload_button = nullptr;
};
