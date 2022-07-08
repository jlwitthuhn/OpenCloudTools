#include "util_enum.h"

#include <QString>

QString get_enum_string(const HttpRequestType enum_in)
{
	switch (enum_in)
	{
	case HttpRequestType::Get:
		return "Get";
	case HttpRequestType::Post:
		return "Post";
	case HttpRequestType::Delete:
		return "Delete";
	}
	return "Big Error";
}

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
		return "Table (Json)";
	}
	return "Big Error";
}
