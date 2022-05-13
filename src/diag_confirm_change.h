#pragma once

#include <QDialog>

class QCheckBox;
class QWidget;

enum class ChangeType
{
	Revert,
	Update,
	Delete,
};

class ConfirmChangeDialog : public QDialog
{
	Q_OBJECT
public:
	ConfirmChangeDialog(QWidget* parent, ChangeType change_type);

private:
	void handle_prod_confirm_check_changed();

	QCheckBox* prod_confirm_check = nullptr;
	QPushButton* yes_button = nullptr;
};
