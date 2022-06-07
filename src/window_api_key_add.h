#pragma once

#include <cstddef>

#include <optional>
#include <utility>

#include <QObject>
#include <QWidget>

#include "api_key.h"

class QCheckBox;
class QLineEdit;
class QPushButton;

// This is needed to appease IWYU
constexpr size_t _window_api_key_h_dummy = sizeof(ApiKeyProfile);

class AddApiKeyWindow : public QWidget
{
	Q_OBJECT
public:
	explicit AddApiKeyWindow(QWidget* parent = nullptr, std::optional<std::pair<unsigned int, ApiKeyProfile>> existing = std::nullopt);

private:
	bool input_is_valid() const;

	void input_changed();
	void add_key();
	void update_key();

	unsigned int existing_id = 0;

	QLineEdit* name_edit = nullptr;
	QLineEdit* key_edit = nullptr;
	QCheckBox* production_check = nullptr;
	QCheckBox* save_to_disk_check = nullptr;

	QPushButton* save_button = nullptr;
};
