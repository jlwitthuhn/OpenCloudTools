#pragma once

#include <memory>

#include <QObject>
#include <QString>
#include <QWidget>

class QCheckBox;
class QPushButton;

class UniverseProfile;

class BulkDataPanel : public QWidget
{
	Q_OBJECT
public:
	BulkDataPanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe);

private:
	void gui_refresh();

	void handle_datastore_danger_toggle();

	void pressed_delete();
	void pressed_download();
	void pressed_download_resume();
	void pressed_snapshot();
	void pressed_undelete();
	void pressed_upload();

	QString api_key;
	std::weak_ptr<UniverseProfile> attached_universe;

	QPushButton* datastore_download_button = nullptr;
	QPushButton* datastore_download_resume_button = nullptr;
	QPushButton* datastore_snapshot_button = nullptr;

	QCheckBox* danger_buttons_check = nullptr;
	QPushButton* datastore_delete_button = nullptr;
	QPushButton* datastore_undelete_button = nullptr;
	QPushButton* datastore_upload_button = nullptr;
};
