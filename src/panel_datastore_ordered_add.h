#pragma once

#include <memory>

#include <QObject>
#include <QString>
#include <QWidget>

class QLineEdit;
class QPushButton;

class UniverseProfile;

class OrderedDatastoreAddEntryPanel : public QWidget
{
	Q_OBJECT

public:
	OrderedDatastoreAddEntryPanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe);

private:
	void gui_refresh();

	void datastore_name_selected(const QString& name);

	void pressed_select_datastore();
	void pressed_submit();

	QString api_key;
	std::weak_ptr<UniverseProfile> attached_universe;

	QLineEdit* datastore_name_edit = nullptr;
	QLineEdit* scope_edit = nullptr;
	QLineEdit* key_name_edit = nullptr;
	QLineEdit* value_edit = nullptr;

	QPushButton* button_submit = nullptr;
};
