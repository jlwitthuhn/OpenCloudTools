#pragma once

#include <QWidget>

#include "profile.h"

enum class SubwindowType
{
	DATA_STORES_STANDARD,
};

class SubwindowId
{
public:
	SubwindowId(SubwindowType type, UniverseProfile::Id universe_profile_id);

	SubwindowType get_type() const { return type; }
	const UniverseProfile::Id& get_universe_profile_id() const { return universe_profile_id; }

	bool operator==(const SubwindowId& other) const;
	bool operator<(const SubwindowId& other) const;

private:
	SubwindowType type;
	UniverseProfile::Id universe_profile_id;
};
