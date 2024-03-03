#pragma once

#include <cstddef>

#include <functional>
#include <memory>
#include <optional>
#include <vector>
#include <set>

#include <QObject>
#include <QString>

#include "util_lock.h"

class UniverseProfile : public QObject
{
	Q_OBJECT
public:
	UniverseProfile(QObject* parent, const QString& name, long long universe_id, std::function<bool(const QString&)> name_available, std::function<bool(long long)> id_available);

	bool matches_name_and_id(const UniverseProfile& other) const;

	QString get_name() const { return name; }
	long long get_universe_id() const { return universe_id; }
	bool get_save_recent_message_topics() const { return save_recent_message_topics; }
	bool get_save_recent_ordered_datastores() const { return save_recent_ordered_datastores; }
	bool get_show_hidden_standard_datastores() const { return show_hidden_standard_datastores; }
	const std::set<QString>& get_hidden_datastore_set() const { return hidden_datastore_set; }
	const std::set<QString>& get_recent_ordered_datastore_set() const { return recent_ordered_datastore_set; }
	const std::set<QString>& get_recent_topic_set() const { return recent_topic_set; }

	bool set_details(const QString& name, long long universe_id);

	void set_save_recent_message_topics(bool save_topics);
	void set_save_recent_ordered_datastores(bool save_datastores);
	void set_show_hidden_standard_datastores(bool show_datastores);

	void add_hidden_datastore(const QString& datastore);
	void remove_hidden_datastore(const QString& datastore);

	void add_recent_ordered_datastore(const QString& datastore_name);
	void remove_recent_ordered_datastore(const QString& datastore_name);

	void add_recent_topic(const QString& topic);
	void remove_recent_topic(const QString& topic);

signals:
	void force_save();
	void details_changed();
	void hidden_datastore_list_changed();
	void recent_ordered_datastore_list_changed();
	void recent_topic_list_changed();

private:
	QString name;
	long long universe_id;

	bool save_recent_message_topics = true;
	bool save_recent_ordered_datastores = true;
	bool show_hidden_standard_datastores = false;

	std::set<QString> hidden_datastore_set;
	std::set<QString> recent_ordered_datastore_set;
	std::set<QString> recent_topic_set;

	std::function<bool(const QString&)> name_available;
	std::function<bool(long long)> id_available;
};

class ApiKeyProfile : public QObject
{
	Q_OBJECT
public:
	ApiKeyProfile(QObject* parent, const QString& name, const QString& key, bool production, bool save_to_disk, std::function<bool(const QString&)> api_key_name_available);

	QString get_name() const { return name; };
	QString get_key() const { return key; };
	bool get_production() const { return production; }
	bool get_save_to_disk() const { return save_to_disk; }

	bool set_details(const QString& name, const QString& key, bool production, bool save_to_disk);

	const std::vector<UniverseProfile*>& get_universe_list() const { return universe_list; }
	UniverseProfile* get_universe_profile_by_index(size_t universe_index) const;
	UniverseProfile* get_selected_universe() const;

	std::optional<size_t> add_universe(const QString& universe_name, long long universe_id);
	void remove_universe(size_t universe_index);

	void select_universe(std::optional<size_t> universe_index);
	void remove_selected_universe();

signals:
	void force_save();
	void details_changed();
	void universe_list_changed(std::optional<size_t> selected_universe_index);
	void hidden_datastore_list_changed();
	void recent_ordered_datastore_list_changed();
	void recent_topic_list_changed();

private:
	void sort_universe_list();
	bool universe_name_available(const QString& universe_name) const;
	bool universe_id_available(long long universe_id) const;

	QString name;
	QString key;
	bool production = false;
	bool save_to_disk = false;

	std::vector<UniverseProfile*> universe_list;
	std::optional<size_t> selected_universe_index;

	std::function<bool(const QString&)> api_key_name_available;
};

class UserProfile : public QObject
{
	Q_OBJECT
public:
	static std::unique_ptr<UserProfile>& get();
	static ApiKeyProfile* get_selected_api_key(UserProfile* user_profile = nullptr);
	static UniverseProfile* get_selected_universe(UserProfile* user_profile = nullptr);

	const QString& get_qt_theme() const { return qt_theme; }
	void set_qt_theme(const QString& theme_name);

	bool get_autoclose_progress_window() const { return autoclose_progress_window; }
	void set_autoclose_progress_window(bool autoclose);

	bool get_less_verbose_bulk_operations() const { return less_verbose_bulk_operations; }
	void set_less_verbose_bulk_operations(bool less_verbose);

	bool get_show_datastore_name_filter() const { return show_datastore_name_filter; }
	void set_show_datastore_name_filter(bool show_filter);

	const std::vector<ApiKeyProfile*>& get_api_key_list() const { return api_key_list; };
	ApiKeyProfile* get_api_key_by_index(size_t key_index);

	std::optional<size_t> add_api_key(const QString& name, const QString& key, bool production, bool save_key_to_disk);
	void delete_api_key(size_t index);

	void select_api_key(std::optional<size_t> index);

signals:
	void qt_theme_changed();
	void autoclose_changed();
	void api_key_list_changed(std::optional<size_t> selected_api_index);
	void universe_list_changed(std::optional<size_t> selected_universe_index);
	void hidden_datastore_list_changed();
	void recent_ordered_datastore_list_changed();
	void recent_topic_list_changed();
	void show_datastore_filter_changed();

private:
	explicit UserProfile(QObject* parent = nullptr);

	bool profile_name_available(const QString& name) const;

	void sort_api_key_profiles();

	void load_from_disk();
	void save_to_disk();

	QString qt_theme;
	bool autoclose_progress_window = true;
	bool less_verbose_bulk_operations = true;
	bool show_datastore_name_filter = false;

	std::vector<ApiKeyProfile*> api_key_list;
	std::optional<size_t> selected_key_index;

	LockableBool load_flag;
};
