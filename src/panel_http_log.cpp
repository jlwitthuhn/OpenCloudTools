#include "panel_http_log.h"

#include <QAbstractItemModel>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>

#include "http_wrangler.h"

HttpLogPanel::HttpLogPanel(QWidget* parent) : QWidget{ parent }
{
	tree_view = new QTreeView{ this };

	QPushButton* clear_button = new QPushButton{ "Clear", this };
	connect(clear_button, &QPushButton::clicked, this, &HttpLogPanel::pressed_clear);

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->addWidget(tree_view);
	layout->addWidget(clear_button);

	refresh();
}

void HttpLogPanel::tab_opened()
{
	refresh();
}

void HttpLogPanel::refresh()
{
	QAbstractItemModel* const existing_model = tree_view->model();
	if (existing_model)
	{
		existing_model->deleteLater();
	}
	HttpLogModel* new_model = HttpWrangler::make_log_model(this);
	tree_view->setModel(new_model);
	for (int i = 0; i < new_model->columnCount(); i++)
	{
		tree_view->resizeColumnToContents(i);
	}
}

void HttpLogPanel::pressed_clear()
{
	HttpWrangler::clear_log();
	refresh();
}
