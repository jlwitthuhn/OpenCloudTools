#pragma once

#include <QObject>
#include <QWidget>

class QCheckBox;
class QLineEdit;

class UniverseProfile;

class BanAddPanel : public QWidget
{
	Q_OBJECT

public:
	BanAddPanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe);

private:
	void pressed_ban();

	QString api_key;
	std::weak_ptr<UniverseProfile> attached_universe;

	QLineEdit* user_id_edit = nullptr;
	QLineEdit* duration_edit = nullptr;
	QLineEdit* private_reason_edit = nullptr;
	QLineEdit* display_reason_edit = nullptr;
	QCheckBox* exclude_alts_check = nullptr;
};
