#pragma once

#include <QObject>
#include <QWidget>

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

	QTreeView* tree_view = nullptr;
};
