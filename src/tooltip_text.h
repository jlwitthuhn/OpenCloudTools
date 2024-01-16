#pragma once

#include <QString>

namespace ToolTip
{
	static const QString AddApiKeyWindow_ApiKey{ "Copy and paste the key exactly as it appears on the Roblox Creator Dashboard." };
	static const QString AddApiKeyWindow_IsProduction{ "Adds an extra warning before any write operation." };
	static const QString AddApiKeyWindow_Name{
		"Choose a unique name for this key.\n"
		"This name will be visible in the API key list."
	};
	static const QString AddApiKeyWindow_SaveToDisk{
		"Saves this key so it can be used the next time you run OpenCloudTools.\n"
		"If this option is unchecked, the key will be lost after closing."
	};

	static const QString BulkDataPanel_Download{
		"Dump all of the entries from one or more datastores to a sqlite database.\n"
		"This data can later be uploaded through the 'Bulk upload...' button."
	};
	static const QString BulkDataPanel_Delete{ "Delete all of the entries in one or more datastores." };
	static const QString BulkDataPanel_ResumeDownload{ "Resume a previous bulk download from an existing sqlite database." };
	static const QString BulkDataPanel_Undelete{ "Scan one or more datastores for deleted entries and restore their previous version." };
	static const QString BulkDataPanel_Upload{
		"Upload a sqlite datastore dump.\n"
		"This can be used to restore from a backup or transfer data from one universe to another."
	};
}
