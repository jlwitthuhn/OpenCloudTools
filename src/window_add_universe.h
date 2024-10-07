#pragma once

#include <memory>

#include <QObject>
#include <QString>
#include <QWidget>

class QLineEdit;
class QPushButton;

class UniverseProfile;

class AddUniverseWindow : public QWidget
{
	Q_OBJECT
public:
	AddUniverseWindow(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& existing_universe);

private:
	bool id_is_valid() const;
	bool name_is_valid() const;

	void text_changed();
	void pressed_add();
	void pressed_fetch();


	QString api_key;
	std::weak_ptr<UniverseProfile> attached_universe;

	QLineEdit* name_edit = nullptr;
	QLineEdit* id_edit = nullptr;

	QPushButton* fetch_name_button = nullptr;
	QPushButton* add_button = nullptr;
};
