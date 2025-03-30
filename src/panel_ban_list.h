#pragma once

#include <memory>

#include <QObject>
#include <QString>
#include <QWidget>

#include "profile.h"

class QTreeView;

class BanListQTableModel;

class BanListPanel : public QWidget
{
Q_OBJECT

public:
    BanListPanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe);

private:
    void gui_refresh();

    void set_table_model(BanListQTableModel* entry_model);

    void pressed_refresh();

    QString api_key;
    std::weak_ptr<UniverseProfile> attached_universe;

    QTreeView* tree_view = nullptr;
};
