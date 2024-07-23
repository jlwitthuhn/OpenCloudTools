#include "profile.h"

#include <algorithm>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include <QApplication>
#include <QPalette>
#include <QSettings>
#include <QString>
#include <QStyle>
#include <QVariant>

#include "assert.h"

static bool compare_api_key_profile(const ApiKeyProfile* const a, const ApiKeyProfile* const b)
{
	return a->get_name() < b->get_name();
}

static bool compare_universe_profile(const std::shared_ptr<const UniverseProfile> a, const std::shared_ptr<const UniverseProfile> b)
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

UniverseProfile::UniverseProfile(QObject* parent, const QString& name, const long long universe_id, const std::function<bool(const QString&)> name_available, const std::function<bool(long long)> id_available)
	: QObject{ parent }, name{ name }, universe_id{ universe_id }, name_available{ name_available }, id_available{ id_available }
{

}

bool UniverseProfile::matches_name_and_id(const UniverseProfile& other) const
{
	return name == other.name && universe_id == other.universe_id;
}

bool UniverseProfile::set_details(const QString& name_in, const long long universe_id_in)
{
	const bool name_good = name == name_in || name_available(name_in);
	const bool id_good = universe_id == universe_id_in || id_available(universe_id_in);
	if (name_good && id_good)
	{
		name = name_in;
		universe_id = universe_id_in;
		emit details_changed();
		return true;
	}
	else
	{
		return false;
	}
}

void UniverseProfile::set_save_recent_message_topics(const bool save_topics)
{
	if (save_recent_message_topics != save_topics)
	{
		save_recent_message_topics = save_topics;
		emit force_save();
	}
}

void UniverseProfile::set_save_recent_ordered_datastores(const bool save_datastores)
{
	if (save_recent_ordered_datastores != save_datastores)
	{
		save_recent_ordered_datastores = save_datastores;
		emit force_save();
	}
}

void UniverseProfile::add_hidden_datastore(const QString& datastore)
{
	hidden_datastore_set.insert(datastore);
	emit hidden_datastore_list_changed();
}

void UniverseProfile::remove_hidden_datastore(const QString& datastore)
{
	if (hidden_datastore_set.erase(datastore))
	{
		emit hidden_datastore_list_changed();
	}
}

void UniverseProfile::add_recent_ordered_datastore(const QString& datastore_name)
{
	recent_ordered_datastore_set.insert(datastore_name);
	emit recent_ordered_datastore_list_changed();
}

void UniverseProfile::remove_recent_ordered_datastore(const QString& datastore_name)
{
	if (recent_ordered_datastore_set.erase(datastore_name))
	{
		emit recent_ordered_datastore_list_changed();
	}
}

void UniverseProfile::add_recent_topic(const QString& topic)
{
	recent_topic_set.insert(topic);
	emit recent_topic_list_changed();
}

void UniverseProfile::remove_recent_topic(const QString& topic)
{
	if (recent_topic_set.erase(topic))
	{
		emit recent_topic_list_changed();
	}
}

ApiKeyProfile::ApiKeyProfile(QObject* parent, const QString& name, const QString& key, const bool production, const bool save_to_disk, const std::function<bool(const QString&)> api_key_name_available)
	: QObject{ parent }, name{ name }, key{ key }, production{ production }, save_to_disk{ save_to_disk }, api_key_name_available{ api_key_name_available }
{

}

bool ApiKeyProfile::set_details(const QString& name_in, const QString& key_in, const bool production_in, const bool save_to_disk_in)
{
	if (name == name_in || api_key_name_available(name_in))
	{
		name = name_in;
		key = key_in;
		production = production_in;
		save_to_disk = save_to_disk_in;
		emit details_changed();
		return true;
	}
	else
	{
		return false;
	}
}

std::shared_ptr<UniverseProfile> ApiKeyProfile::get_universe_profile_by_index(size_t universe_index) const
{
	return universe_list.at(universe_index);
}

std::shared_ptr<UniverseProfile> ApiKeyProfile::get_selected_universe() const
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

