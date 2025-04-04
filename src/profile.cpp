#include "profile.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <mutex>
#include <set>
#include <utility>
#include <vector>

#include <QtGlobal>
#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QSettings>
#include <QString>
#include <QStyle>
#include <QVariant>

#include "assert.h"

static bool compare_api_key_profile(const std::shared_ptr<const ApiKeyProfile>& a, const std::shared_ptr<const ApiKeyProfile>& b)
{
	return a->get_name() < b->get_name();
}

static bool compare_universe_profile(const std::shared_ptr<const UniverseProfile>& a, const std::shared_ptr<const UniverseProfile>& b)
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

UniverseProfile::UniverseProfile(QObject* parent, const QString& name, const long long universe_id, const std::function<bool(const QString&)>& is_name_available, const std::function<bool(long long)>& is_universe_id_available)
	: QObject{ parent }, name{ name }, universe_id{ universe_id }, is_name_available{ is_name_available }, is_universe_id_available{ is_universe_id_available }
{

}

bool UniverseProfile::matches_name_and_id(const UniverseProfile& other) const
{
	return name == other.name && universe_id == other.universe_id;
}

QString UniverseProfile::get_display_name() const
{
	return QString{ "%1 [%2]" }.arg(get_name()).arg(get_universe_id());
}

bool UniverseProfile::set_details(const QString& name_in, const long long universe_id_in)
{
	const bool name_good = name == name_in || is_name_available(name_in);
	const bool id_good = universe_id == universe_id_in || is_universe_id_available(universe_id_in);
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

void UniverseProfile::set_save_recent_mem_sorted_maps(const bool save_maps)
{
	if (save_recent_mem_sorted_maps != save_maps)
	{
		save_recent_mem_sorted_maps = save_maps;
		emit force_save();
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

void UniverseProfile::set_show_hidden_standard_datastores(const bool show_datastores)
{
	if (show_hidden_standard_datastores != show_datastores)
	{
		show_hidden_standard_datastores = show_datastores;
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

void UniverseProfile::add_hidden_operation(const QString& op)
{
	hidden_operations_set.insert(op);
	emit hidden_operations_changed();
}

void UniverseProfile::set_hidden_operations_set(const std::set<QString>& ops)
{
	hidden_operations_set = ops;
	emit hidden_operations_changed();
}

void UniverseProfile::add_recent_mem_sorted_map(const QString& map_name)
{
	recent_mem_sorted_map_set.insert(map_name);
	emit recent_mem_sorted_map_list_changed();
}

void UniverseProfile::remove_recent_mem_sorted_map(const QString& map_name)
{
	if (recent_mem_sorted_map_set.erase(map_name))
	{
		emit recent_mem_sorted_map_list_changed();
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

ApiKeyProfile::ApiKeyProfile(QObject* parent, const QString& name, const QString& key, const bool production, const bool save_to_disk, const std::function<bool(const QString&)>& api_key_name_available)
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

std::vector<std::shared_ptr<UniverseProfile>> ApiKeyProfile::get_universe_list() const
{
	std::vector<std::shared_ptr<UniverseProfile>> result;

	for (const auto& this_universe_pair : universes)
	{
		result.push_back(this_universe_pair.second);
	}
	std::sort(result.begin(), result.end(), compare_universe_profile);

	return result;
}

std::shared_ptr<UniverseProfile> ApiKeyProfile::get_universe_profile_by_id(const UniverseProfile::Id universe_id) const
{
	auto universe_iter = universes.find(universe_id);
	if (universe_iter == universes.end())
	{
		return nullptr;
	}
	return universe_iter->second;
}

std::optional<UniverseProfile::Id> ApiKeyProfile::add_universe(const QString& universe_name, long long universe_id)
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
	const std::shared_ptr<UniverseProfile> this_universe = std::make_shared<UniverseProfile>(nullptr, universe_name, universe_id, name_check, id_check);
	const std::weak_ptr<UniverseProfile> weak_universe = this_universe;
	connect(this_universe.get(), &UniverseProfile::details_changed, this, [this, weak_universe] {
		if (auto shared_universe = weak_universe.lock())
		{
			emit universe_details_changed(shared_universe->get_id());
		}
	});
	connect(this_universe.get(), &UniverseProfile::force_save, this, &ApiKeyProfile::force_save);
	connect(this_universe.get(), &UniverseProfile::hidden_datastore_list_changed, this, &ApiKeyProfile::hidden_datastore_list_changed);
	connect(this_universe.get(), &UniverseProfile::hidden_operations_changed, this, &ApiKeyProfile::universe_hidden_operations_changed);
	connect(this_universe.get(), &UniverseProfile::recent_mem_sorted_map_list_changed, this, &ApiKeyProfile::recent_mem_sorted_map_list_changed);
	connect(this_universe.get(), &UniverseProfile::recent_ordered_datastore_list_changed, this, &ApiKeyProfile::recent_ordered_datastore_list_changed);
	connect(this_universe.get(), &UniverseProfile::recent_topic_list_changed, this, &ApiKeyProfile::recent_topic_list_changed);

	OCTASSERT(universes.count(this_universe->get_id()) == 0);
	universes[this_universe->get_id()] = this_universe;

	emit universe_list_changed(this_universe->get_id());
	return this_universe->get_id();
}

void ApiKeyProfile::delete_universe(const UniverseProfile::Id universe_id)
{
	auto selected_universe_iter = universes.find(universe_id);
	if (selected_universe_iter == universes.end())
	{
		OCTASSERT(false);
		return;
	}
	universes.erase(selected_universe_iter);

	emit universe_list_changed(std::nullopt);
}

bool ApiKeyProfile::universe_name_available(const QString& universe_name) const
{
	for (const auto& this_pair : universes)
	{
		if (this_pair.second->get_name() == universe_name)
		{
			return false;
		}
	}
	return true;
}

bool ApiKeyProfile::universe_id_available(const long long universe_id) const
{
	for (const auto& this_pair : universes)
	{
		if (this_pair.second->get_universe_id() == universe_id)
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

std::shared_ptr<ApiKeyProfile> UserProfile::get_active_api_key()
{
	UserProfile& user_profile = get();
	if (const std::optional<ApiKeyProfile::Id> opt_id = user_profile.active_key_id)
	{
		return user_profile.get_api_key_by_id(*opt_id);
	}
	return nullptr;
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

		constexpr QColor color_background{ 0x32, 0x32, 0x32 };
		constexpr QColor color_background_alt{ 0x26, 0x26, 0x26 };
		constexpr QColor color_background_input{ 0x20, 0x20, 0x20 };
		constexpr QColor color_text { 0xFF, 0xFF, 0xFF };
		constexpr QColor color_text_alt{ 0xDD, 0xDD, 0xDD };
		constexpr QColor color_text_disabled{ 0xBB, 0xBB, 0xBB };

		constexpr QColor color_tooltip_bg{ 0xFF, 0xFF, 0xFF };
		constexpr QColor color_tooltip_fg{ 0x00, 0x00, 0x00 };

		main_palette.setColor(QPalette::Active, QPalette::Window, color_background);
		main_palette.setColor(QPalette::Active, QPalette::WindowText, color_text);
		main_palette.setColor(QPalette::Active, QPalette::Base, color_background_input);
		main_palette.setColor(QPalette::Active, QPalette::AlternateBase, color_background_alt);
		main_palette.setColor(QPalette::Active, QPalette::ToolTipBase, color_tooltip_bg);
		main_palette.setColor(QPalette::Active, QPalette::ToolTipText, color_tooltip_fg);
		main_palette.setColor(QPalette::Active, QPalette::PlaceholderText, color_text);
		main_palette.setColor(QPalette::Active, QPalette::Text, color_text);
		main_palette.setColor(QPalette::Active, QPalette::Button, color_background);
		main_palette.setColor(QPalette::Active, QPalette::ButtonText, color_text);
		main_palette.setColor(QPalette::Active, QPalette::BrightText, color_text_alt);

		main_palette.setColor(QPalette::Inactive, QPalette::Window, color_background);
		main_palette.setColor(QPalette::Inactive, QPalette::WindowText, color_text);
		main_palette.setColor(QPalette::Inactive, QPalette::Base, color_background_input);
		main_palette.setColor(QPalette::Inactive, QPalette::AlternateBase, color_background_alt);
		main_palette.setColor(QPalette::Inactive, QPalette::ToolTipBase, color_tooltip_bg);
		main_palette.setColor(QPalette::Inactive, QPalette::ToolTipText, color_tooltip_fg);
		main_palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, color_text);
		main_palette.setColor(QPalette::Inactive, QPalette::Text, color_text);
		main_palette.setColor(QPalette::Inactive, QPalette::Button, color_background);
		main_palette.setColor(QPalette::Inactive, QPalette::ButtonText, color_text);
		main_palette.setColor(QPalette::Inactive, QPalette::BrightText, color_text_alt);

		main_palette.setColor(QPalette::Disabled, QPalette::Window, color_background);
		main_palette.setColor(QPalette::Disabled, QPalette::WindowText, color_text_disabled);
		main_palette.setColor(QPalette::Disabled, QPalette::Base, color_background);
		main_palette.setColor(QPalette::Disabled, QPalette::AlternateBase, color_background_alt);
		main_palette.setColor(QPalette::Disabled, QPalette::ToolTipBase, color_tooltip_bg);
		main_palette.setColor(QPalette::Disabled, QPalette::ToolTipText, color_tooltip_fg);
		main_palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, color_text_disabled);
		main_palette.setColor(QPalette::Disabled, QPalette::Text, color_text_disabled);
		main_palette.setColor(QPalette::Disabled, QPalette::Button, color_background);
		main_palette.setColor(QPalette::Disabled, QPalette::ButtonText, color_text_disabled);
		main_palette.setColor(QPalette::Disabled, QPalette::BrightText, color_text_alt);

		QPalette menu_palette{ main_palette };

		menu_palette.setColor(QPalette::Active, QPalette::Base, color_background_alt);
		menu_palette.setColor(QPalette::Inactive, QPalette::Base, color_background_alt);
		menu_palette.setColor(QPalette::Disabled, QPalette::Base, color_background_alt);

		QApplication::setPalette(main_palette);
		QApplication::setPalette(menu_palette, "QMenu");
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

std::vector<std::shared_ptr<ApiKeyProfile>> UserProfile::get_api_key_list() const
{
	std::vector<std::shared_ptr<ApiKeyProfile>> result;

	for (const auto& this_key_pair : api_keys)
	{
		result.push_back(this_key_pair.second);
	}
	std::sort(result.begin(), result.end(), compare_api_key_profile);

	return result;
}

std::shared_ptr<ApiKeyProfile> UserProfile::get_api_key_by_id(const ApiKeyProfile::Id key_id) const
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
		const std::shared_ptr<ApiKeyProfile> this_profile = std::make_shared<ApiKeyProfile>(nullptr, name, key, production, save_key_to_disk, api_key_name_available);
		connect(this_profile.get(), &ApiKeyProfile::force_save, this, &UserProfile::save_to_disk);
		connect(this_profile.get(), &ApiKeyProfile::details_changed, this, &UserProfile::api_key_details_changed);
		connect(this_profile.get(), &ApiKeyProfile::hidden_datastore_list_changed, this, &UserProfile::save_to_disk);
		connect(this_profile.get(), &ApiKeyProfile::recent_mem_sorted_map_list_changed, this, &UserProfile::save_to_disk);
		connect(this_profile.get(), &ApiKeyProfile::recent_ordered_datastore_list_changed, this, &UserProfile::save_to_disk);
		connect(this_profile.get(), &ApiKeyProfile::recent_topic_list_changed, this, &UserProfile::save_to_disk);
		connect(this_profile.get(), &ApiKeyProfile::universe_hidden_operations_changed, this, &UserProfile::save_to_disk);
		connect(this_profile.get(), &ApiKeyProfile::universe_list_changed, this, &UserProfile::save_to_disk);

		OCTASSERT(api_keys.count(this_profile->get_id()) == 0);
		api_keys[this_profile->get_id()] = this_profile;

		emit api_key_list_changed();
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
	api_keys.erase(selected_key_iter);

	emit api_key_list_changed();
}

void UserProfile::activate_api_key(const std::optional<ApiKeyProfile::Id> id)
{
	if (id && api_keys.count(*id))
	{
		active_key_id = id;
	}
	else
	{
		active_key_id = std::nullopt;
	}
	emit active_api_key_changed();
}

void UserProfile::api_key_details_changed()
{
	emit api_key_list_changed();
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
		const QVariant keys_version = settings.value("version");
		if (keys_version.toUInt() == 1)
		{
			settings.beginGroup("keys");
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

							const QVariant maybe_save_mem_sorted_maps = settings.value("save_recent_mem_sorted_map");
							const bool save_recent_mem_sorted_maps = maybe_save_mem_sorted_maps.isNull() ? true : maybe_save_mem_sorted_maps.toBool();

							const QVariant maybe_save_topics = settings.value("save_recent_message_topics");
							const bool save_recent_message_topics = maybe_save_topics.isNull() ? true : maybe_save_topics.toBool();

							const QVariant maybe_save_datastores = settings.value("save_recent_ordered_datastores");
							const bool save_recent_ordered_datastores = maybe_save_datastores.isNull() ? true : maybe_save_datastores.toBool();

							const QVariant maybe_show_hidden_datastores = settings.value("show_hidden_standard_datastores");
							const bool show_hidden_datastores = maybe_show_hidden_datastores.isNull() ? false : maybe_show_hidden_datastores.toBool();

							if (const std::shared_ptr<ApiKeyProfile> this_api_key = get_api_key_by_id(*opt_key_id))
							{
								if (const std::optional<UniverseProfile::Id> new_universe_id = this_api_key->add_universe(universe_name, this_universe_id))
								{
									if (const std::shared_ptr<UniverseProfile> this_universe = this_api_key->get_universe_profile_by_id(*new_universe_id))
									{
										this_universe->set_save_recent_mem_sorted_maps(save_recent_mem_sorted_maps);
										this_universe->set_save_recent_message_topics(save_recent_message_topics);
										this_universe->set_save_recent_ordered_datastores(save_recent_ordered_datastores);
										this_universe->set_show_hidden_standard_datastores(show_hidden_datastores);

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

										const int hidden_ops_size = settings.beginReadArray("hidden_operations");
										{
											for (int k = 0; k < hidden_ops_size; k++)
											{
												settings.setArrayIndex(k);
												const QString datastore_name = settings.value("name").toString();
												this_universe->add_hidden_operation(datastore_name);
											}
										}
										settings.endArray();

										const int map_list_size = settings.beginReadArray("mem_sorted_maps");
										{
											for (int k = 0; k < map_list_size; k++)
											{
												settings.setArrayIndex(k);
												const QString map_name = settings.value("name").toString();
												this_universe->add_recent_mem_sorted_map(map_name);
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
	}

	settings.endGroup();

	set_qt_theme(qt_theme);

	emit api_key_list_changed();
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
		for (const std::shared_ptr<const ApiKeyProfile> this_key : get_api_key_list())
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
						settings.setValue("save_recent_mem_sorted_maps", this_universe_profile->get_save_recent_mem_sorted_maps());
						settings.setValue("save_recent_message_topics", this_universe_profile->get_save_recent_message_topics());
						settings.setValue("save_recent_ordered_datastores", this_universe_profile->get_save_recent_ordered_datastores());
						settings.setValue("show_hidden_standard_datastores", this_universe_profile->get_show_hidden_standard_datastores());

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

						settings.beginWriteArray("hidden_operations");
						{
							int next_datastore_index = 0;
							for (const QString& this_op : this_universe_profile->get_hidden_operations_set())
							{
								settings.setArrayIndex(next_datastore_index++);
								settings.setValue("name", this_op);
							}
						}
						settings.endArray();

						settings.beginWriteArray("mem_sorted_maps");
						{
							int next_map_index = 0;
							for (const QString& this_map : this_universe_profile->get_recent_mem_sorted_map_set())
							{
								settings.setArrayIndex(next_map_index++);
								settings.setValue("name", this_map);
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		qt_theme = QApplication::style()->name();
#else
		qt_theme = QApplication::style()->objectName();
#endif
	}
	load_from_disk();
	connect(this, &UserProfile::api_key_list_changed, this, &UserProfile::save_to_disk);
}
