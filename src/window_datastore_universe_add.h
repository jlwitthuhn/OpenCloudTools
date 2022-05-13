#pragma once

#include <QWidget>

class QLineEdit;
class QPushButton;

class AddUniverseToDatastoreWindow : public QWidget
{
	Q_OBJECT
public:
	explicit AddUniverseToDatastoreWindow(QWidget* parent = nullptr);

private:
	bool input_is_valid() const;

	void text_changed();
	void pressed_add();

	uint existing_id = 0;

	QLineEdit* name_edit = nullptr;
	QLineEdit* id_edit = nullptr;

	QPushButton* add_button = nullptr;
};
