#pragma once

#include <QObject>
#include <QWidget>

class QLineEdit;
class QPushButton;

class AddHiddenDatastoreWindow : public QWidget
{
	Q_OBJECT
public:
	AddHiddenDatastoreWindow(QWidget* parent);

private:
	void handle_text_changed();

	void pressed_add();

	QLineEdit* name_edit = nullptr;

	QPushButton* add_button;
};
