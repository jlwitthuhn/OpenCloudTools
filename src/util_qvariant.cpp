#include "util_qvariant.h"

#include <QMetaType>
#include <QVariant>

bool qvariant_is_byte_array(const QVariant& variant)
{
#ifdef QT5_COMPAT
	return variant.type() == QVariant::Type::ByteArray;
#else
	return variant.metaType().id() == QMetaType::QByteArray;
#endif
}