std::optional<size_t> ApiKeyProfile::add_universe(const QString& universe_name, long long universe_id)
{
	if (universe_name_available(universe_name) == false || universe_id_available(universe_id) == false)
	{
		return std::nullopt;
	}
	const std::function<bool(const QString&)> name_check = [this](const QString& name) -> bool {
		return universe_name_available(name);
	};
	const std::function<bool(long long)> id_check = [this](long long id) -> bool {
		return universe_id_available(id);
	};
	const std::shared_ptr<UniverseProfile> this_universe = std::make_shared<UniverseProfile>(this, universe_name, universe_id, name_check, id_check);
	connect(this_universe.get(), &UniverseProfile::force_save, this, &ApiKeyProfile::force_save);
	connect(this_universe.get(), &UniverseProfile::hidden_datastore_list_changed, this, &ApiKeyProfile::hidden_datastore_list_changed);
	connect(this_universe.get(), &UniverseProfile::recent_ordered_datastore_list_changed, this, &ApiKeyProfile::recent_ordered_datastore_list_changed);
	connect(this_universe.get(), &UniverseProfile::recent_topic_list_changed, this, &ApiKeyProfile::recent_topic_list_changed);
	connect(this_universe.get(), &UniverseProfile::details_changed, this, &ApiKeyProfile::sort_universe_list);
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
	std::shared_ptr<const UniverseProfile> selected_universe;
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

bool ApiKeyProfile::universe_name_available(const QString& universe_name) const
{
	for (const std::shared_ptr<const UniverseProfile> this_universe : universe_list)
	{
		if (this_universe->get_name() == universe_name)
		{
			return false;
		}
	}
	return true;
}

bool ApiKeyProfile::universe_id_available(const long long universe_id) const
{
	for (const std::shared_ptr<const UniverseProfile> this_universe : universe_list)
	{
		if (this_universe->get_universe_id() == universe_id)
		{
			return false;
		}
	}
	return true;
}

UserProfile& UserProfile::get()
{
	static UserProfile manager{};
	return manager;
}

ApiKeyProfile* UserProfile::get_selected_api_key()
{
	UserProfile& user_profile = get();
	if (const std::optional<ApiKeyProfile::Id> opt_id = user_profile.selected_key_id)
	{
		return user_profile.get_api_key_by_id(*opt_id);
	}
	return nullptr;
}

std::shared_ptr<UniverseProfile> UserProfile::get_selected_universe()
{
	if (ApiKeyProfile* selected_key = get_selected_api_key())
	{
		return selected_key->get_selected_universe();
	}
	else
	{
		return nullptr;
	}
}

void UserProfile::set_qt_theme(const QString& theme_name)
{
	qt_theme = theme_name;
	if (qt_theme == "_fusion_dark")
	{
		QApplication::setStyle("Windows");
		QApplication::setPalette(QApplication::style()->standardPalette());

		QApplication::setStyle("Fusion");

		QPalette main_palette;

		const char* color_background = "#323232";
		const char* color_background_alt = "#262626";
		const char* color_background_input = "#141414";
		const char* color_text = "#FFFFFF";
		const char* color_text_alt = "#DDDDDD";
		const char* color_text_disabled = "#BBBBBB";

		main_palette.setColor(QPalette::Active, QPalette::Window, color_background);
		main_palette.setColor(QPalette::Active, QPalette::WindowText, color_text);
		main_palette.setColor(QPalette::Active, QPalette::Base, color_background_input);
		main_palette.setColor(QPalette::Active, QPalette::AlternateBase, color_background_alt);
		main_palette.setColor(QPalette::Active, QPalette::ToolTipBase, "#FFFFFF");
		main_palette.setColor(QPalette::Active, QPalette::ToolTipText, "#000000");
		main_palette.setColor(QPalette::Active, QPalette::PlaceholderText, color_text);
		main_palette.setColor(QPalette::Active, QPalette::Text, color_text);
		main_palette.setColor(QPalette::Active, QPalette::Button, color_background);
		main_palette.setColor(QPalette::Active, QPalette::ButtonText, color_text);
		main_palette.setColor(QPalette::Active, QPalette::BrightText, color_text_alt);

		main_palette.setColor(QPalette::Inactive, QPalette::Window, color_background);
		main_palette.setColor(QPalette::Inactive, QPalette::WindowText, color_text);
		main_palette.setColor(QPalette::Inactive, QPalette::Base, color_background_input);
		main_palette.setColor(QPalette::Inactive, QPalette::AlternateBase, color_background_alt);
		main_palette.setColor(QPalette::Inactive, QPalette::ToolTipBase, "#FFFFFF");
		main_palette.setColor(QPalette::Inactive, QPalette::ToolTipText, "#000000");
		main_palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, color_text);
		main_palette.setColor(QPalette::Inactive, QPalette::Text, color_text);
		main_palette.setColor(QPalette::Inactive, QPalette::Button, color_background);
		main_palette.setColor(QPalette::Inactive, QPalette::ButtonText, color_text);
		main_palette.setColor(QPalette::Inactive, QPalette::BrightText, color_text_alt);

		main_palette.setColor(QPalette::Disabled, QPalette::Window, color_background);
		main_palette.setColor(QPalette::Disabled, QPalette::WindowText, color_text_disabled);
		main_palette.setColor(QPalette::Disabled, QPalette::Base, color_background);
		main_palette.setColor(QPalette::Disabled, QPalette::AlternateBase, color_background_alt);
		main_palette.setColor(QPalette::Disabled, QPalette::ToolTipBase, "#FFFFFF");
		main_palette.setColor(QPalette::Disabled, QPalette::ToolTipText, "#000000");
		main_palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, color_text_disabled);
		main_palette.setColor(QPalette::Disabled, QPalette::Text, color_text_disabled);
		main_palette.setColor(QPalette::Disabled, QPalette::Button, color_background);
		main_palette.setColor(QPalette::Disabled, QPalette::ButtonText, color_text_disabled);
		main_palette.setColor(QPalette::Disabled, QPalette::BrightText, color_text_alt);

		QPalette menu_palette{ main_palette };

		menu_palette.setColor(QPalette::Active, QPalette::Base, color_background_alt);
		menu_palette.setColor(QPalette::Inactive, QPalette::Base, color_background_alt);
		menu_palette.setColor(QPalette::Disabled, QPalette::Base, color_background_alt);

		QPalette menu_bar_palette{ main_palette };

		menu_bar_palette.setColor(QPalette::Active, QPalette::Window, color_background_alt);
		menu_bar_palette.setColor(QPalette::Inactive, QPalette::Window, color_background_alt);
		menu_bar_palette.setColor(QPalette::Disabled, QPalette::Window, color_background_alt);

		QApplication::setPalette(main_palette);
		QApplication::setPalette(menu_palette, "QMenu");
		QApplication::setPalette(menu_bar_palette, "QMenuBar");
	}
	else
	{
		QApplication::setStyle(qt_theme);
		QApplication::setPalette(QApplication::style()->standardPalette());
	}
	emit qt_theme_changed();
	save_to_disk();
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

