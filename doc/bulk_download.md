# Bulk Download

OpenCloudTools supports downloading all of the data saved into a single datastore through the 'Bulk Download' feature. The button to activate this can be found on the 'Bulk Data' tab. As the download progresses, the current download progress as well as the raw data will be stored into the specified sqlite database.

## Resuming a download

Once a download has been started, it can be resumed through the 'Resume Download' button on the 'Bulk Data' tab. Note that when resuming a download in the enumeration step, this will re-use the last `cursor` that was provided by the Open Cloud API. It is not clear how long these `cursor` values are valid for, so you should take care to stop the download as little as possible during this step. Once enumeration is complete and the data begins downloading, it is safe to stop downloading for any duration and then resume later.

## Tables

### datastore

This is the main table that will store the contents of the datastore(s) that you choose to download.

```
CREATE TABLE datastore (
    universe_id INTEGER NOT NULL,
    datastore_name TEXT NOT NULL,
    scope TEXT NOT NULL,
    key_name TEXT NOT NULL,
    version TEXT NOT NULL,
    data_type TEXT NOT NULL,
    data_raw TEXT NOT NULL,
    data_str TEXT,
    data_num REAL,
    data_bool INTEGER,
    userids TEXT,
    attributes TEXT,
    PRIMARY KEY (universe_id, datastore_name, scope, key_name)
)
```

* `universe_id`, `datastore_name`, `scope`, and `key_name`, uniquely describe the key being accessed.
* `version` lists the version of the key.
* `data_type` is one of `Bool`, `Number`, `String`, or `Table (Json)`.
* `data_raw` will always be populated with the raw json response received from Open Cloud.
* One of `data_str`, `data_num`, or `data_bool` may be populated for rows with a matching `data_type`. None will be populated for the 'table' type.
* `userids` and `attributes` will contain the metadata from your Datastore.

### datastore_deleted

This table stores a list of all keys that were no longer present when OpenCloudTools attempted to download them because they had been deleted. This data is stored for completeness so you can check for sure if a given key name was included in a bulk download, even if that key had no data.

```
CREATE TABLE datastore_deleted (
    universe_id INTEGER NOT NULL,
    datastore_name TEXT NOT NULL,
    scope TEXT NOT NULL,
    key_name TEXT NOT NULL,
    PRIMARY KEY (universe_id, datastore_name, scope, key_name)
)
```

### datastore_enumerate

This table stores a list of datastores that are either entirely un-enumerated or partially-enumerated. It is exclusively for resuming a download that was aborted in the enumeration step and doesn't have any important user-facing data. Once the enumeration step is complete this table will be empty.

```
CREATE TABLE datastore_enumerate (
    universe_id INTEGER NOT NULL,
    datastore_name TEXT NOT NULL,
    next_cursor TEXT,
    PRIMARY KEY (universe_id, datastore_name)
)
```

### datastore_enumerate_meta

This table stores search parameters used in the enumeration step, specifically the scope and key prefix.

```
CREATE TABLE datastore_enumerate_meta (
    universe_id INTEGER NOT NULL,
    key TEXT NOT NULL,
    value TEXT NOT NULL,
    PRIMARY KEY (universe_id, key)
)
```

### datastore_pending

This table stores a list of enumerated keys that have not yet been downloaded. It will be empty after a bulk download has fully completed.

```
CREATE TABLE datastore_pending (
    id INTEGER PRIMARY KEY,
    universe_id INTEGER NOT NULL,
    datastore_name TEXT NOT NULL,
    scope TEXT NOT NULL,
    key_name TEXT NOT NULL
)
```
