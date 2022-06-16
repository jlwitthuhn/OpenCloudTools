#include "user_settings.h"

#include <utility>
#include <vector>

#include <QSettings>
#include <QString>
#include <QVariant>

std::unique_ptr<UserSettings>& UserSettings::get()
{
	static std::unique_ptr<UserSettings> manager = std::unique_ptr<UserSettings>(new UserSettings());
	return manager;
}

void UserSettings::set_autoclose_progress_window(const bool autoclose)
{
	if (autoclose_progress_window != autoclose)
	{
		autoclose_progress_window = autoclose;
		emit autoclose_changed();
		save_to_disk();
	}
}

std::optional<ApiKeyProfile> UserSettings::get_api_key(const size_t key_index) const
{
	if (key_index < api_keys.size())
	{
		return api_keys.at(key_index);
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<size_t> UserSettings::add_api_key(const ApiKeyProfile& details, const bool emit_signal)
{
	if (universe_name_in_use( details.name() ))
	{
		return std::nullopt;
	}
	else
	{
		const size_t this_index = api_keys.size();
		api_keys.push_back(details);
		if (emit_signal)
		{
			emit api_key_list_changed();
		}
		return this_index;
	}
}

void UserSettings::update_api_key(const size_t index, const ApiKeyProfile& details)
{
	std::optional<ApiKeyProfile> existing = get_api_key(index);
	if (existing)
	{
		// Copy existing universe list, this operation only effects top-level params
		ApiKeyProfile to_insert{ details };
		for (const UniverseProfile& this_universe_profile : existing->universes())
		{
			to_insert.add_universe(this_universe_profile);
		}

		api_keys.at(index) = to_insert;
		emit api_key_list_changed();
	}
}

void UserSettings::delete_api_key(const size_t index)
{
	if (index < api_keys.size())
	{
		api_keys.erase(api_keys.begin() + index);
		emit api_key_list_changed();
	}
}

void UserSettings::select_api_key(const std::optional<size_t> index)
{
	if (index)
	{
		selected_key_index = *index;
	}
	else
	{
		selected_key_index = std::nullopt;
	}
	selected_universe_index = std::nullopt;
}

std::optional<ApiKeyProfile> UserSettings::get_selected_profile() const
{
	if (selected_key_index)
	{
		return get_api_key(*selected_key_index);
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<size_t> UserSettings::selected_profile_add_universe(const UniverseProfile& universe_profile)
{
	if (selected_key_index && *selected_key_index < api_keys.size())
	{
		ApiKeyProfile& this_profile = api_keys.at(*selected_key_index);
		const std::optional<size_t> new_universe_index = this_profile.add_universe(universe_profile);
		if (new_universe_index)
		{
			selected_universe_index = *new_universe_index;
			sort_universes();
			emit universe_list_changed(*selected_universe_index);
		}
		return new_universe_index;
	}
	return std::nullopt;
}

void UserSettings::select_universe(const std::optional<size_t> universe_index)
{
	selected_universe_index = universe_index;
}

void UserSettings::remove_selected_universe()
{
	if (selected_key_index && *selected_key_index < api_keys.size() && selected_universe_index)
	{
		ApiKeyProfile& this_profile = api_keys.at(*selected_key_index);
		this_profile.remove_universe(*selected_universe_index);
		emit universe_list_changed(std::nullopt);
	}
}

bool UserSettings::update_selected_universe(const QString& name, const long long universe_id)
{
	if (selected_key_index && *selected_key_index < api_keys.size() && selected_universe_index)
	{
		ApiKeyProfile& this_profile = api_keys.at(*selected_key_index);
		for (const UniverseProfile& this_profile : this_profile.universes())
		{
			if (this_profile.name() == name && this_profile.universe_id() == universe_id)
			{
				return false;
			}
		}
		const bool success = this_profile.update_universe_details(*selected_universe_index, name, universe_id);
		if (success)
		{
			sort_universes();
			emit universe_list_changed(*selected_universe_index);
			return true;
		}
	}
	return false;
}

std::optional<UniverseProfile> UserSettings::get_selected_universe() const
{
	if (selected_key_index && *selected_key_index < api_keys.size())
	{
		const ApiKeyProfile& this_profile = api_keys.at(*selected_key_index);
		if (selected_universe_index && *selected_universe_index < this_profile.universes().size())
		{
			return this_profile.universes().at(*selected_universe_index);
		}
	}
	return std::nullopt;
}

bool UserSettings::universe_name_in_use(const QString& name) const
{
	bool result = false;
	for (const ApiKeyProfile& this_profile : api_keys)
	{
		result = result || (name == this_profile.name());
	}
	return result;
}

void UserSettings::sort_universes()
{
	const std::optional<UniverseProfile> originally_selected = get_selected_universe();
	if (selected_key_index && *selected_key_index < api_keys.size())
	{
		ApiKeyProfile& this_profile = api_keys.at(*selected_key_index);
		this_profile.sort_universes();

		if (originally_selected)
		{
			for (size_t i = 0; i < this_profile.universes().size(); i++)
			{
				if (originally_selected->matches_name_and_id(this_profile.universes().at(i)))
				{
					selected_universe_index = i;
				}
			}
		}
	}
}

void UserSettings::load_from_disk()
{
	QSettings settings;

	settings.beginGroup("prefs");
	autoclose_progress_window = settings.value("autoclose_progress_window").toBool();
	settings.endGroup();

	settings.beginGroup("api_keys");

	QVariant settings_version = settings.value("version");
	if (settings_version.toUInt() == 1)
	{
		settings.beginGroup("keys");
		QVariant keys_version = settings.value("version");

		const int key_list_size = settings.beginReadArray("list");
		for (int i = 0; i < key_list_size; i++)
		{
			settings.setArrayIndex(i);
			QString name = settings.value("name").toString();
			QString key = settings.value("key").toString();
			bool production = settings.value("production").toBool();
			if (name.size() > 0 && key.size() > 0)
			{
				ApiKeyProfile loaded_key{ name, key, production, true };
				const int universe_list_size = settings.beginReadArray("universe_ids");
				for (int j = 0; j < universe_list_size; j++)
				{
					settings.setArrayIndex(j);
					const long long this_universe_id = settings.value("universe_id").toLongLong();

					QString name = "Unnamed";
					auto maybe_name = settings.value("name");
					if (maybe_name.isNull() == false)
					{
						name = maybe_name.toString();
					}

					loaded_key.add_universe(UniverseProfile{ name, this_universe_id });
				}
				settings.endArray();
				add_api_key(loaded_key, false);
			}
		}
		settings.endArray();

		settings.endGroup();
	}

	settings.endGroup();

	emit api_key_list_changed();
}

void UserSettings::save_to_disk()
{
	QSettings settings;

	settings.beginGroup("prefs");
	settings.setValue("autoclose_progress_window", autoclose_progress_window);
	settings.endGroup();

	settings.beginGroup("api_keys");

	settings.setValue("version", static_cast<unsigned int>(1));

	settings.beginGroup("keys");
	settings.setValue("version", static_cast<unsigned int>(1));
	settings.beginWriteArray("list");
	{
		int next_array_index = 0;
		for (const ApiKeyProfile& this_key : api_keys)
		{
			if (this_key.save_to_disk())
			{
				settings.setArrayIndex(next_array_index++);
				settings.setValue("name", this_key.name());
				settings.setValue("key", this_key.key());
				settings.setValue("production", this_key.production());

				settings.beginWriteArray("universe_ids");
				{
					int next_universe_array_index = 0;
					for (const UniverseProfile& this_universe_profile : this_key.universes())
					{
						settings.setArrayIndex(next_universe_array_index++);
						settings.setValue("name", this_universe_profile.name());
						settings.setValue("universe_id", this_universe_profile.universe_id());
					}
				}
				settings.endArray();
			}
		}
	}
	settings.endArray();
	settings.endGroup();

	settings.endGroup();
}

UserSettings::UserSettings(QObject* parent) : QObject{ parent }
{
	load_from_disk();
	connect(this, &UserSettings::api_key_list_changed, this, &UserSettings::save_to_disk);
	connect(this, &UserSettings::universe_list_changed, this, &UserSettings::save_to_disk);
}
