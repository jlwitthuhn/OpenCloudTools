#pragma once

class QString;

class DataValidator
{
public:
	static bool is_bool(const QString& string);
	static bool is_number(const QString& string);
	static bool is_json(const QString& string);
	static bool is_json_array(const QString& string);
};
