#include "profile.h"

#include <algorithm>
#include <memory>
#include <set>
#include <vector>

#include <QSettings>
#include <QString>
#include <QVariant>

static bool sort_api_key_profile(const ApiKeyProfile& a, const ApiKeyProfile& b)
{
	return a.name() < b.name();
}

UniverseProfile::UniverseProfile(const QString& name, const long long universe_id) : _name{ name }, _universe_id{ universe_id }
{

}

bool UniverseProfile::matches_name_and_id(const UniverseProfile& other) const
{
	return _name == other._name && _universe_id == other._universe_id;
}

ApiKeyProfile::ApiKeyProfile(const QString& name, const QString& key, const bool production, const bool save_to_disk) : _name{ name }, _key{ key }, _production{ production }, _save_to_disk{ save_to_disk }
{

}

std::optional<size_t> ApiKeyProfile::add_universe(const UniverseProfile& universe_profile)
{
	for (const UniverseProfile& existing_profile : _universes)
	{
		if (universe_profile.matches_name_and_id(existing_profile))
		{
			// Name + ID should uniquely identify a given slot
			return std::nullopt;
		}
	}
	const size_t next_index = _universes.size();
	_universes.push_back(universe_profile);
	return next_index;
}

void ApiKeyProfile::remove_universe(const size_t universe_index)
{
	if (universe_index < _universes.size())
	{
		_universes.erase(_universes.begin() + universe_index);
	}
}

bool ApiKeyProfile::update_universe_details(const size_t universe_index, const QString& name, long long universe_id)
{
	if (universe_index < _universes.size())
	{
		_universes.at(universe_index).set_name(name);
		_universes.at(universe_index).set_universe_id(universe_id);
		return true;
	}
	return false;
}
void ApiKeyProfile::add_hidden_datastore(const size_t universe_index, const QString& datastore_name)
{
	if (universe_index < _universes.size())
	{
		_universes.at(universe_index).add_hidden_datastore(datastore_name);
	}
}
void ApiKeyProfile::remove_hidden_datastore(const size_t universe_index, const QString& datastore_name)
{
	if (universe_index < _universes.size())
	{
		_universes.at(universe_index).remove_hidden_datastore(datastore_name);
	}
}

void ApiKeyProfile::sort_universes()
{
	std::sort(_universes.begin(), _universes.end(), [](const UniverseProfile& a, const UniverseProfile& b) -> bool {
		if (a.name() < b.name())
		{
			return true;
		}
		else if (b.name() < a.name())
		{
			return false;
		}

		if (a.universe_id() < b.universe_id())
		{
			return true;
		}
		else if (b.universe_id() < a.universe_id())
		{
			return false;
		}
		return false;
		});
}

std::unique_ptr<UserProfile>& UserProfile::get()
{
	static std::unique_ptr<UserProfile> manager = std::unique_ptr<UserProfile>(new UserProfile());
	return manager;
}

void UserProfile::set_autoclose_progress_window(const bool autoclose)
{
	if (autoclose_progress_window != autoclose)
	{
		autoclose_progress_window = autoclose;
		emit autoclose_changed();
		save_to_disk();
	}
}

void UserProfile::set_less_verbose_bulk_operations(const bool less_verbose)
{
	if (less_verbose_bulk_operations != less_verbose)
	{
		less_verbose_bulk_operations = less_verbose;
		save_to_disk();
	}
}

std::optional<ApiKeyProfile> UserProfile::get_api_key(const size_t key_index) const
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

std::optional<size_t> UserProfile::add_api_key(const ApiKeyProfile& details, const bool emit_signal)
{
	if (universe_name_in_use( details.name() ))
	{
		return std::nullopt;
	}
	else
	{
		const size_t this_index = api_keys.size();
		api_keys.push_back(details);
		std::sort(api_keys.begin(), api_keys.end(), sort_api_key_profile);
		if (emit_signal)
		{
			emit api_key_list_changed();
		}
		return this_index;
	}
}

void UserProfile::update_api_key(const size_t index, const ApiKeyProfile& details)
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
		std::sort(api_keys.begin(), api_keys.end(), sort_api_key_profile);
		emit api_key_list_changed();
	}
}

void UserProfile::delete_api_key(const size_t index)
{
	if (index < api_keys.size())
	{
		api_keys.erase(api_keys.begin() + index);
		emit api_key_list_changed();
	}
}

