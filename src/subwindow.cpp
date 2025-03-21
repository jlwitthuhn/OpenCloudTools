#include "subwindow.h"

#include <memory>

#include <QString>

#include "profile.h"
#include "assert.h"

QString subwindow_type_display_name(const SubwindowType type)
{
	switch (type)
	{
	case SubwindowType::DATA_STORES_STANDARD:
		return "Data Stores";
	case SubwindowType::DATA_STORES_STANDARD_ADD:
		return "Add to Data Store";
	case SubwindowType::DATA_STORES_ORDERED:
		return "Ordered Data Stores";
	case SubwindowType::MEMORY_STORE_SORTED_MAP:
		return "Memory Store, Sorted Map";
	case SubwindowType::BULK_DATA:
		return "Bulk Data";
	case SubwindowType::MESSAGING:
		return "Messaging Service";
	case SubwindowType::UNIVERSE_PREFERENCES:
		return "Universe Preferences";
	}
	OCTASSERT(false);
	return "ERROR: Invalid SubwindowType";
}

SubwindowId::SubwindowId(const SubwindowType type, const UniverseProfile::Id universe_profile_id) : type{ type }, universe_profile_id{ universe_profile_id }
{

}

QString SubwindowId::get_window_title() const
{
	const QString type_str = subwindow_type_display_name(type);
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
