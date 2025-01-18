#include "assert.h"

#include <sstream>
#include <string>

#include <QMessageBox>
#include <QString>

void oct_do_assert(const char* const file, int line, const bool condition)
{
	if (!condition)
	{
		std::stringstream stream;
#ifdef GIT_DESCRIBE
		stream << "Git: " << GIT_DESCRIBE << "\n";
#endif
		stream << "Assert failed:\n" << file << "\nLine " << std::to_string(line);
		QMessageBox::critical(nullptr, "Assert Failed", QString::fromStdString(stream.str()));
	}
}
