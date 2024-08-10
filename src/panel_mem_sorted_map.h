#pragma once

#include <QObject>
#include <QWidget>

#include "util_enum.h"

class QLineEdit;
class QPushButton;

class MemoryStoreSortedMapPanel : public QWidget
{
	Q_OBJECT
public:
	MemoryStoreSortedMapPanel(QWidget* parent, const QString& api_key);

	void selected_universe_changed();

private:
	void handle_search_name_changed();

	void pressed_list_all(bool ascending);
	void pressed_list_all_asc() { return pressed_list_all(true); };
	void pressed_list_all_desc() { return pressed_list_all(false); }

	QString api_key;

	// Search panel
	QLineEdit* edit_map_name = nullptr;
	QPushButton* button_list_all_asc = nullptr;
	QPushButton* button_list_all_desc = nullptr;
	QLineEdit* edit_list_limit = nullptr;
};
