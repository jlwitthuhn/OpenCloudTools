#include "util_compiler.h"

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
