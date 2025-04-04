#pragma once

#include <cstdint>
#include <map>
#include <memory>

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QWidget>

class QCheckBox;
class QLineEdit;
class QListWidget;
class QPushButton;

class UniverseProfile;

enum class SubwindowType : std::uint8_t;

class UniversePreferencesPanel : public QWidget
{
	Q_OBJECT
public:
	UniversePreferencesPanel(QWidget* parent, const QString& dummy, const std::shared_ptr<UniverseProfile>& universe);

private:
	void gui_refresh();

	void handle_hidden_datastores_changed();
	void handle_hidden_operation_checkbox_changed();
	void handle_list_selection_changed();

	void pressed_add();
	void pressed_remove();

	std::weak_ptr<UniverseProfile> attached_universe;
	QMetaObject::Connection conn_universe_hidden_datastores_changed;

	std::map<QCheckBox*, SubwindowType> checkbox_to_type;

	QListWidget* hidden_datastore_list = nullptr;

	QPushButton* button_add = nullptr;
	QPushButton* button_remove = nullptr;
};

class UniversePreferencesAddHiddenDatastoreWindow : public QWidget
{
	Q_OBJECT
public:
	UniversePreferencesAddHiddenDatastoreWindow(QWidget* parent, const std::shared_ptr<UniverseProfile>& universe);

private:
	void handle_text_changed();

	void pressed_add();

	std::weak_ptr<UniverseProfile> attached_universe;

	QLineEdit* name_edit = nullptr;

	QPushButton* add_button;
};
