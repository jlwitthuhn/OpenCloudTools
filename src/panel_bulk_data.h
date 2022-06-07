#pragma once

#include <QObject>
#include <QWidget>

class QPushButton;

class BulkDataPanel : public QWidget
{
	Q_OBJECT
public:
	BulkDataPanel(QWidget* parent, const QString& api_key, const std::optional<long long> selected_universe_id);

	void selected_universe_changed(std::optional<long long> new_universe);

private:
	void pressed_download();

	QString api_key;
	std::optional<long long> selected_universe_id;

	QPushButton* datastore_download_button = nullptr;
};
