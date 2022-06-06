#include "user_settings.h"

#include <QSettings>

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

std::optional<ApiKeyProfile> UserSettings::get_api_key(const unsigned int id) const
{
	auto it = api_keys.find(id);
	if (it != api_keys.end())
	{
		return it->second;
	}
	else
	{
		return std::nullopt;
	}
}

void UserSettings::add_api_key(const ApiKeyProfile& details, const bool emit_signal)
{
	api_keys.insert(std::pair{ next_api_key_id++, details });
	if (emit_signal)
	{
		emit api_key_list_changed();
	}
}

void UserSettings::update_api_key(const unsigned int id, const ApiKeyProfile& details)
{
	std::optional<ApiKeyProfile> existing = get_api_key(id);
	if (existing)
	{
		// Copy existing universe list, this operation only effects top-level params
		ApiKeyProfile to_insert{ details };
		for (auto this_pair : existing->universe_ids())
		{
			to_insert.add_universe(this_pair.first, this_pair.second);
		}

		api_keys.insert_or_assign(id, to_insert);
		emit api_key_list_changed();
	}
}

void UserSettings::delete_api_key(const unsigned int id)
{
	api_keys.erase(id);
	emit api_key_list_changed();
}

void UserSettings::select_api_key(const unsigned int id)
{
	if (id > 0)
	{
		selected_key = id;
	}
	else
	{
		selected_key = std::nullopt;
	}
}

std::optional<ApiKeyProfile> UserSettings::get_selected_profile() const
{
	if (selected_key)
	{
		return get_api_key(*selected_key);
	}
	else
	{
		return std::nullopt;
	}
}

void UserSettings::selected_add_universe(const long long universe_id, const QString& name)
{
	if (selected_key)
	{
		auto selected_it = api_keys.find(*selected_key);
		if (selected_it != api_keys.end())
		{
			selected_it->second.add_universe(universe_id, name);
			emit universe_list_changed();
		}
	}
}

void UserSettings::selected_remove_universe(const long long universe_id)
{
	if (selected_key)
	{
		auto selected_it = api_keys.find(*selected_key);
		if (selected_it != api_keys.end())
		{
			selected_it->second.remove_universe(universe_id);
			emit universe_list_changed();
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
					loaded_key.add_universe(this_universe_id, name);
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
		for (const std::pair<const unsigned int, ApiKeyProfile>& this_pair : api_keys)
		{
			if (this_pair.second.save_to_disk())
			{
				settings.setArrayIndex(next_array_index++);
				settings.setValue("name", this_pair.second.name());
				settings.setValue("key", this_pair.second.key());
				settings.setValue("production", this_pair.second.production());

				settings.beginWriteArray("universe_ids");
				{
					int next_universe_array_index = 0;
					for (const std::pair<const long long, QString>& this_inner_pair : this_pair.second.universe_ids())
					{
						settings.setArrayIndex(next_universe_array_index++);
						settings.setValue("universe_id", this_inner_pair.first);
						settings.setValue("name", this_inner_pair.second);
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
