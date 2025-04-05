#pragma once

#include <cstdint>

class QString;

enum class DatastoreEntryType : std::uint8_t
{
	Error,
	Bool,
	Number,
	String,
	Json,
};

enum class HttpRequestType : std::uint8_t
{
	Get,
	Patch,
	Post,
	Delete,
};

enum class JsonDataType : std::uint8_t
{
	Bool,
	Number,
	String,
	Array,
	Object,
};

enum class ViewEditMode : std::uint8_t
{
	View,
	Edit,
};

QString get_enum_string(DatastoreEntryType enum_in);
QString get_enum_string(HttpRequestType enum_in);
