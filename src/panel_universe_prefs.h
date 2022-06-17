#pragma once

#include <QObject>
#include <QWidget>

class QListWidget;
class QPushButton;

class UniversePreferencesPanel : public QWidget
{
	Q_OBJECT
public:
	UniversePreferencesPanel(QWidget* parent);

	void selected_universe_changed();

private:
	void handle_ignored_datastores_changed();
	void handle_list_selection_changed();

	void pressed_add();
	void pressed_remove();

	QListWidget* hidden_datastore_list = nullptr;

	QPushButton* button_add = nullptr;
	QPushButton* button_remove = nullptr;
};
