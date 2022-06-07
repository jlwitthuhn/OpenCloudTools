#include "api_key.h"

ApiKeyProfile::ApiKeyProfile(const QString& name, const QString& key, const bool production, const bool save_to_disk) : _name{ name }, _key{ key }, _production{ production }, _save_to_disk{ save_to_disk }
{

}

void ApiKeyProfile::add_universe(const long long universe_id, const QString& name)
{
	_universe_ids[universe_id] = name;
}

void ApiKeyProfile::remove_universe(const long long universe_id)
{
	_universe_ids.erase(universe_id);
}

std::map<long long, QString> ApiKeyProfile::universe_ids() const
{
	std::map<long long, QString> result{ _universe_ids.begin(), _universe_ids.end() };
	return result;
}
