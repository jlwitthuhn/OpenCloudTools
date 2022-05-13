#pragma once

#include <map>

#include <QString>

class ApiKeyProfile
{
public:
	ApiKeyProfile(const QString& name, const QString& key, bool production, bool save_to_disk);

	QString name() const { return _name; };
	QString key() const { return _key; };
	bool production() const { return _production; }
	bool save_to_disk() const { return _save_to_disk; }

	void add_universe(long long universe_id, const QString& name);
	void remove_universe(long long universe_id);
	std::map<long long, QString> universe_ids() const;

private:
	QString _name;
	QString _key;
	bool _production = false;
	bool _save_to_disk = false;

	std::map<long long, QString> _universe_ids;
};
