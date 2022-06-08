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
	Post,
	Delete,
};

enum class ViewEditMode
{
	View,
	Edit,
};

QString get_enum_string(DatastoreEntryType enum_in);
