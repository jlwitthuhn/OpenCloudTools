#include "profile.h"

#include <algorithm>
#include <memory>
#include <set>
#include <vector>

#include <QPointer>
#include <QSettings>
#include <QString>
#include <QVariant>

static bool compare_api_key_profile(const ApiKeyProfile* const a, const ApiKeyProfile* const b)
{
	return a->get_name() < b->get_name();
}

static bool compare_universe_profile(const UniverseProfile* const a, const UniverseProfile* const b)
{
	if (a->get_name() < b->get_name())
	{
		return true;
	}
	else if (b->get_name() < a->get_name())
	{
		return false;
	}

	if (a->get_universe_id() < b->get_universe_id())
	{
		return true;
	}
	else if (b->get_universe_id() < a->get_universe_id())
	{
		return false;
	}
	return false;
}

UniverseProfile::UniverseProfile(QObject* parent, const QString& name, const long long universe_id, const std::function<bool(const QString&, long long)>& name_and_id_available)
	: QObject{ parent }, name{ name }, universe_id{ universe_id }, name_and_id_available{ name_and_id_available }
{

}

bool UniverseProfile::matches_name_and_id(const UniverseProfile& other) const
{
	return name == other.name && universe_id == other.universe_id;
}

bool UniverseProfile::set_details(const QString& name, long long universe_id)
{
	if (name_and_id_available(name, universe_id))
	{
		this->name = name;
		this->universe_id = universe_id;
		emit details_changed();
		return true;
	}
	else
	{
		return false;
	}
}

void UniverseProfile::add_hidden_datastore(const QString& datastore)
{
	hidden_datastore_set.insert(datastore);
	emit hidden_datastore_list_changed();
}

void UniverseProfile::remove_hidden_datastore(const QString& datastore) {
	hidden_datastore_set.erase(datastore);
	emit hidden_datastore_list_changed();
}

ApiKeyProfile::ApiKeyProfile(QObject* parent, const QString& name, const QString& key, const bool production, const bool save_to_disk, const std::function<bool(const QString&)> api_key_name_available)
	: QObject{ parent }, name{ name }, key{ key }, production{ production }, save_to_disk{ save_to_disk }, api_key_name_available{ api_key_name_available }
{

}

bool ApiKeyProfile::set_details(const QString& name, const QString& key, const bool production, const bool save_to_disk)
{
	if (api_key_name_available(name))
	{
		this->name = name;
		this->key = key;
		this->production = production;
		this->save_to_disk = save_to_disk;
		emit details_changed();
		return true;
	}
	else
	{
		return false;
	}
}

UniverseProfile* ApiKeyProfile::get_universe_profile_by_index(size_t universe_index) const
{
	return universe_list.at(universe_index);
}

UniverseProfile* ApiKeyProfile::get_selected_universe() const
{
	if (selected_universe_index && *selected_universe_index < universe_list.size())
	{
		return universe_list.at(*selected_universe_index);
	}
	else
	{
		return nullptr;
	}
}

std::optional<size_t> ApiKeyProfile::add_universe(const QString& name, long long universe_id)
{
	for (const UniverseProfile* existing_profile : universe_list)
	{
		if (existing_profile->get_name() == name && existing_profile->get_universe_id() == universe_id)
		{
			// Name + ID should uniquely identify a given slot
			return std::nullopt;
		}
	}
	const std::function<bool(const QString&, long long)> name_id_check = [this](const QString& name, long long id) -> bool {
		return universe_name_and_id_available(name, id);
	};
	UniverseProfile* this_universe = new UniverseProfile{ this, name, universe_id, name_id_check };
	connect(this_universe, &UniverseProfile::details_changed, this, &ApiKeyProfile::sort_universe_list);
	connect(this_universe, &UniverseProfile::hidden_datastore_list_changed, this, &ApiKeyProfile::hidden_datastore_list_changed);
	universe_list.push_back(this_universe);
	std::sort(universe_list.begin(), universe_list.end(), compare_universe_profile);
	for (size_t this_index = 0; this_index < universe_list.size(); this_index++)
	{
		if (universe_list.at(this_index)->matches_name_and_id(*this_universe))
		{
			emit universe_list_changed(this_index);
			return this_index;
		}
	}
	// Should never happen
	emit universe_list_changed(std::nullopt);
	return std::nullopt;
}

void ApiKeyProfile::remove_universe(const size_t universe_index)
{
	if (universe_index < universe_list.size())
	{
		universe_list.erase(universe_list.begin() + universe_index);
	}
}

void ApiKeyProfile::select_universe(const std::optional<size_t> universe_index)
{
	if (universe_index && *universe_index < universe_list.size())
	{
		selected_universe_index = universe_index;
	}
	else
	{
		selected_universe_index = std::nullopt;
	}
}

void ApiKeyProfile::remove_selected_universe()
{
	if (selected_universe_index)
	{
		remove_universe(*selected_universe_index);
	}
	emit universe_list_changed(std::nullopt);
}

void ApiKeyProfile::sort_universe_list()
{
	UniverseProfile* selected_universe = nullptr;
	if (selected_universe_index)
	{
		selected_universe = universe_list.at(*selected_universe_index);
	}
	std::sort(universe_list.begin(), universe_list.end(), compare_universe_profile);
	if (selected_universe)
	{
		for (size_t i = 0; i < universe_list.size(); i++)
		{
			if (selected_universe == universe_list.at(i))
			{
				selected_universe_index = i;
				break;
			}
		}
	}
	emit universe_list_changed(selected_universe_index);
}

