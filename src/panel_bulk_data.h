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

	QString api_key;

	QPushButton* datastore_download_button = nullptr;

	QCheckBox* danger_buttons_check = nullptr;
	QPushButton* datastore_delete_button = nullptr;
};
