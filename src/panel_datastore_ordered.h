#pragma once

#include <QObject>

#include "panel_datastore_base.h"

class OrderedDatastorePanel : public BaseDatastorePanel
{
	Q_OBJECT
public:
	OrderedDatastorePanel(QWidget* parent, const QString& api_key);

private:
	virtual void clear_model() override;
	virtual void refresh_datastore_list() override;
};
