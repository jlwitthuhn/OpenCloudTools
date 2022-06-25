#include "build_info.h"

#include <cstddef>

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

#if !defined(_MSC_FULL_VER) && !defined(__clang_version__) && !defined(__GNUC__)
[[deprecated]]
static std::string get_default_string()
{
	return "Unknown Compiler";
}
#endif

std::string get_cxx_compiler_version_string()
{
#if defined(_MSC_FULL_VER)
	return "MSVC " + std::to_string(_MSC_FULL_VER);
#elif defined(__clang_version__) && defined(__apple_build_version__)
	return std::string{ "Apple Clang " } + __clang_version__;
#elif defined(__clang_version__)
	return std::string{ "Clang " } + __clang_version__;
#elif defined(__GNUC__)
	return std::string{ "GCC " } + std::to_string(__GNUC__) + '.' + std::to_string(__GNUC_MINOR__) + '.' + std::to_string(__GNUC_PATCHLEVEL__);
#else
	return get_default_string();
#endif
}
