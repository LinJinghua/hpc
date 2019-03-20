cmake_minimum_required(VERSION 2.8)

project(mongoc C)

# Specify the minimum version you require.
# find_package (libmongoc-1.0 1.7 REQUIRED)

# message ("--   mongoc found version \"${MONGOC_VERSION}\"")
# message ("--   mongoc include path \"${MONGOC_INCLUDE_DIRS}\"")
# message ("--   mongoc libraries \"${MONGOC_LIBRARIES}\"")

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCS_DIR)

set(MONGOC_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include/libmongoc-1.0 ${PROJECT_SOURCE_DIR}/include/libbson-1.0)
set(MONGOC_LIBRARIES ${PROJECT_SOURCE_DIR}/lib/libbson-1.0.so ${PROJECT_SOURCE_DIR}/lib/libmongoc-1.0.so)

add_compile_options(-Wl,–disable-new-dtags,-rpath=${PROJECT_SOURCE_DIR}/lib)

message ("--   mongoc src  ${SRCS_DIR}")
message ("--   mongoc project  ${PROJECT_SOURCE_DIR}")

# The "hello_mongoc.c" sample program is shared among four tests.
add_executable (hello_mongoc ${SRCS_DIR})
target_include_directories (hello_mongoc PRIVATE "${MONGOC_INCLUDE_DIRS}")
target_link_libraries (hello_mongoc PRIVATE "${MONGOC_LIBRARIES}")
target_compile_definitions (hello_mongoc PRIVATE "${MONGOC_DEFINITIONS}")

