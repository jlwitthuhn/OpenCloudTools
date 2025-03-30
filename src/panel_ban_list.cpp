#include "panel_ban_list.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>

#include "assert.h"
#include "data_request.h"
#include "diag_operation_in_progress.h"
#include "model_qt.h"

BanListPanel::BanListPanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe) :
    QWidget{ parent },
    api_key{ api_key },
    attached_universe{ universe }
{
    tree_view = new QTreeView{ this };
    tree_view->setMinimumSize(400, 200);

    QWidget* const button_bar = new QWidget{ this };
    {
        QPushButton* const refresh_button = new QPushButton{ "Refresh", button_bar };
        connect(refresh_button, &QPushButton::clicked, this, &BanListPanel::pressed_refresh);

        QHBoxLayout* const button_layout = new QHBoxLayout{ button_bar };
        button_layout->setContentsMargins(0, 0, 0, 0);
        button_layout->addWidget(refresh_button);
        button_layout->addStretch();
    }

    QVBoxLayout* const layout = new QVBoxLayout{ this };
    layout->addWidget(tree_view);
    layout->addWidget(button_bar);

    set_table_model(nullptr);
    gui_refresh();
}

void BanListPanel::gui_refresh()
{

}

void BanListPanel::set_table_model(BanListQTableModel* entry_model)
{
    if (entry_model == nullptr)
    {
        entry_model = new BanListQTableModel{ this, {} };
    }
    tree_view->setModel(entry_model);
    for (int i = 0; i < entry_model->columnCount(); i++)
    {
        tree_view->resizeColumnToContents(i);
    }
    gui_refresh();
}

void BanListPanel::pressed_refresh()
{
    auto universe = attached_universe.lock();
    OCTASSERT(universe);

    auto req = std::make_shared<UserRestrictionsGetListV2Request>(api_key, universe->get_universe_id());
    OperationInProgressDialog diag{ this, req };
    diag.exec();

    BanListQTableModel* const new_model = new BanListQTableModel{ this, req->get_restrictions() };
    set_table_model(new_model);
}
