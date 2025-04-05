#include "model_common.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QVariant>

#include "util_enum.h"
#include "util_json.h"
#include "util_validator.h"

QString BanListGameJoinRestrictionUpdate::to_json() const
{
	QJsonObject inner_obj;
	inner_obj.insert("active", QJsonValue::fromVariant(active));
	if (duration)
	{
		inner_obj.insert("duration", QJsonValue::fromVariant(*duration));
	}
	inner_obj.insert("privateReason", QJsonValue::fromVariant(private_reason));
	inner_obj.insert("displayReason", QJsonValue::fromVariant(display_reason));
	inner_obj.insert("excludeAltAccounts", QJsonValue::fromVariant(exclude_alt_accounts));

	QJsonObject outer_obj;
	outer_obj.insert("gameJoinRestriction", inner_obj);

	const QJsonDocument json_doc{ outer_obj };
	return QString::fromUtf8(json_doc.toJson(QJsonDocument::Compact));
}

StandardDatastoreEntryFull::StandardDatastoreEntryFull(long long universe_id, const QString& datastore_name, const QString& scope, const QString& key_name, const QString& version, const std::optional<QString>& userids, const std::optional<QString>& attributes, const QString& data) :
	universe_id{ universe_id }, datastore_name{ datastore_name }, scope{ scope }, key_name{ key_name }, version{ version }, userids{ userids }, attributes{ attributes }, data_raw{ data }
{
	data_decoded = data_raw;
	if (DataValidator::is_json(data))
	{
		entry_type = DatastoreEntryType::Json;
	}
	else if (std::optional<QString> decoded_string = decode_json_string(data))
	{
		data_decoded = *decoded_string;
		entry_type = DatastoreEntryType::String;
	}
	else if (DataValidator::is_number(data))
	{
		entry_type = DatastoreEntryType::Number;
	}
	else if (DataValidator::is_bool(data))
	{
		entry_type = DatastoreEntryType::Bool;
	}
	else
	{
		entry_type = DatastoreEntryType::Error;
	}
}
