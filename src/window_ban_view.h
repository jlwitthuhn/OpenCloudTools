#pragma once

#include <cstdint>

#include <QObject>
#include <QString>
#include <QWidget>

class QCheckBox;
class QLineEdit;

class BanListUserRestriction;

enum class ViewEditMode : std::uint8_t;

class ViewBanWindow : public QWidget
{
	Q_OBJECT

public:
	ViewBanWindow(ViewEditMode view_edit_mode, const QString& api_key, const BanListUserRestriction& user_restriction, QWidget* parent = nullptr);

	void pressed_submit();

private:
	QString api_key;

	QLineEdit* path_edit = nullptr;
	QCheckBox* update_active_check = nullptr;
	QLineEdit* update_duration_edit = nullptr;
	QLineEdit* update_private_reason_edit = nullptr;
	QLineEdit* update_display_reason_edit = nullptr;
	QCheckBox* update_exclude_alt_accounts_check = nullptr;
};
