#pragma once

#include <QObject>
#include <QWidget>

#include "util_enum.h"

class BanListUserRestriction;

class ViewBanWindow : public QWidget
{
	Q_OBJECT

public:
	ViewBanWindow(const BanListUserRestriction& user_restriction, ViewEditMode view_edit_mode, QWidget* parent = nullptr);
};
