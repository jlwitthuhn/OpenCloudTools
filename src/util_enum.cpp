#include "util_enum.h"

#include <QString>

QString get_enum_string(const DatastoreEntryType enum_in)
{
	switch (enum_in)
	{
	case DatastoreEntryType::Error:
		return "Error";
	case DatastoreEntryType::Bool:
		return "Bool";
	case DatastoreEntryType::Number:
		return "Number";
	case DatastoreEntryType::String:
		return "String";
	case DatastoreEntryType::Json:
		return "Json";
	}
	return "Big Error";
}
