#include "api_key.h"

#include <algorithm>

UniverseProfile::UniverseProfile(const QString& name, const long long universe_id) : _name{ name }, _universe_id{ universe_id }
{

}

bool UniverseProfile::matches_name_and_id(const UniverseProfile& other) const
{
	return _name == other._name && _universe_id == other._universe_id;
}

ApiKeyProfile::ApiKeyProfile(const QString& name, const QString& key, const bool production, const bool save_to_disk) : _name{ name }, _key{ key }, _production{ production }, _save_to_disk{ save_to_disk }
{

}

std::optional<size_t> ApiKeyProfile::add_universe(const UniverseProfile& universe_profile)
{
	for (const UniverseProfile& existing_profile : _universes)
	{
		if (universe_profile.matches_name_and_id(existing_profile))
		{
			// Name + ID should uniquely identify a given slot
			return std::nullopt;
		}
	}
	const size_t next_index = _universes.size();
	_universes.push_back(universe_profile);
	return next_index;
}

void ApiKeyProfile::remove_universe(const size_t universe_index)
{
	if (universe_index < _universes.size())
	{
		_universes.erase(_universes.begin() + universe_index);
	}
}

bool ApiKeyProfile::update_universe_details(const size_t universe_index, const QString& name, long long universe_id)
{
	if (universe_index < _universes.size())
	{
		_universes.at(universe_index).set_name(name);
		_universes.at(universe_index).set_universe_id(universe_id);
		return true;
	}
	return false;
}
void ApiKeyProfile::add_hidden_datastore(const size_t universe_index, const QString& datastore_name)
{
	if (universe_index < _universes.size())
	{
		_universes.at(universe_index).add_hidden_datastore(datastore_name);
	}
}
void ApiKeyProfile::remove_hidden_datastore(const size_t universe_index, const QString& datastore_name)
{
	if (universe_index < _universes.size())
	{
		_universes.at(universe_index).remove_hidden_datastore(datastore_name);
	}
}

const std::vector<UniverseProfile>& ApiKeyProfile::universes() const
{
	return _universes;
}

void ApiKeyProfile::sort_universes()
{
	std::sort(_universes.begin(), _universes.end(), [](const UniverseProfile& a, const UniverseProfile& b) -> bool {
		if (a.name() < b.name())
		{
			return true;
		}
		else if (b.name() < a.name())
		{
			return false;
		}

		if (a.universe_id() < b.universe_id())
		{
			return true;
		}
		else if (b.universe_id() < a.universe_id())
		{
			return false;
		}
		return false;
	});
}
