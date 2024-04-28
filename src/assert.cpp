#include "assert.h"

#include <string>

#include <QMessageBox>

void oct_do_assert(const char* const file, int line, const bool condition)
{
	if (!condition)
	{
		const std::string message = std::string{ "Assert failed:\n" } + file + "\nLine " + std::to_string(line);
		QMessageBox::critical(nullptr, "Assert Failed", QString::fromStdString(message));
	}
}
