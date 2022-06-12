#pragma once

#include <cstddef>

#include <optional>
#include <vector>

#include <QString>

class UniverseProfile
{
public:
	UniverseProfile(const QString& name, long long universe_id);

	bool matches_name_and_id(const UniverseProfile& other) const;

	QString name() const { return _name; }
	long long universe_id() const { return _universe_id; }

	void set_name(const QString& name_in) { _name = name_in; }
	void set_universe_id(const long long universe_id_in) { _universe_id = universe_id_in; }

private:
	QString _name;
	long long _universe_id;
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
	const std::vector<UniverseProfile>& universes() const;

	void sort_universes();

private:
	QString _name;
	QString _key;
	bool _production = false;
	bool _save_to_disk = false;

	std::vector<UniverseProfile> _universes;
};
