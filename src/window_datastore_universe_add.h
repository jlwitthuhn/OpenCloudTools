#pragma once

#include <QObject>
#include <QWidget>

class QLineEdit;
class QPushButton;

class AddUniverseToDatastoreWindow : public QWidget
{
	Q_OBJECT
public:
	explicit AddUniverseToDatastoreWindow(QWidget* parent = nullptr, bool edit_current = false);

private:
	bool input_is_valid() const;

	void text_changed();
	void pressed_add();

	bool edit_mode = false;

	QLineEdit* name_edit = nullptr;
	QLineEdit* id_edit = nullptr;

	QPushButton* add_button = nullptr;
};
