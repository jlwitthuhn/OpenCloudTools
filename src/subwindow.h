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
	SubwindowId(SubwindowType type, UniverseProfile::Id universe_id);

	SubwindowType get_type() const { return type; }
	UniverseProfile::Id get_universe_id() const { return universe_id; }

	bool operator==(const SubwindowId& other) const;
	bool operator<(const SubwindowId& other) const;

private:
	SubwindowType type;
	UniverseProfile::Id universe_id;
};
