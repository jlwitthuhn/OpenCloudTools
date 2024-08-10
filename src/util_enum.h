#pragma once

class QString;

enum class DatastoreEntryType
{
	Error,
	Bool,
	Number,
	String,
	Json,
};

enum class HttpRequestType
{
	Get,
	Patch,
	Post,
	Delete,
};

enum class JsonDataType
{
	Bool,
	Number,
	String,
	Array,
	Object,
};

enum class ViewEditMode
{
	View,
	Edit,
};

QString get_enum_string(DatastoreEntryType enum_in);
QString get_enum_string(HttpRequestType enum_in);
