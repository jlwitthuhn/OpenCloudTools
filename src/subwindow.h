#pragma once

#include <cstdint>

#include <QString>

#include "profile.h"

enum class SubwindowType : std::uint8_t
{
	CAT_DATA_STORES,
	DATA_STORES_STANDARD,
	DATA_STORES_STANDARD_ADD,
	DATA_STORES_ORDERED,
	DATA_STORES_ORDERED_ADD,
	MEMORY_STORE_SORTED_MAP,
	BULK_DATA,
	MESSAGING,
	CAT_MODERATION,
	BAN_LIST,
	BAN_LIST_ADD,
	UNIVERSE_PREFERENCES,
};

QString subwindow_type_display_name(SubwindowType type);
QString subwindow_type_id(SubwindowType type);

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
