#pragma once

#include <cstddef>

#include <memory>
#include <optional>
#include <vector>

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

	std::vector<ApiKeyProfile> get_all_api_keys() const { return api_keys; };
	std::optional<ApiKeyProfile> get_api_key(size_t key_index) const;

	std::optional<size_t> add_api_key(const ApiKeyProfile& details, bool emit_signal = true);
	void update_api_key(size_t index, const ApiKeyProfile& details);
	void delete_api_key(size_t index);

	void select_api_key(std::optional<size_t> index);
	std::optional<ApiKeyProfile> get_selected_profile() const;

	std::optional<size_t> selected_profile_add_universe(const UniverseProfile& universe_profile);

	void select_universe(std::optional<size_t> universe_index);
	void remove_selected_universe();
	bool update_selected_universe(const QString& name, long long universe_id);
	std::optional<UniverseProfile> get_selected_universe() const;

	void add_hidden_datastore(const QString& datastore_name);
	void remove_hidden_datastore(const QString& datastore_name);

signals:
	void api_key_list_changed();
	void hidden_datastores_changed();
	void universe_list_changed(std::optional<size_t> selected_universe_index);
	void autoclose_changed();

private:
	explicit UserSettings(QObject* parent = nullptr);

	bool universe_name_in_use(const QString& name) const;

	void sort_universes();

	void load_from_disk();
	void save_to_disk();

	bool autoclose_progress_window = true;

	std::optional<size_t> selected_key_index;
	std::optional<size_t> selected_universe_index;

	std::vector<ApiKeyProfile> api_keys;
};
