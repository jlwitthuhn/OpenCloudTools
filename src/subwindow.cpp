#include "subwindow.h"

SubwindowId::SubwindowId(const SubwindowType type, const UniverseProfile::Id universe_id) : type{ type }, universe_id{ universe_id }
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

	if (universe_id < other.universe_id)
	{
		return true;
	}
	else if (other.universe_id < universe_id)
	{
		return false;
	}

	return false;
}
