#pragma once

#include <cstdint>

#include <QDialog>
#include <QObject>
#include <QString>

class QCheckBox;
class QPushButton;
class QWidget;

enum class ChangeType : std::uint8_t
{
	BanListUnbanUser,
	BanListUpdateRestriction,
	OrderedDatastoreCreate,
	OrderedDatastoreDelete,
	OrderedDatastoreIncrement,
	OrderedDatastoreUpdate,
	StandardDatastoreRevert,
	StandardDatastoreUpdate,
	StandardDatastoreDelete,
	StandardDatastoreMultiDelete,
	StandardDatastoreBulkDelete,
	StandardDatastoreBulkUndelete,
	StandardDatastoreBulkUpload,
};

class ConfirmChangeDialog : public QDialog
{
	Q_OBJECT

public:
	ConfirmChangeDialog(QWidget* parent, ChangeType change_type, const QString& name = "?");

private:
	void handle_prod_confirm_check_changed();

	QCheckBox* prod_confirm_check = nullptr;
	QPushButton* yes_button = nullptr;
};
