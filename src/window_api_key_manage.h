#pragma once

#include <QObject>
#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QPushButton;

class ManageApiKeysWindow : public QWidget
{
	Q_OBJECT
public:
	explicit ManageApiKeysWindow(QWidget* parent = nullptr);
	virtual ~ManageApiKeysWindow() override;

private:
	void double_clicked_profile(QListWidgetItem* item);
	void pressed_add();
	void pressed_edit();
	void pressed_delete();
	void pressed_select();
	void rebuild_slots();
	void selection_changed();

private:
	QListWidget* list_widget = nullptr;

	QPushButton* mod_button = nullptr;
	QPushButton* del_button = nullptr;
	QPushButton* sel_button = nullptr;
};
