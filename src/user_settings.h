#pragma once

#include <map>
#include <memory>
#include <optional>

#include <QObject>
#include <QString>

#include "api_key.h"

class UserSettings : public QObject
{
	Q_OBJECT
public:
	static std::unique_ptr<UserSettings>& get();

	bool get_autoclose_progress_window() const { return autoclose_progress_window; }
	void set_autoclose_progress_window(bool autoclose);

	std::map<unsigned int, ApiKeyProfile> get_all_api_keys() const { return api_keys; };
	std::optional<ApiKeyProfile> get_api_key(unsigned int id) const;

	void add_api_key(const ApiKeyProfile& details, bool emit_signal = true);
	void update_api_key(unsigned int id, const ApiKeyProfile& details);
	void delete_api_key(unsigned int id);

	void select_api_key(unsigned int id);
	std::optional<ApiKeyProfile> get_selected_profile() const;
	void selected_add_universe(long long universe_id, const QString& name);
	void selected_remove_universe(long long universe_id);

signals:
	void api_key_list_changed();
	void universe_list_changed();
	void autoclose_changed();

private:
	explicit UserSettings(QObject* parent = nullptr);

	void load_from_disk();
	void save_to_disk();

	bool autoclose_progress_window = true;

	std::optional<unsigned int> selected_key;

	unsigned int next_api_key_id = 1;
	std::map<unsigned int, ApiKeyProfile> api_keys;
};
