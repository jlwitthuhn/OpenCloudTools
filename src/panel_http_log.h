#pragma once

#include <QObject>
#include <QWidget>

class QPoint;
class QTreeView;

class HttpLogEntry;

class HttpLogPanel : public QWidget
{
	Q_OBJECT
public:
	HttpLogPanel(QWidget* parent);

	void tab_opened();

private:
	void refresh();

#ifdef OCT_NEW_GUI
	void handle_log_entry_added(HttpLogEntry log_entry);
#endif

	void pressed_clear();
	void pressed_right_click(const QPoint& pos);

	QTreeView* tree_view = nullptr;
};
