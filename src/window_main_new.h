#pragma once

#include <QMainWindow>
#include <QObject>

class MyNewMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MyNewMainWindow();

private:
	void pressed_change_key();
};
