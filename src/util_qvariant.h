#pragma once

#include <optional>

class QVariant;

bool qvariant_is_byte_array(const QVariant& variant, std::optional<int> length = std::nullopt);
