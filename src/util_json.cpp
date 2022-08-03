#include "util_json.h"

#include <QChar>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

std::optional<QString> decode_json_string(const QString& json_string)
{
	if (json_string.size() < 2 || json_string[0] != '"' || json_string[json_string.size() - 1] != '"')
	{
		return std::nullopt;
	}

	QString possibly_json = QString{ "{\"the_string\":%1}" }.arg(json_string);
	QJsonDocument doc = QJsonDocument::fromJson(possibly_json.toUtf8());
	if (doc.isObject() == false)
	{
		return std::nullopt;
	}

	QJsonObject root = doc.object();
	QJsonObject::iterator string_it = root.find("the_string");
	if (string_it == root.end() || string_it->isString() == false)
	{
		return std::nullopt;
	}

	return string_it->toString();
}

QString encode_json_string(const QString& string)
{
	QJsonArray tmp_array;
	tmp_array.append(string);
	QString array_string = QJsonDocument(tmp_array).toJson(QJsonDocument::Compact);
	return array_string.mid(1, array_string.size() - 2);
}
