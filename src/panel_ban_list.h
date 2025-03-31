#pragma once

#include <memory>

#include <QObject>
#include <QString>
#include <QWidget>

#include "profile.h"

class QModelIndex;
class QPushButton;
class QTreeView;

class BanListQTableModel;

class BanListPanel : public QWidget
{
Q_OBJECT

public:
    BanListPanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe);

private:
    void gui_refresh();

    QModelIndex get_selected_single_index() const;

    void set_table_model(BanListQTableModel* entry_model);

    void handle_selected_ban_changed();

    void pressed_details();
    void pressed_refresh();

    QString api_key;
    std::weak_ptr<UniverseProfile> attached_universe;

    QTreeView* tree_view = nullptr;

    QPushButton* details_button = nullptr;
};
