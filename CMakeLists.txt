cmake_minimum_required(VERSION 3.16)

project(OpenCloudTools VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(USE_QT5 "Build with Qt 5 instead of Qt 6" FALSE)

if (USE_QT5)
	find_package(Qt5 REQUIRED COMPONENTS Network Widgets)
else()
	find_package(Qt6 REQUIRED COMPONENTS Network Widgets)
endif()

add_library(extern_sqlite3 STATIC ./extern/sqlite/sqlite3.c)

add_executable(OpenCloudTools
	./src/main.cpp
	./src/api_key.cpp
	./src/api_key.h
	./src/api_response.cpp
	./src/api_response.h
	./src/data_request.cpp
	./src/data_request.h
	./src/datastore_model.cpp
	./src/datastore_model.h
	./src/diag_confirm_change.cpp
	./src/diag_confirm_change.h
	./src/diag_operation_in_progress.cpp
	./src/diag_operation_in_progress.h
	./src/http_req_builder.cpp
	./src/http_req_builder.h
	./src/http_wrangler.cpp
	./src/http_wrangler.h
	./src/panel_bulk_data.cpp
	./src/panel_bulk_data.h
	./src/panel_datastore_explore.cpp
	./src/panel_datastore_explore.h
	./src/panel_http_log.cpp
	./src/panel_http_log.h
	./src/panel_universe_prefs.cpp
	./src/panel_universe_prefs.h
	./src/sqlite_wrapper.cpp
	./src/sqlite_wrapper.h
	./src/tooltip_text.h
	./src/user_settings.cpp
	./src/user_settings.h
	./src/util_enum.cpp
	./src/util_enum.h
	./src/util_json_string.cpp
	./src/util_json_string.h
	./src/util_validator.cpp
	./src/util_validator.h
	./src/util_wed.cpp
	./src/util_wed.h
	./src/window_api_key_add.cpp
	./src/window_api_key_add.h
	./src/window_api_key_manage.cpp
	./src/window_api_key_manage.h
	./src/window_datastore_bulk_op.cpp
	./src/window_datastore_bulk_op.h
	./src/window_datastore_bulk_op_progress.cpp
	./src/window_datastore_bulk_op_progress.h
	./src/window_datastore_entry_versions_view.cpp
	./src/window_datastore_entry_versions_view.h
	./src/window_datastore_entry_view.cpp
	./src/window_datastore_entry_view.h
	./src/window_datastore_universe_add.cpp
	./src/window_datastore_universe_add.h
	./src/window_hidden_datastore_add.cpp
	./src/window_hidden_datastore_add.h
	./src/window_main.cpp
	./src/window_main.h
)

if (USE_QT5)
	target_compile_definitions(OpenCloudTools PRIVATE QT5_COMPAT)
endif()

target_include_directories(OpenCloudTools PRIVATE ./extern/sqlite)

target_link_libraries(OpenCloudTools PRIVATE extern_sqlite3)
if (USE_QT5)
	target_link_libraries(OpenCloudTools PRIVATE Qt5::Network)
	target_link_libraries(OpenCloudTools PRIVATE Qt5::Widgets)
else()
	target_link_libraries(OpenCloudTools PRIVATE Qt6::Network)
	target_link_libraries(OpenCloudTools PRIVATE Qt6::Widgets)
endif()


if (MSVC)
	target_compile_options(OpenCloudTools PRIVATE /W3)
else()
	target_compile_options(OpenCloudTools PRIVATE -Wall -Wextra)
endif()

install(TARGETS OpenCloudTools)
