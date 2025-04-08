#pragma once

#include <memory>
#include <optional>

#include <QObject>
#include <QString>
#include <QWidget>

class QCheckBox;
class QModelIndex;
class QPushButton;
class QTreeView;

class BanListQTableModel;
class BanListUserRestriction;
class UniverseProfile;

class BanListPanel : public QWidget
{
Q_OBJECT

public:
	BanListPanel(QWidget* parent, const QString& api_key, const std::shared_ptr<UniverseProfile>& universe);

private:
	void gui_refresh();

	QModelIndex get_selected_single_index() const;
	std::optional<BanListUserRestriction> get_selected_restriction() const;

	void set_table_model(BanListQTableModel* entry_model);

	void handle_selected_ban_changed();

	void pressed_details();
	void pressed_edit();
	void pressed_refresh();
	void pressed_unban();

	QString api_key;
	std::weak_ptr<UniverseProfile> attached_universe;

	QCheckBox* active_only_check = nullptr;

	QTreeView* tree_view = nullptr;

	QPushButton* unban_button = nullptr;
	QPushButton* edit_button = nullptr;
	QPushButton* details_button = nullptr;
};
