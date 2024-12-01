#pragma once

#include <QWidget>

#include "profile.h"

class QString;

enum class SubwindowType
{
	DATA_STORES_STANDARD,
	DATA_STORES_ORDERED,
	MEMORY_STORE_SORTED_MAP,
	BULK_DATA,
	MESSAGING,
	UNIVERSE_PREFERENCES,
};

QString subwindow_type_display_name(SubwindowType type);

class SubwindowId
{
public:
	SubwindowId(SubwindowType type, UniverseProfile::Id universe_profile_id);

	SubwindowType get_type() const { return type; }
	const UniverseProfile::Id& get_universe_profile_id() const { return universe_profile_id; }

	QString get_window_title() const;

	bool operator==(const SubwindowId& other) const;
	bool operator<(const SubwindowId& other) const;

private:
	QString get_universe_string() const;

	SubwindowType type;
	UniverseProfile::Id universe_profile_id;
};