void UserProfile::set_show_datastore_name_filter(const bool show_filter)
{
	if (show_datastore_name_filter != show_filter)
	{
		show_datastore_name_filter = show_filter;
		emit show_datastore_filter_changed();
		save_to_disk();
	}
}

std::vector<ApiKeyProfile*> UserProfile::get_api_key_list() const
{
	std::vector<ApiKeyProfile*> result;

	for (const auto& this_key_pair : api_keys)
	{
		result.push_back(this_key_pair.second);
	}
	std::sort(result.begin(), result.end(), compare_api_key_profile);

	return result;
}

ApiKeyProfile* UserProfile::get_api_key_by_id(const ApiKeyProfile::Id key_id) const
{
	const auto map_iter = api_keys.find(key_id);
	if (map_iter != api_keys.end())
	{
		return map_iter->second;
	}
	return nullptr;
}

std::optional<ApiKeyProfile::Id> UserProfile::add_api_key(const QString& name, const QString& key, bool production, bool save_key_to_disk)
{
	if (profile_name_available(name))
	{
		std::function<bool(const QString&)> api_key_name_available = [this](const QString& name) -> bool {
			return profile_name_available(name);
		};
		ApiKeyProfile* this_profile = new ApiKeyProfile{ this, name, key, production, save_key_to_disk, api_key_name_available };
		connect(this_profile, &ApiKeyProfile::force_save, this, &UserProfile::save_to_disk);
		connect(this_profile, &ApiKeyProfile::details_changed, this, &UserProfile::api_key_details_changed);
		connect(this_profile, &ApiKeyProfile::hidden_datastore_list_changed, this, &UserProfile::hidden_datastore_list_changed);
		connect(this_profile, &ApiKeyProfile::recent_ordered_datastore_list_changed, this, &UserProfile::recent_ordered_datastore_list_changed);
		connect(this_profile, &ApiKeyProfile::recent_topic_list_changed, this, &UserProfile::recent_topic_list_changed);
		connect(this_profile, &ApiKeyProfile::universe_list_changed, this, &UserProfile::universe_list_changed);

		OCTASSERT(api_keys.count(this_profile->get_id()) == 0);
		api_keys[this_profile->get_id()] = this_profile;

		emit api_key_list_changed(this_profile->get_id());
		return this_profile->get_id();
	}
	return std::nullopt;
}