bool ApiKeyProfile::universe_name_and_id_available(const QString& name, const long long universe_id)
{
	for (const UniverseProfile* const this_universe : universe_list)
	{
		if (this_universe->get_name() == name && this_universe->get_universe_id() == universe_id)
		{
			return false;
		}
	}
	return true;
}

std::unique_ptr<UserProfile>& UserProfile::get()
{
	static std::unique_ptr<UserProfile> manager = std::unique_ptr<UserProfile>(new UserProfile());
	return manager;
}

ApiKeyProfile* UserProfile::get_selected_api_key(UserProfile* user_profile)
{
	if (user_profile == nullptr && get())
	{
		user_profile = get().get();
	}
	if (user_profile != nullptr)
	{
		if (const std::optional<size_t> opt_index = user_profile->selected_key_index)
		{
			return user_profile->get_api_key_by_index(*opt_index);
		}
	}
	return nullptr;
}

UniverseProfile* UserProfile::get_selected_universe(UserProfile* const user_profile)
{
	if (ApiKeyProfile* selected_key = get_selected_api_key(user_profile))
	{
		return selected_key->get_selected_universe();
	}
	else
	{
		return nullptr;
	}
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

ApiKeyProfile* UserProfile::get_api_key_by_index(const size_t key_index)
{
	if (key_index < api_key_list.size())
	{
		return api_key_list.at(key_index);
	}
	else
	{
		return nullptr;
	}
}

std::optional<size_t> UserProfile::add_api_key(const QString& name, const QString& key, bool production, bool save_key_to_disk)
{
	if (profile_name_available(name))
	{
		std::function<bool(const QString&)> api_key_name_available = [this](const QString& name) -> bool {
			return profile_name_available(name);
		};
		ApiKeyProfile* this_profile = new ApiKeyProfile{ this, name, key, production, save_key_to_disk, api_key_name_available };
		connect(this_profile, &ApiKeyProfile::details_changed, this, &UserProfile::sort_api_key_profiles);
		connect(this_profile, &ApiKeyProfile::hidden_datastore_list_changed, this, &UserProfile::hidden_datastore_list_changed);
		connect(this_profile, &ApiKeyProfile::universe_list_changed, this, &UserProfile::universe_list_changed);
		api_key_list.push_back(this_profile);
		std::sort(api_key_list.begin(), api_key_list.end(), compare_api_key_profile);
		for (size_t this_index = 0; this_index < api_key_list.size(); this_index++)
		{
			if (api_key_list.at(this_index)->get_name() == name)
			{
				emit api_key_list_changed();
				return this_index;
			}
		}
	}
	return std::nullopt;
}

void UserProfile::delete_api_key(const size_t index)
{
	if (index < api_key_list.size())
	{
		api_key_list.erase(api_key_list.begin() + index);
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
}

bool UserProfile::profile_name_available(const QString& name) const
{
	for (const ApiKeyProfile* this_profile : api_key_list)
	{
		if (name == this_profile->get_name())
		{
			return false;
		}
	}
	return true;
}

void UserProfile::sort_api_key_profiles()
{
	std::sort(api_key_list.begin(), api_key_list.end(), compare_api_key_profile);
	emit api_key_list_changed();
}

void UserProfile::load_from_disk()
{
	QSettings settings;

	settings.beginGroup("prefs");
	if (settings.value("autoclose_progress_window").isValid())
	{
		autoclose_progress_window = settings.value("autoclose_progress_window").toBool();
	}
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
				std::optional<size_t> opt_key_index = add_api_key(name, key, production, true);

				if (opt_key_index)
				{
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

						if (ApiKeyProfile* const this_api_key = get_api_key_by_index(*opt_key_index))
						{
							if (const std::optional<size_t> new_universe_index = this_api_key->add_universe(name, this_universe_id))
							{
								const int hidden_list_size = settings.beginReadArray("hidden_datastores");
								for (int k = 0; k < hidden_list_size; k++)
								{
									settings.setArrayIndex(k);
									const QString datastore_name = settings.value("name").toString();
									if (UniverseProfile* const this_universe = this_api_key->get_universe_profile_by_index(*new_universe_index))
									{
										this_universe->add_hidden_datastore(datastore_name);
									}
								}
								settings.endArray();
							}
						}
					}
					settings.endArray();
				}
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
		for (const ApiKeyProfile* this_key : api_key_list)
		{
			if (this_key->get_save_to_disk())
			{
				settings.setArrayIndex(next_array_index++);
				settings.setValue("name", this_key->get_name());
				settings.setValue("key", this_key->get_key());
				settings.setValue("production", this_key->get_production());

				settings.beginWriteArray("universe_ids");
				{
					int next_universe_array_index = 0;
					for (const UniverseProfile* this_universe_profile : this_key->get_universe_list())
					{
						settings.setArrayIndex(next_universe_array_index++);
						settings.setValue("name", this_universe_profile->get_name());
						settings.setValue("universe_id", this_universe_profile->get_universe_id());
						settings.beginWriteArray("hidden_datastores");
						{
							int next_hidden_array_index = 0;
							for (const QString& this_datastore_name : this_universe_profile->get_hidden_datastore_set())
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
	connect(this, &UserProfile::hidden_datastore_list_changed, this, &UserProfile::save_to_disk);
	connect(this, &UserProfile::universe_list_changed, this, &UserProfile::save_to_disk);
}