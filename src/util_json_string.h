#pragma once

#include <optional>

#include <QString>

std::optional<QString> decode_json_string(const QString& json_string);
QString encode_json_string(const QString& string);
