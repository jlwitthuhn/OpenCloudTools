cmake_minimum_required(VERSION 3.16)

project(OpenCloudTools VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Network Widgets)

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
	./src/window_datastore_explore.cpp
	./src/window_datastore_explore.h
	./src/window_datastore_universe_add.cpp
	./src/window_datastore_universe_add.h
	./src/window_view_datastore_entry.cpp
	./src/window_view_datastore_entry.h
	./src/window_view_datastore_entry_versions.cpp
	./src/window_view_datastore_entry_versions.h
)

target_link_libraries(OpenCloudTools PRIVATE Qt6::Network)
target_link_libraries(OpenCloudTools PRIVATE Qt6::Widgets)

install(TARGETS OpenCloudTools)
