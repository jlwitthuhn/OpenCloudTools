#pragma once

#include <QObject>
#include <QWidget>

class QLineEdit;
class QListWidget;
class QPushButton;

class UniversePreferencesPanel : public QWidget
{
	Q_OBJECT
public:
	UniversePreferencesPanel(QWidget* parent);

	void selected_universe_changed();

private:
	void handle_hidden_datastores_changed();
	void handle_list_selection_changed();

	void pressed_add();
	void pressed_remove();

	QListWidget* hidden_datastore_list = nullptr;

	QPushButton* button_add = nullptr;
	QPushButton* button_remove = nullptr;
};

class UniversePreferencesAddHiddenDatastoreWindow : public QWidget
{
	Q_OBJECT
public:
	UniversePreferencesAddHiddenDatastoreWindow(QWidget* parent);

private:
	void handle_text_changed();

	void pressed_add();

	QLineEdit* name_edit = nullptr;

	QPushButton* add_button;
};