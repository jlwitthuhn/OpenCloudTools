#pragma once

#include <optional>

#include <QObject>
#include <QWidget>

#include "profile.h"

class QCheckBox;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;

class ManageApiKeysWindow : public QWidget
{
	Q_OBJECT
public:
	explicit ManageApiKeysWindow(QWidget* parent = nullptr);
	virtual ~ManageApiKeysWindow() override;

protected:
	virtual void closeEvent(QCloseEvent* event) override;

private:
	void double_clicked_profile(QListWidgetItem* item);
	void pressed_add();
	void pressed_edit();
	void pressed_delete();
	void pressed_select();
	void rebuild_slots();
	void selection_changed();

private:
	QListWidget* list_widget = nullptr;

	QPushButton* mod_button = nullptr;
	QPushButton* del_button = nullptr;
	QPushButton* sel_button = nullptr;
};

class AddApiKeyWindow : public QWidget
{
	Q_OBJECT
public:
	explicit AddApiKeyWindow(QWidget* parent = nullptr, std::optional<ApiKeyProfile::Id> existing_key_id = std::nullopt);

private:
	bool input_is_valid() const;

	void input_changed();
	void add_key();
	void update_key();

	std::optional<ApiKeyProfile::Id> existing_key_id = std::nullopt;

	QLineEdit* name_edit = nullptr;
	QLineEdit* key_edit = nullptr;
	QCheckBox* production_check = nullptr;
	QCheckBox* save_to_disk_check = nullptr;

	QPushButton* save_button = nullptr;
};
