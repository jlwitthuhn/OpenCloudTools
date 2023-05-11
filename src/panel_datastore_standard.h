#pragma once

#include <QObject>

#include "panel_datastore_base.h"

class QString;
class QWidget;

class StandardDatastorePanel : public BaseDatastorePanel
{
	Q_OBJECT
public:
	StandardDatastorePanel(QWidget* parent, const QString& api_key);
};
