# Open Cloud Tools

Open Cloud Tools is a desktop application for Windows and MacOS that allows you to interact with Roblox's [Open Cloud](https://create.roblox.com/docs/cloud/open-cloud) API. Open Cloud Tools supports the DataStore and MessagingService APIs.

Icon by Roblox user [Loominatrx](https://devforum.roblox.com/u/loominatrx/summary).

## Screenshots

![Search Datastore](./extra/img/datastores_search.png)
![View Versions](./extra/img/datastore_entry_versions.png)
![Messaging](./extra/img/messaging.png)

## Features

### Messaging Service

Send messages that your game servers can consume using [MessagingService](https://create.roblox.com/docs/reference/engine/classes/MessagingService).

### Datastore Operations

Store and retrive data using Roblox's [Datastores](https://create.roblox.com/docs/cloud-services/datastores).

#### Basic Datastore Operations

* List datastores
* List datastore entries
* View entries
* View entry version history
  * View or revert to old versions
* Edit entries
* Delete entries

#### Advanced Datastore Operations

* [Bulk Download](./doc/bulk_download.md)
  * Dump all of the entries in one or more datastores to a sqlite database. This data can later be uploaded through the 'Bulk Upload' operation.
  * Large downloads can be stopped and resumed later.
* Bulk Delete
  * Delete all of the entries in one or more datastores.
* Bulk Undelete
  * Scan one or more datastores for deleted entries and restore their previous version.
* Bulk Upload
  * Upload a sqlite datastore dump. This can be used to restore from a backup or transfer data from one universe to another.

### Ordered Datastore Operations



## Creating an API Key

To create an API key, go to the [Credentials page of the Roblox Creator Dashboard](https://create.roblox.com/credentials) and click the "Create API Key" button.

### Messaging Permissions

The only permission available for Messaging is 'Publish', which is required to send messages.

### DataStore Access Permissions

The bare minimum permissions that the key will need to list and view entries are:
* List Datastores
* List Entry Keys
* Read Entry

A read-only key can also optionally include:
* List Versions
* Read Version

The remaining permissions all allow modifying the datastore and you should give them out as needed:
* Create Datastore
* Create Entry
* Update Entry
* Delete Entry

## Building

A CMake build file is provided. This project requires C++17 to build.

### Dependencies

This project depends on SQLite and Qt 5/6. A recent version of SQLite is bundled in this repository, you have to provide your own Qt.

### Supported Platforms

This program can probably build and run on any platform that supports Qt 5 or 6, but has only been tested in the following configurations that are used to build the binary distributions:

| Platform | OpenCloudTools Compiler | Qt Version | Qt Compiler | Qt Configure Options |
|----------|-------------------------|------------|-------------|----------------------|
| Mac      | XCode 13.2.1            | 6.4.2      | XCode 13.2.1 | `-opensource -opengl desktop` |
| Win Qt5  | Visual Studio 2022      | 5.15.8     | Visual Studio 2019 | `-opensource -opengl desktop -ssl -schannel -no-openssl` |
| Win Qt6  | Visual Studio 2022      | 6.4.2      | Visual Studio 2022 | `-opensource -opengl desktop -no-openssl -skip qtsensors` |

## License

This program is available under the GPLv3 License. See the [LICENSE](./LICENSE) file for the full text of this license.