void UserProfile::select_api_key(const std::optional<size_t> index)
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

std::optional<ApiKeyProfile> UserProfile::get_selected_profile() const
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

std::optional<size_t> UserProfile::selected_profile_add_universe(const UniverseProfile& universe_profile)
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

void UserProfile::select_universe(const std::optional<size_t> universe_index)
{
	selected_universe_index = universe_index;
}

void UserProfile::remove_selected_universe()
{
	if (selected_key_index && *selected_key_index < api_keys.size() && selected_universe_index)
	{
		ApiKeyProfile& this_profile = api_keys.at(*selected_key_index);
		this_profile.remove_universe(*selected_universe_index);
		emit universe_list_changed(std::nullopt);
	}
}

bool UserProfile::update_selected_universe(const QString& name, const long long universe_id)
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

std::optional<UniverseProfile> UserProfile::get_selected_universe() const
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

void UserProfile::add_hidden_datastore(const QString& datastore_name)
{
	if (selected_key_index && *selected_key_index < api_keys.size())
	{
		ApiKeyProfile& this_profile = api_keys.at(*selected_key_index);
		if (selected_universe_index && *selected_universe_index < this_profile.universes().size())
		{
			this_profile.add_hidden_datastore(*selected_universe_index, datastore_name);
			emit hidden_datastores_changed();
		}
	}
}

void UserProfile::remove_hidden_datastore(const QString& datastore_name)
{
	if (selected_key_index && *selected_key_index < api_keys.size())
	{
		ApiKeyProfile& this_profile = api_keys.at(*selected_key_index);
		if (selected_universe_index && *selected_universe_index < this_profile.universes().size())
		{
			this_profile.remove_hidden_datastore(*selected_universe_index, datastore_name);
			emit hidden_datastores_changed();
		}
	}
}

bool UserProfile::universe_name_in_use(const QString& name) const
{
	bool result = false;
	for (const ApiKeyProfile& this_profile : api_keys)
	{
		result = result || (name == this_profile.name());
	}
	return result;
}

void UserProfile::sort_universes()
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

void UserProfile::load_from_disk()
{
	QSettings settings;

	settings.beginGroup("prefs");
	autoclose_progress_window = settings.value("autoclose_progress_window").toBool();
	if (settings.value("less_verbose_bulk_operations").isValid())
	{
		less_verbose_bulk_operations = settings.value("less_verbose_bulk_operations").toBool();
	}
	settings.endGroup();

	settings.beginGroup("api_keys");

	QVariant settings_version = settings.value("version");
	if (settings_version.toUInt() == 1)
	{
		settings.beginGroup("keys");
		const QVariant keys_version = settings.value("version");

		const int key_list_size = settings.beginReadArray("list");
		for (int i = 0; i < key_list_size; i++)
		{
			settings.setArrayIndex(i);
			const QString name = settings.value("name").toString();
			const QString key = settings.value("key").toString();
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

					UniverseProfile new_profile{ name, this_universe_id };

					const int hidden_list_size = settings.beginReadArray("hidden_datastores");
					for (int k = 0; k < hidden_list_size; k++)
					{
						settings.setArrayIndex(k);
						const QString datastore_name = settings.value("name").toString();
						new_profile.add_hidden_datastore(datastore_name);
					}
					settings.endArray();

					loaded_key.add_universe(new_profile);
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

void UserProfile::save_to_disk()
{
	QSettings settings;

	settings.beginGroup("prefs");
	settings.setValue("autoclose_progress_window", autoclose_progress_window);
	settings.setValue("less_verbose_bulk_operations", less_verbose_bulk_operations);
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
						settings.beginWriteArray("hidden_datastores");
						{
							int next_hidden_array_index = 0;
							for (const QString& this_datastore_name : this_universe_profile.hidden_datastores())
							{
								settings.setArrayIndex(next_hidden_array_index++);
								settings.setValue("name", this_datastore_name);
							}
						}
						settings.endArray();
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

UserProfile::UserProfile(QObject* parent) : QObject{ parent }
{
	load_from_disk();
	connect(this, &UserProfile::api_key_list_changed, this, &UserProfile::save_to_disk);
	connect(this, &UserProfile::hidden_datastores_changed, this, &UserProfile::save_to_disk);
	connect(this, &UserProfile::universe_list_changed, this, &UserProfile::save_to_disk);
}
