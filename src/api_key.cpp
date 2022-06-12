#include "api_key.h"

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

const std::vector<UniverseProfile>& ApiKeyProfile::universes() const
{
	return _universes;
}
