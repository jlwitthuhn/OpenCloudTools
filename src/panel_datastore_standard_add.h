#pragma once

#include <memory>

#include <QObject>
#include <QWidget>

#include "profile.h"

class QComboBox;
class QLineEdit;
class QPushButton;
class QTextEdit;

class StandardDatastoreAddEntryPanel : public QWidget
{
	Q_OBJECT

public:
	StandardDatastoreAddEntryPanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe);

private:
	void gui_refresh();

	void datastore_name_selected(const QString& name);

	void pressed_select_datastore();
	void pressed_submit();

	QString api_key;
	std::weak_ptr<UniverseProfile> attached_universe;

	QLineEdit* edit_add_datastore_name = nullptr;
	QLineEdit* edit_add_datastore_scope = nullptr;
	QLineEdit* edit_add_datastore_key_name = nullptr;

	QComboBox* combo_add_entry_type = nullptr;

	QTextEdit* edit_add_entry_data = nullptr;
	QTextEdit* edit_add_entry_userids = nullptr;
	QTextEdit* edit_add_entry_attributes = nullptr;

	QPushButton* button_add_entry_submit = nullptr;
};
