#pragma once

#include <vector>

#include <QObject>
#include <QString>
#include <QDialog>

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QWidget;

class StringListDialog : public QDialog
{
	Q_OBJECT

public:
	StringListDialog(const QString& title, const std::vector<QString>& strings, QWidget* parent = nullptr);

signals:
	void selected(QString the_string);

private:
	void double_clicked_item(QListWidgetItem* item);
	void pressed_select();
	void selection_changed();

	QListWidget* string_list_widget = nullptr;
	QPushButton* select_button = nullptr;
};
