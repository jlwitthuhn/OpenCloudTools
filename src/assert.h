#pragma once

void oct_do_assert(const char* file, int line, bool condition);

#define OCTASSERT(COND) oct_do_assert(__FILE__, __LINE__, static_cast<bool>(COND))