void UserProfile::delete_api_key(const ApiKeyProfile::Id id)
{
	auto selected_key_iter = api_keys.find(id);
	if (selected_key_iter == api_keys.end())
	{
		OCTASSERT(false);
		return;
	}
	delete selected_key_iter->second;
	api_keys.erase(selected_key_iter);

	if (selected_key_id && *selected_key_id == id)
	{
		api_key_list_changed(std::nullopt);
	}
	else
	{
		api_key_list_changed(selected_key_id);
	}
}

void UserProfile::select_api_key(const std::optional<ApiKeyProfile::Id> id)
{
	if (id && api_keys.count(*id))
	{
		selected_key_id = id;
	}
	else
	{
		selected_key_id = std::nullopt;
	}
	emit selected_api_key_changed();
}

void UserProfile::api_key_details_changed()
{
	emit api_key_list_changed(selected_key_id);
}

bool UserProfile::profile_name_available(const QString& name) const
{
	for (const auto& this_pair : api_keys)
	{
		if (name == this_pair.second->get_name())
		{
			return false;
		}
	}
	return true;
}

void UserProfile::load_from_disk()
{
	std::lock_guard<LockableBool> load_guard{ load_flag };

	QSettings settings;

	settings.beginGroup("prefs");
	if (settings.value("qt_theme").isValid())
	{
		qt_theme = settings.value("qt_theme").toString();
	}
	if (settings.value("autoclose_progress_window").isValid())
	{
		autoclose_progress_window = settings.value("autoclose_progress_window").toBool();
	}
	if (settings.value("less_verbose_bulk_operations").isValid())
	{
		less_verbose_bulk_operations = settings.value("less_verbose_bulk_operations").toBool();
	}
	if (settings.value("show_datastore_name_filter").isValid())
	{
		show_datastore_name_filter = settings.value("show_datastore_name_filter").toBool();
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
				std::optional<ApiKeyProfile::Id> opt_key_id = add_api_key(name, key, production, true);

				if (opt_key_id)
				{
					const int universe_list_size = settings.beginReadArray("universe_ids");
					for (int j = 0; j < universe_list_size; j++)
					{
						settings.setArrayIndex(j);
						const long long this_universe_id = settings.value("universe_id").toLongLong();

						const QVariant maybe_universe_name = settings.value("name");
						const QString universe_name = maybe_universe_name.isNull() ? "Unnamed" : maybe_universe_name.toString();

						const QVariant maybe_save_topics = settings.value("save_recent_message_topics");
						const bool save_recent_message_topics = maybe_save_topics.isNull() ? true : maybe_save_topics.toBool();

						const QVariant maybe_save_datastores = settings.value("save_recent_ordered_datastores");
						const bool save_recent_ordered_datastores = maybe_save_datastores.isNull() ? true : maybe_save_datastores.toBool();

						if (ApiKeyProfile* const this_api_key = get_api_key_by_id(*opt_key_id))
						{
							if (const std::optional<size_t> new_universe_index = this_api_key->add_universe(universe_name, this_universe_id))
							{
								if (const std::shared_ptr<UniverseProfile> this_universe = this_api_key->get_universe_profile_by_index(*new_universe_index))
								{
									this_universe->set_save_recent_message_topics(save_recent_message_topics);
									this_universe->set_save_recent_ordered_datastores(save_recent_ordered_datastores);

									const int hidden_list_size = settings.beginReadArray("hidden_datastores");
									{
										for (int k = 0; k < hidden_list_size; k++)
										{
											settings.setArrayIndex(k);
											const QString datastore_name = settings.value("name").toString();
											this_universe->add_hidden_datastore(datastore_name);
										}
									}
									settings.endArray();

									const int topic_list_size = settings.beginReadArray("message_topics");
									{
										for (int k = 0; k < topic_list_size; k++)
										{
											settings.setArrayIndex(k);
											const QString topic_name = settings.value("name").toString();
											this_universe->add_recent_topic(topic_name);
										}
									}
									settings.endArray();

									const int ordered_datastore_list_size = settings.beginReadArray("ordered_datastores");
									{
										for (int k = 0; k < ordered_datastore_list_size; k++)
										{
											settings.setArrayIndex(k);
											const QString datastore_name = settings.value("name").toString();
											this_universe->add_recent_ordered_datastore(datastore_name);
										}
									}
									settings.endArray();
								}
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

	set_qt_theme(qt_theme);

	emit api_key_list_changed(std::nullopt);
}

void UserProfile::save_to_disk()
{
	if (load_flag)
	{
		return;
	}

	QSettings settings;

	settings.beginGroup("prefs");
	settings.setValue("qt_theme", qt_theme);
	settings.setValue("autoclose_progress_window", autoclose_progress_window);
	settings.setValue("less_verbose_bulk_operations", less_verbose_bulk_operations);
	settings.setValue("show_datastore_name_filter", show_datastore_name_filter);
	settings.endGroup();

	settings.beginGroup("api_keys");

	settings.setValue("version", static_cast<unsigned int>(1));

	settings.beginGroup("keys");
	settings.setValue("version", static_cast<unsigned int>(1));
	settings.beginWriteArray("list");
	{
		int next_array_index = 0;
		for (const ApiKeyProfile* this_key : get_api_key_list())
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
					for (const std::shared_ptr<const UniverseProfile> this_universe_profile : this_key->get_universe_list())
					{
						settings.setArrayIndex(next_universe_array_index++);
						settings.setValue("name", this_universe_profile->get_name());
						settings.setValue("universe_id", this_universe_profile->get_universe_id());
						settings.setValue("save_recent_message_topics", this_universe_profile->get_save_recent_message_topics());
						settings.setValue("save_recent_ordered_datastores", this_universe_profile->get_save_recent_ordered_datastores());

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

						settings.beginWriteArray("message_topics");
						{
							int next_topic_index = 0;
							for (const QString& this_topic : this_universe_profile->get_recent_topic_set())
							{
								settings.setArrayIndex(next_topic_index++);
								settings.setValue("name", this_topic);
							}
						}
						settings.endArray();

						settings.beginWriteArray("ordered_datastores");
						{
							int next_datastore_index = 0;
							for (const QString& this_datastore_name : this_universe_profile->get_recent_ordered_datastore_set())
							{
								settings.setArrayIndex(next_datastore_index++);
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
	if (QApplication::style())
	{
#ifdef QT5_COMPAT
		qt_theme = QApplication::style()->objectName();
#else
		qt_theme = QApplication::style()->name();
#endif
	}
	load_from_disk();
	connect(this, &UserProfile::api_key_list_changed, this, &UserProfile::save_to_disk);
	connect(this, &UserProfile::hidden_datastore_list_changed, this, &UserProfile::save_to_disk);
	connect(this, &UserProfile::recent_ordered_datastore_list_changed, this, &UserProfile::save_to_disk);
	connect(this, &UserProfile::recent_topic_list_changed, this, &UserProfile::save_to_disk);
	connect(this, &UserProfile::universe_list_changed, this, &UserProfile::save_to_disk);
}
