#pragma once

#include <vector>

#include <QObject>
#include <QString>
#include <QWidget>

class QLineEdit;
class QModelIndex;
class QPoint;
class QPushButton;
class QTreeView;

class StandardDatastoreEntryVersion;

class ViewDatastoreEntryVersionsWindow : public QWidget
{
	Q_OBJECT
public:
	ViewDatastoreEntryVersionsWindow(QWidget* parent, const QString& api_key, long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const std::vector<StandardDatastoreEntryVersion>& versions);

	QLineEdit* universe_id_edit = nullptr;
	QLineEdit* datastore_name_edit = nullptr;
	QLineEdit* scope_edit = nullptr;
	QLineEdit* key_name_edit = nullptr;

	QTreeView* versions_tree = nullptr;

	QPushButton* refresh_button = nullptr;
	QPushButton* view_button = nullptr;
	QPushButton* revert_button = nullptr;

private:
	void revert_to_version(const QModelIndex& index);
	void view_version(const QModelIndex& index);

	void handle_selected_version_changed();
	void handle_version_double_clicked(const QModelIndex& index);

	void pressed_refresh();
	void pressed_revert();
	void pressed_right_click(const QPoint& pos);
	void pressed_view();

	QString api_key;
};
