#include "diag_list_string.h"

#include <QHBoxLayout>
#include <QList>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

StringListDialog::StringListDialog(const QString& title, const std::vector<QString>& strings, QWidget* const parent) : QDialog{ parent }
{
	setWindowTitle(title);

	string_list_widget = new QListWidget{ this };
	connect(string_list_widget, &QListWidget::itemDoubleClicked, this, &StringListDialog::double_clicked_item);
	connect(string_list_widget, &QListWidget::itemSelectionChanged, this, &StringListDialog::selection_changed);
	for (const QString& this_string : strings)
	{
		string_list_widget->addItem(this_string);
	}

	QWidget* const button_line = new QWidget{ this };
	{
		select_button = new QPushButton{ "Select", button_line };
		select_button->setEnabled(false);
		connect(select_button, &QPushButton::clicked, this, &StringListDialog::pressed_select);

		QPushButton* const cancel_button = new QPushButton{ "Cancel", button_line };
		connect(cancel_button, &QPushButton::clicked, this, &QWidget::close);

		QHBoxLayout* const button_layout = new QHBoxLayout{ button_line };
		button_layout->setContentsMargins(0, 0, 0, 0);
		button_layout->addWidget(select_button);
		button_layout->addWidget(cancel_button);
	}

	QVBoxLayout* const layout = new QVBoxLayout{ this };
	layout->addWidget(string_list_widget);
	layout->addWidget(button_line);
}

void StringListDialog::double_clicked_item(QListWidgetItem* const item)
{
	emit selected(item->text());
	close();
}

void StringListDialog::pressed_select()
{
	const QList<QListWidgetItem*> items = string_list_widget->selectedItems();
	if (items.size() == 1)
	{
		emit selected(items[0]->text());
	}
	close();
}

void StringListDialog::selection_changed()
{
	const QList<QListWidgetItem*> selected = string_list_widget->selectedItems();
	select_button->setEnabled(selected.size() == 1);
}
