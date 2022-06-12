#pragma once

#include <QObject>
#include <QString>
#include <QWidget>

class QPushButton;

class BulkDataPanel : public QWidget
{
	Q_OBJECT
public:
	BulkDataPanel(QWidget* parent, const QString& api_key);

	void selected_universe_changed();

private:
	void pressed_download();

	QString api_key;

	QPushButton* datastore_download_button = nullptr;
};
