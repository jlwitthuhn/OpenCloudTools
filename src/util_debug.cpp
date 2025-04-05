#include "util_debug.h"

#include <iostream>
#include <string>
#include <vector>

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QString>

void dump_current_palettes()
{
	const std::vector<const char*> types{ "QWidget" };
	const std::vector<QPalette::ColorGroup> groups{ QPalette::Active, QPalette::Inactive, QPalette::Disabled };

	for (const char* const this_type : types)
	{
		std::cout << "--------------------------------\n";
		std::cout << this_type << '\n';
		std::cout << "----------------" << '\n';
		const QPalette this_palette = QApplication::palette(this_type);
		for (const QPalette::ColorGroup this_group : groups)
		{
			std::string group_name = "Unknown";
			switch (this_group)
			{
				case QPalette::Active:
					group_name = "Active";
					break;
				case QPalette::Inactive:
					group_name = "Inactive";
					break;
				case QPalette::Disabled:
					group_name = "Disabled";
					break;
				default:
					// Do nothing
					break;
			}

			std::cout << group_name << " Window:" << this_palette.color(this_group, QPalette::Window).name().toStdString() << '\n';
			std::cout << group_name << " WindowText:" << this_palette.color(this_group, QPalette::WindowText).name().toStdString() << '\n';
			std::cout << group_name << " Base:" << this_palette.color(this_group, QPalette::Base).name().toStdString() << '\n';
			std::cout << group_name << " AlternateBase:" << this_palette.color(this_group, QPalette::AlternateBase).name().toStdString() << '\n';
			std::cout << group_name << " ToolTipBase:" << this_palette.color(this_group, QPalette::ToolTipBase).name().toStdString() << '\n';
			std::cout << group_name << " ToolTipText:" << this_palette.color(this_group, QPalette::ToolTipText).name().toStdString() << '\n';
			std::cout << group_name << " PlaceholderText:" << this_palette.color(this_group, QPalette::PlaceholderText).name().toStdString() << '\n';
			std::cout << group_name << " Text:" << this_palette.color(this_group, QPalette::Text).name().toStdString() << '\n';
			std::cout << group_name << " Button:" << this_palette.color(this_group, QPalette::Button).name().toStdString() << '\n';
			std::cout << group_name << " ButtonText:" << this_palette.color(this_group, QPalette::ButtonText).name().toStdString() << '\n';
			std::cout << group_name << " BrightText:" << this_palette.color(this_group, QPalette::BrightText).name().toStdString() << '\n';
            std::cout.flush();
		}
	}
}
