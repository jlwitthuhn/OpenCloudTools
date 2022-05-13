#include "util_wed.h"

#include <QDate>

std::optional<QString> wed()
{
	return QDate::currentDate().dayOfWeek() == 3 ? QString{ "It is Wednesday, my dudes." } : static_cast<std::optional<QString>>(std::nullopt);
}
