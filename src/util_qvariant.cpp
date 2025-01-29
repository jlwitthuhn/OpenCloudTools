#include "util_qvariant.h"

#include <QtGlobal>
#include <QByteArray>
#include <QMetaType>
#include <QVariant>

bool qvariant_is_byte_array(const QVariant& variant, std::optional<int> length)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	const bool type_matches = variant.metaType().id() == QMetaType::QByteArray;
#else
	const bool type_matches = variant.type() == QVariant::Type::ByteArray;
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
