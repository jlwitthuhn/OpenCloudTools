#include "util_validator.h"

#include <QJsonDocument>
#include <QString>

bool DataValidator::is_bool(const QString& string)
{
	return string == "true" || string == "false";
}

bool DataValidator::is_number(const QString& string)
{
	bool success = false;
	string.toDouble(&success);
	return success;
}

bool DataValidator::is_json(const QString& string)
{
	QJsonDocument doc = QJsonDocument::fromJson(string.toUtf8());
	return doc.isObject() || doc.isArray();
}

bool DataValidator::is_json_array(const QString& string)
{
	QJsonDocument doc = QJsonDocument::fromJson(string.toUtf8());
	return doc.isArray();
}
