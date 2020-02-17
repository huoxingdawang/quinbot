cmake_minimum_required(VERSION 3.9)
project(quinbot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "/utf-8 ${CMAKE_CXX_FLAGS}")  # use UTF-8 source files
set(CMAKE_CXX_FLAGS "/MP ${CMAKE_CXX_FLAGS}")  # build with object level parallelism

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include(cotire)
include(FindVcpkgIncludeDir)
include(FixDebugLibraryLookup)

include_directories(${VCPKG_INCLUDE_DIR})
include_directories(src)

message("VCPKG_INCLUDE_DIR = ${VCPKG_INCLUDE_DIR}")
message("VCPKG_TARGET_TRIPLET = ${VCPKG_TARGET_TRIPLET}")

add_compile_definitions(BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
                        _WIN32_WINNT=0x0501
                        WIN32_LEAN_AND_MEAN
                        NOMINMAX)

# read app id from app_id.txt
file(READ "app_id.txt" APP_ID)
string(STRIP "${APP_ID}" APP_ID)
set(APP_ID "\"${APP_ID}\"")
add_compile_definitions(APP_ID=${APP_ID})

find_package(unofficial-iconv CONFIG REQUIRED)
find_package(sqlite3 CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(cpr CONFIG REQUIRED)

include(FixLinkConflict)
include_directories("C:/Python-x86/include")

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
set(LIB_NAME "app")
add_library(${LIB_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${LIB_NAME} PRIVATE unofficial::iconv::libiconv unofficial::iconv::libcharset)
target_link_libraries(${LIB_NAME} PRIVATE sqlite3)
target_link_libraries(${LIB_NAME} PRIVATE ZLIB::ZLIB)
target_link_libraries(${LIB_NAME} PRIVATE cpr)
target_link_libraries(${LIB_NAME} PRIVATE E:/quinbot/quinbot/libs/Urlmon.lib)
target_link_libraries(${LIB_NAME} PRIVATE "C:/Python-x86/libs/python37_d.lib")


cotire(${LIB_NAME})

add_custom_command(TARGET ${LIB_NAME}
                   POST_BUILD
                   COMMAND
                   powershell -ExecutionPolicy Bypass -NoProfile -File "${PROJECT_SOURCE_DIR}/scripts/post_build.ps1" ${APP_ID} ${LIB_NAME} "$<TARGET_FILE_DIR:${LIB_NAME}>")
