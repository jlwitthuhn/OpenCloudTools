#pragma once

#include <QString>

namespace ToolTip
{
	static const QString BulkDataPanel_Download{ "Dump all of the entries in one or more datastores to a sqlite database. This data can later be uploaded through the 'Bulk upload...' button." };
	static const QString BulkDataPanel_Delete{ "Delete all of the entries in one or more datastores." };
	static const QString BulkDataPanel_ResumeDownload{ "Resume a previous bulk download from an existing sqlite database." };
	static const QString BulkDataPanel_Undelete{ "Scan one or more datastores for deleted entries and restore their previous version." };
	static const QString BulkDataPanel_Upload{ "Upload a sqlite datastore dump. This can be used to restore from a backup or transfer data from one universe to another." };
}
