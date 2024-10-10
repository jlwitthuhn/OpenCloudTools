#include "util_qvariant.h"

#include <QMetaType>
#include <QVariant>

bool qvariant_is_byte_array(const QVariant& variant, std::optional<int> length)
{
#ifdef QT5_COMPAT
	const bool type_matches = variant.type() == QVariant::Type::ByteArray;
#else
	const bool type_matches = variant.metaType().id() == QMetaType::QByteArray;
#endif
	if (!type_matches)
	{
		return false;
	}

	if (!length)
	{
		return true;
	}

	return variant.toByteArray().size() == *length;
}
