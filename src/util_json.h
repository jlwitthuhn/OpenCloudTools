#pragma once

#include <optional>

#include <QString>

#include "util_enum.h"

class QJsonValueRef;

class JsonValue
{
public:
	static std::optional<JsonValue> from_qjson(const QJsonValueRef& qjson);

	static JsonValue from_json_array(const QString& input);
	static JsonValue from_json_object(const QString& input);

	JsonValue(double input);
	JsonValue(bool input);
	JsonValue(const QString& input);

	QString get_short_display_string() const;

private:
	JsonValue(JsonDataType type, const QString& json_string);

	JsonDataType type = JsonDataType::String;
	QString json_string;
};

std::optional<QString> condense_json(const QString& json_string);
std::optional<QString> decode_json_string(const QString& json_string);
QString encode_json_string(const QString& string);
