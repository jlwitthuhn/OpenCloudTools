#pragma once

#include <QObject>
#include <QWidget>

class QPoint;
class QTreeView;

class HttpLogPanel : public QWidget
{
	Q_OBJECT
public:
	HttpLogPanel(QWidget* parent);

	void tab_opened();

private:
	void refresh();

	void pressed_clear();
	void pressed_right_click(const QPoint& pos);

	QTreeView* tree_view = nullptr;
};
