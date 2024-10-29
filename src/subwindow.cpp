#include "subwindow.h"

SubwindowId::SubwindowId(const SubwindowType type, const UniverseProfile::Id universe_profile_id) : type{ type }, universe_profile_id{ universe_profile_id }
{

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
