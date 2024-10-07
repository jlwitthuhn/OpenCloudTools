#include "util_json.h"

#include <sstream>
#include <string>

#include <QChar>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "assert.h"

std::optional<JsonValue> JsonValue::from_qjson(const QJsonValueRef& qjson)
{
	if (qjson.isBool())
	{
		return JsonValue{ qjson.toBool() };
	}
	else if (qjson.isDouble())
	{
		return JsonValue{ qjson.toDouble() };
	}
	else if (qjson.isString())
	{
		return JsonValue{ qjson.toString() };
	}
	else if (qjson.isArray())
	{
		const QJsonDocument qjson_doc{ qjson.toArray() };
		const QString json_str{ qjson_doc.toJson() };
		return JsonValue::from_json_array(json_str);
	}
	else if (qjson.isObject())
	{
		const QJsonDocument qjson_doc{ qjson.toObject() };
		const QString json_str{ qjson_doc.toJson() };
		return JsonValue::from_json_object(json_str);
	}
	else
	{
		OCTASSERT(false);
		return std::nullopt;
	}
}

JsonValue JsonValue::from_json_array(const QString& input)
{
	return JsonValue(JsonDataType::Array, input);
}

JsonValue JsonValue::from_json_object(const QString& input)
{
	return JsonValue(JsonDataType::Object, input);
}

JsonValue::JsonValue(const bool input) :
	type{ JsonDataType::Bool },
	json_string{ input ? "true" : "false" }
{

}

JsonValue::JsonValue(const double input) :
	type{ JsonDataType::Number },
	json_string{ QString::fromStdString((std::stringstream{} << input).str()) }
{

}

JsonValue::JsonValue(const QString& input) :
	type{ JsonDataType::String },
	json_string{ encode_json_string(input) }
{

}

QString JsonValue::get_short_display_string() const
{
	switch (type)
	{
		case JsonDataType::Bool:
			return json_string;
		case JsonDataType::Number:
			return json_string;
		case JsonDataType::String:
			if (const std::optional<QString> the_string = decode_json_string(json_string))
			{
				return *the_string;
			}
			else
			{
				OCTASSERT(false);
				return "[Json String Error]";
			}
		case JsonDataType::Array:
			return "[Array]";
		case JsonDataType::Object:
			return "[Table]";
		default:
			OCTASSERT(false);
			return "[Type Error]";
	}
}

JsonValue::JsonValue(const JsonDataType type, const QString& json_string) :
	type{ type },
	json_string{ json_string }
{

}

std::optional<QString> condense_json(const QString& json_string)
{
	QJsonDocument doc = QJsonDocument::fromJson(json_string.toUtf8());
	if (doc.isNull())
	{
		return std::nullopt;
	}
	else
	{
		return doc.toJson(QJsonDocument::Compact);
	}
}

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
