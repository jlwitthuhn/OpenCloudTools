#pragma once

#include <QWidget>

class QListWidget;
class QPushButton;

class ManageApiKeysWindow : public QWidget
{
	Q_OBJECT
public:
	explicit ManageApiKeysWindow(QWidget* parent = nullptr);
	virtual ~ManageApiKeysWindow() override;

private:
	void click_add();
	void click_ed();
	void click_del();
	void click_sel();
	void rebuild_slots();
	void selection_changed();

private:
	QListWidget* list_widget = nullptr;

	QPushButton* mod_button = nullptr;
	QPushButton* del_button = nullptr;
	QPushButton* sel_button = nullptr;
};
