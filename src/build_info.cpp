#include "build_info.h"

#include <algorithm>

std::string get_build_date()
{
	std::string result{ __DATE__ };
	const size_t fix_index = result.find("  ");
	if (fix_index != std::string::npos)
	{
		result.at(fix_index + 1) = '0';
	}
	return result;
}

[[deprecated]]
static std::string get_default_string()
{
	return "Unknown Compiler";
}

std::string get_cxx_compiler_version_string()
{
#ifdef _MSC_FULL_VER
	return "MSVC " + std::to_string(_MSC_FULL_VER);
#else
	return get_default_string();
#endif
}
