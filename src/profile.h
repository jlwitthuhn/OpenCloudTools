#pragma once

#include <cstddef>

#include <memory>
#include <optional>
#include <vector>
#include <set>

#include <QObject>
#include <QString>

class UniverseProfile
{
public:
	UniverseProfile(const QString& name, long long universe_id);

	bool matches_name_and_id(const UniverseProfile& other) const;

	QString name() const { return _name; }
	long long universe_id() const { return _universe_id; }
	const std::set<QString>& hidden_datastores() const { return _hidden_datastores; }

	void set_name(const QString& name_in) { _name = name_in; }
	void set_universe_id(const long long universe_id_in) { _universe_id = universe_id_in; }
	void add_hidden_datastore(const QString& datastore) { _hidden_datastores.insert(datastore); }
	void remove_hidden_datastore(const QString& datastore) { _hidden_datastores.erase(datastore); }

private:
	QString _name;
	long long _universe_id;
	std::set<QString> _hidden_datastores;
};

class ApiKeyProfile
{
public:
	ApiKeyProfile(const QString& name, const QString& key, bool production, bool save_to_disk);

	QString name() const { return _name; };
	QString key() const { return _key; };
	bool production() const { return _production; }
	bool save_to_disk() const { return _save_to_disk; }

	std::optional<size_t> add_universe(const UniverseProfile& universe_profile);
	void remove_universe(size_t universe_index);
	bool update_universe_details(size_t universe_index, const QString& name, long long universe_id);
	void add_hidden_datastore(size_t universe_index, const QString& datastore_name);
	void remove_hidden_datastore(size_t universe_index, const QString& datastore_name);
	const std::vector<UniverseProfile>& universes() const { return _universes; }

	void sort_universes();

private:
	QString _name;
	QString _key;
	bool _production = false;
	bool _save_to_disk = false;

	std::vector<UniverseProfile> _universes;
};

class UserProfile : public QObject
{
	Q_OBJECT
public:
	static std::unique_ptr<UserProfile>& get();

	bool get_autoclose_progress_window() const { return autoclose_progress_window; }
	void set_autoclose_progress_window(bool autoclose);

	bool get_less_verbose_bulk_operations() const { return less_verbose_bulk_operations; }
	void set_less_verbose_bulk_operations(bool less_verbose);

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
	explicit UserProfile(QObject* parent = nullptr);

	bool universe_name_in_use(const QString& name) const;

	void sort_universes();

	void load_from_disk();
	void save_to_disk();

	bool autoclose_progress_window = true;
	bool less_verbose_bulk_operations = true;

	std::optional<size_t> selected_key_index;
	std::optional<size_t> selected_universe_index;

	std::vector<ApiKeyProfile> api_keys;
};
