#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <vector>
#include <set>

#include <QByteArray>
#include <QObject>
#include <QString>

#include "util_id.h"
#include "util_lock.h"

class UniverseProfile : public QObject
{
	Q_OBJECT
public:
	using Id = RandomId128;

	UniverseProfile(QObject* parent, const QString& name, long long universe_id, std::function<bool(const QString&)> name_available, std::function<bool(long long)> is_universe_id_available);

	bool matches_name_and_id(const UniverseProfile& other) const;

	Id get_id() const { return id; }
	QString get_name() const { return name; }
	long long get_universe_id() const { return universe_id; }
	bool get_save_recent_mem_sorted_maps() const { return save_recent_mem_sorted_maps; }
	bool get_save_recent_message_topics() const { return save_recent_message_topics; }
	bool get_save_recent_ordered_datastores() const { return save_recent_ordered_datastores; }
	bool get_show_hidden_standard_datastores() const { return show_hidden_standard_datastores; }
	const std::set<QString>& get_hidden_datastore_set() const { return hidden_datastore_set; }
	const std::set<QString>& get_recent_mem_sorted_map_set() const { return recent_mem_sorted_map_set; }
	const std::set<QString>& get_recent_ordered_datastore_set() const { return recent_ordered_datastore_set; }
	const std::set<QString>& get_recent_topic_set() const { return recent_topic_set; }

	bool set_details(const QString& name, long long universe_id);

	void set_save_recent_mem_sorted_maps(bool save_maps);
	void set_save_recent_message_topics(bool save_topics);
	void set_save_recent_ordered_datastores(bool save_datastores);
	void set_show_hidden_standard_datastores(bool show_datastores);

	void add_hidden_datastore(const QString& datastore);
	void remove_hidden_datastore(const QString& datastore);

	void add_recent_mem_sorted_map(const QString& map_name);
	void remove_recent_mem_sorted_map(const QString& map_name);

	void add_recent_ordered_datastore(const QString& datastore_name);
	void remove_recent_ordered_datastore(const QString& datastore_name);

	void add_recent_topic(const QString& topic);
	void remove_recent_topic(const QString& topic);

signals:
	void force_save();
	void details_changed();
	void hidden_datastore_list_changed();
	void recent_mem_sorted_map_list_changed();
	void recent_ordered_datastore_list_changed();
	void recent_topic_list_changed();

private:
	Id id;

	QString name;
	long long universe_id;

	bool save_recent_mem_sorted_maps = true;
	bool save_recent_message_topics = true;
	bool save_recent_ordered_datastores = true;
	bool show_hidden_standard_datastores = false;

	std::set<QString> hidden_datastore_set;
	std::set<QString> recent_mem_sorted_map_set;
	std::set<QString> recent_ordered_datastore_set;
	std::set<QString> recent_topic_set;

	std::function<bool(const QString&)> is_name_available;
	std::function<bool(long long)> is_universe_id_available;
};

class ApiKeyProfile : public QObject
{
	Q_OBJECT
public:
	using Id = RandomId128;

	ApiKeyProfile(QObject* parent, const QString& name, const QString& key, bool production, bool save_to_disk, std::function<bool(const QString&)> api_key_name_available);

	Id get_id() const { return id; }
	QString get_name() const { return name; };
	QString get_key() const { return key; };
	bool get_production() const { return production; }
	bool get_save_to_disk() const { return save_to_disk; }

	bool set_details(const QString& name, const QString& key, bool production, bool save_to_disk);

	std::vector<std::shared_ptr<UniverseProfile>> get_universe_list() const;
	std::shared_ptr<UniverseProfile> get_universe_profile_by_id(UniverseProfile::Id universe_id) const;

	std::optional<UniverseProfile::Id> add_universe(const QString& universe_name, long long universe_id);
	void delete_universe(UniverseProfile::Id id);

signals:
	void force_save();
	void details_changed();
	void universe_list_changed(std::optional<UniverseProfile::Id> new_universe);
	void hidden_datastore_list_changed();
	void recent_mem_sorted_map_list_changed();
	void recent_ordered_datastore_list_changed();
	void recent_topic_list_changed();

private:
	bool universe_name_available(const QString& universe_name) const;
	bool universe_id_available(long long universe_id) const;

	Id id;

	QString name;
	QString key;
	bool production = false;
	bool save_to_disk = false;

	std::map<UniverseProfile::Id, std::shared_ptr<UniverseProfile>> universes;

	std::function<bool(const QString&)> api_key_name_available;
};

class UserProfile : public QObject
{
	Q_OBJECT
public:
	static UserProfile& get();
	static std::shared_ptr<ApiKeyProfile> get_selected_api_key();

	const QString& get_qt_theme() const { return qt_theme; }
	void set_qt_theme(const QString& theme_name);

	bool get_autoclose_progress_window() const { return autoclose_progress_window; }
	void set_autoclose_progress_window(bool autoclose);

	bool get_less_verbose_bulk_operations() const { return less_verbose_bulk_operations; }
	void set_less_verbose_bulk_operations(bool less_verbose);

	bool get_show_datastore_name_filter() const { return show_datastore_name_filter; }
	void set_show_datastore_name_filter(bool show_filter);

	std::vector<std::shared_ptr<ApiKeyProfile>> get_api_key_list() const;
	std::shared_ptr<ApiKeyProfile> get_api_key_by_id(ApiKeyProfile::Id id) const;

	std::optional<ApiKeyProfile::Id> add_api_key(const QString& name, const QString& key, bool production, bool save_key_to_disk);
	void delete_api_key(ApiKeyProfile::Id id);

	void select_api_key(std::optional<ApiKeyProfile::Id> id);

signals:
	void qt_theme_changed();
	void autoclose_changed();
	void selected_api_key_changed();
	void api_key_list_changed(std::optional<ApiKeyProfile::Id> selected_id);
	void universe_list_changed(std::optional<UniverseProfile::Id> new_universe);
	void hidden_datastore_list_changed();
	void recent_mem_sorted_map_list_changed();
	void recent_ordered_datastore_list_changed();
	void recent_topic_list_changed();
	void show_datastore_filter_changed();

private:
	explicit UserProfile(QObject* parent = nullptr);

	void api_key_details_changed();

	bool profile_name_available(const QString& name) const;

	void load_from_disk();
	void save_to_disk();

	QString qt_theme;
	bool autoclose_progress_window = true;
	bool less_verbose_bulk_operations = true;
	bool show_datastore_name_filter = false;

	std::map<ApiKeyProfile::Id, std::shared_ptr<ApiKeyProfile>> api_keys;
	std::optional<ApiKeyProfile::Id> selected_key_id;

	LockableBool load_flag;
};
