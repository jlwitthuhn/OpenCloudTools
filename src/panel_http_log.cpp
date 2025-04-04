#include "panel_http_log.h"

#include <memory>
#include <optional>

#include <Qt>
#include <QAbstractItemModel>
#include <QAction>
#include <QClipboard>
#include <QGuiApplication>
#include <QMenu>
#include <QModelIndex>
#include <QPushButton>
#include <QString>
#include <QTreeView>
#include <QVBoxLayout>

#include "http_wrangler.h"

HttpLogPanel::HttpLogPanel(QWidget* parent) : QWidget{ parent }
{
	tree_view = new QTreeView{ this };
	tree_view->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	connect(tree_view, &QTreeView::customContextMenuRequested, this, &HttpLogPanel::pressed_right_click);

	QPushButton* clear_button = new QPushButton{ "Clear", this };
	connect(clear_button, &QPushButton::clicked, this, &HttpLogPanel::pressed_clear);

	QVBoxLayout* layout = new QVBoxLayout{ this };
	layout->addWidget(tree_view);
	layout->addWidget(clear_button);

	refresh();

	const std::unique_ptr<HttpWrangler>& wrangler = HttpWrangler::get();
	connect(wrangler.get(), &HttpWrangler::log_entry_added, this, &HttpLogPanel::handle_log_entry_added);
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
	HttpLogModel* new_model = HttpWrangler::get()->make_log_model(this);
	tree_view->setModel(new_model);
	for (int i = 0; i < new_model->columnCount(); i++)
	{
		tree_view->resizeColumnToContents(i);
	}
}

// NOLINTNEXTLINE(*-unnecessary-value-param)
void HttpLogPanel::handle_log_entry_added(const HttpLogEntry log_entry)
{
	if (HttpLogModel* const log_model = dynamic_cast<HttpLogModel*>(tree_view->model()))
	{
		log_model->append_entry(log_entry);
	}
}

void HttpLogPanel::pressed_clear()
{
	HttpWrangler::get()->clear_log();
	refresh();
}

void HttpLogPanel::pressed_right_click(const QPoint& pos)
{
	const QModelIndex the_index = tree_view->indexAt(pos);
	if (the_index.isValid())
	{
		if ( HttpLogModel* log_model = dynamic_cast<HttpLogModel*>( tree_view->model() ) )
		{
			if ( std::optional<HttpLogEntry> log_entry = log_model->get_entry( the_index.row() ) )
			{
				QMenu* context_menu = new QMenu{ tree_view };
				context_menu->setAttribute(Qt::WA_DeleteOnClose);
				{
					QAction* copy_url = new QAction{ "Copy URL", context_menu };
					connect(copy_url, &QAction::triggered, [log_entry]() {
						QClipboard* clipboard = QGuiApplication::clipboard();
						clipboard->setText(log_entry->url());
					});

					context_menu->addAction(copy_url);
				}

				context_menu->move(tree_view->mapToGlobal(pos));
				context_menu->show();
			}
		}
	}
}
