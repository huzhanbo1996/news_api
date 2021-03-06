cmake_minimum_required(VERSION 3.6)

set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "build type")

project(api_news
		LANGUAGES C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

find_library(LIBRT rt)
find_library(LIBGUMBO gumbo)
find_package(OpenSSL REQUIRED)
find_package(workflow REQUIRED CONFIG HINTS /opt/sogou/lib/cmake/workflow/)
include_directories(${OPENSSL_INCLUDE_DIR} ${WORKFLOW_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
link_directories(${WORKFLOW_LIB_DIR})

if (WIN32)
		set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   /MP /wd4200")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /wd4200 /std:c++14")
else ()
		set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -fPIC -pipe -std=gnu90")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -pipe -std=c++11 -fno-exceptions")
endif ()

set(src 
	main.cc
	utils.cpp
	server_news_api.cpp
)

if (APPLE)
	set(WORKFLOW_LIB workflow pthread OpenSSL::SSL OpenSSL::Crypto)
else ()
	set(WORKFLOW_LIB workflow ${LIBRT})
endif ()

add_executable(api_news ${src})
target_link_libraries(api_news ${WORKFLOW_LIB} ${LIBGUMBO})

# foreach(src ${TUTORIAL_LIST})
# 	string(REPLACE "-" ";" arr ${src})
# 	list(GET arr -1 bin_name)
# 	add_executable(${bin_name} ${src}.cc)
# 	target_link_libraries(${bin_name} ${WORKFLOW_LIB} ${LIBGUMBO})
# endforeach()

# set(DIR10 tutorial-10-user_defined_protocol)
# add_executable(server ${DIR10}/server.cc ${DIR10}/message.cc)
# add_executable(client ${DIR10}/client.cc ${DIR10}/message.cc)
# target_link_libraries(server ${WORKFLOW_LIB})
# target_link_libraries(client ${WORKFLOW_LIB})

# set_target_properties(server PROPERTIES
# 	RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${DIR10})
# set_target_properties(client PROPERTIES
# 	RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${DIR10})
