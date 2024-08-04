#pragma once

#include <QObject>
#include <QWidget>

#include "util_enum.h"

class MemoryStoreSortedMapPanel : public QWidget
{
	Q_OBJECT
public:
	MemoryStoreSortedMapPanel(QWidget* parent, const QString& api_key);

	void selected_universe_changed();

private:
	QString api_key;
};
