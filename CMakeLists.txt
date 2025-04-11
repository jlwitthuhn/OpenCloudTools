cmake_minimum_required(VERSION 3.16)

project(OpenCloudTools VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(OCT_USE_CLANG_TIDY "Analyze source files with clang-tidy" FALSE)
option(OCT_USE_GIT_TAG "Pull the current git tag during build" FALSE)
option(OCT_USE_IWYU "Analyze includes with include-what-you-use" FALSE)
option(OCT_USE_QT5 "Build with Qt 5 instead of Qt 6" FALSE)

if (OCT_USE_CLANG_TIDY)
	set(CMAKE_CXX_CLANG_TIDY clang-tidy)
endif()

if (OCT_USE_IWYU)
	set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE include-what-you-use -Xiwyu "--mapping_file=${CMAKE_SOURCE_DIR}/extra/iwyu.imp")
endif()

if(OCT_USE_GIT_TAG)
	execute_process(
		COMMAND git describe --tags
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		OUTPUT_VARIABLE GIT_DESCRIBE
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif()

if(OCT_USE_QT5)
	find_package(Qt5 REQUIRED COMPONENTS Network Widgets)
else()
	find_package(Qt6 REQUIRED COMPONENTS Network Widgets)
endif()

add_library(extern_sqlite3 STATIC ./extern/sqlite/sqlite3.c)

set(OPENCLOUDTOOLS_SRC
	./src/main.cpp
	./src/assert.cpp
	./src/assert.h
	./src/build_info.cpp
	./src/build_info.h
	./src/data_request.cpp
	./src/data_request.h
	./src/diag_confirm_change.cpp
	./src/diag_confirm_change.h
	./src/diag_list_string.cpp
	./src/diag_list_string.h
	./src/diag_operation_in_progress.cpp
	./src/diag_operation_in_progress.h
	./src/gui_constants.cpp
	./src/gui_constants.h
	./src/http_req_builder.cpp
	./src/http_req_builder.h
	./src/http_wrangler.cpp
	./src/http_wrangler.h
	./src/model_api_opencloud.cpp
	./src/model_api_opencloud.h
	./src/model_common.cpp
	./src/model_common.h
	./src/model_qt.cpp
	./src/model_qt.h
	./src/panel_ban_list.cpp
	./src/panel_ban_list.h
	./src/panel_ban_list_add.cpp
	./src/panel_ban_list_add.h
	./src/panel_bulk_data.cpp
	./src/panel_bulk_data.h
	./src/panel_datastore_ordered.cpp
	./src/panel_datastore_ordered.h
	./src/panel_datastore_ordered_add.cpp
	./src/panel_datastore_ordered_add.h
	./src/panel_datastore_standard.cpp
	./src/panel_datastore_standard.h
	./src/panel_datastore_standard_add.cpp
	./src/panel_datastore_standard_add.h
	./src/panel_http_log.cpp
	./src/panel_http_log.h
	./src/panel_messaging_service.cpp
	./src/panel_messaging_service.h
	./src/panel_mem_sorted_map.cpp
	./src/panel_mem_sorted_map.h
	./src/panel_universe_prefs.cpp
	./src/panel_universe_prefs.h
	./src/profile.cpp
	./src/profile.h
	./src/roblox_time.cpp
	./src/roblox_time.h
	./src/sqlite_wrapper.cpp
	./src/sqlite_wrapper.h
	./src/subwindow.cpp
	./src/subwindow.h
	./src/tooltip_text.h
	./src/util_alert.cpp
	./src/util_alert.h
	./src/util_debug.cpp
	./src/util_debug.h
	./src/util_enum.cpp
	./src/util_enum.h
	./src/util_id.cpp
	./src/util_id.h
	./src/util_json.cpp
	./src/util_json.h
	./src/util_lock.h
	./src/util_qvariant.cpp
	./src/util_qvariant.h
	./src/util_validator.cpp
	./src/util_validator.h
	./src/util_wed.cpp
	./src/util_wed.h
	./src/widget_text_log.cpp
	./src/widget_text_log.h
	./src/window_add_universe.cpp
	./src/window_add_universe.h
	./src/window_api_key_manage.cpp
	./src/window_api_key_manage.h
	./src/window_ban_view.cpp
	./src/window_ban_view.h
	./src/window_datastore_bulk_op.cpp
	./src/window_datastore_bulk_op.h
	./src/window_datastore_bulk_op_progress.cpp
	./src/window_datastore_bulk_op_progress.h
	./src/window_datastore_entry_versions_view.cpp
	./src/window_datastore_entry_versions_view.h
	./src/window_datastore_entry_view.cpp
	./src/window_datastore_entry_view.h
	./src/window_main.cpp
	./src/window_main.h
	./src/window_main_menu_bar.cpp
	./src/window_main_menu_bar.h
	./src/window_ordered_datastore_entry_view.cpp
	./src/window_ordered_datastore_entry_view.h
)

if(APPLE)
	set(MACOSX_BUNDLE_ICON_FILE icon.icns)
	set(ICON_MACOS "${CMAKE_CURRENT_SOURCE_DIR}/res/mac/icon.icns")
	set_source_files_properties(${ICON_MACOS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	add_executable(OpenCloudTools
		MACOSX_BUNDLE
		${OPENCLOUDTOOLS_SRC}
		${ICON_MACOS}
	)
elseif(WIN32)
	add_executable(OpenCloudTools
		WIN32
		${OPENCLOUDTOOLS_SRC}
		./res/win/resource.rc
	)
else()
	add_executable(OpenCloudTools
		${OPENCLOUDTOOLS_SRC}
	)
endif()

if(DEFINED GIT_DESCRIBE)
	if(NOT GIT_DESCRIBE STREQUAL "")
		target_compile_definitions(OpenCloudTools PRIVATE GIT_DESCRIBE="${GIT_DESCRIBE}")
	else()
		message(WARNING "Failed to get a valid version from 'git describe'")
	endif()
endif()

target_include_directories(OpenCloudTools PRIVATE ./extern/sqlite)

target_link_libraries(OpenCloudTools PRIVATE extern_sqlite3)
if(OCT_USE_QT5)
	target_link_libraries(OpenCloudTools PRIVATE Qt5::Network)
	target_link_libraries(OpenCloudTools PRIVATE Qt5::Widgets)
else()
	target_link_libraries(OpenCloudTools PRIVATE Qt6::Network)
	target_link_libraries(OpenCloudTools PRIVATE Qt6::Widgets)
endif()

if(MSVC)
	target_compile_options(OpenCloudTools PRIVATE /W4 /MP)
else()
	target_compile_options(OpenCloudTools PRIVATE -Wall -Wextra -pedantic)
endif()

if(APPLE)
	install(TARGETS OpenCloudTools BUNDLE DESTINATION .)
else()
	install(TARGETS OpenCloudTools)
endif()

if(NOT OCT_USE_QT5)
	qt_generate_deploy_app_script(
		TARGET OpenCloudTools
		OUTPUT_SCRIPT deploy_script
		NO_TRANSLATIONS
		NO_COMPILER_RUNTIME
		NO_UNSUPPORTED_PLATFORM_ERROR
	)
	install(SCRIPT ${deploy_script})
endif()
