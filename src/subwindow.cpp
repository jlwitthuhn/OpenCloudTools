#include "subwindow.h"

#include <QString>

#include "profile.h"
#include "assert.h"

SubwindowId::SubwindowId(const SubwindowType type, const UniverseProfile::Id universe_profile_id) : type{ type }, universe_profile_id{ universe_profile_id }
{

}

QString SubwindowId::get_window_title() const
{
	const QString type_str = get_type_string();
	const QString universe_string = get_universe_string();
	return type_str + " - " + universe_string;
}

bool SubwindowId::operator<(const SubwindowId& other) const
{
	if (type < other.type)
	{
		return true;
	}
	else if (other.type < type)
	{
		return false;
	}

	if (universe_profile_id < other.universe_profile_id)
	{
		return true;
	}
	else if (other.universe_profile_id < universe_profile_id)
	{
		return false;
	}

	return false;
}

QString SubwindowId::get_type_string() const
{
	switch (type)
	{
	case SubwindowType::DATA_STORES_STANDARD:
		return "Data Stores";
	default:
		OCTASSERT(false);
		return "Invalid SubwindowType";
	}
}

QString SubwindowId::get_universe_string() const
{
	const std::shared_ptr<ApiKeyProfile> api_key = UserProfile::get_active_api_key();
	if (!api_key)
	{
		OCTASSERT(false);
		return "ERROR: No active API key";
	}
	const std::shared_ptr<UniverseProfile> universe = api_key->get_universe_profile_by_id(universe_profile_id);
	if (!universe)
	{
		OCTASSERT(false);
		return "ERROR: Invalid universe";
	}
	return universe->get_display_name();
}
