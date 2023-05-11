#pragma once

#include <QObject>

#include "panel_datastore_base.h"

class QString;
class QWidget;

class OrderedDatastorePanel : public BaseDatastorePanel
{
	Q_OBJECT
public:
	OrderedDatastorePanel(QWidget* parent, const QString& api_key);
};
