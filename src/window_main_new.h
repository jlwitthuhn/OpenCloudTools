#pragma once

#include <QMainWindow>
#include <QObject>

class QLineEdit;

class MyNewMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MyNewMainWindow();

private:
	void on_selected_api_key_changed();

	void pressed_change_key();

	QLineEdit* api_key_name_edit;
};
