#pragma once

#include <cstddef>

#include <optional>
#include <set>
#include <vector>

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
	const std::vector<UniverseProfile>& universes() const;

	void sort_universes();

private:
	QString _name;
	QString _key;
	bool _production = false;
	bool _save_to_disk = false;

	std::vector<UniverseProfile> _universes;
};
