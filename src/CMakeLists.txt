file(GLOB SOURCE_HEADERS
    ${CMAKE_SOURCE_DIR}/include/*.h
)

file(GLOB SOURCE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

include(FindPkgConfig)
pkg_search_module(LIBPJPROJECT REQUIRED libpjproject)

add_library(SOURCE STATIC ${SOURCE_HEADERS} ${SOURCE_SOURCES})

target_include_directories(SOURCE PUBLIC ${CMAKE_SOURCE_DIR}/include ${LIBPJPROJECT_INCLUDE_DIRS})
target_link_libraries(SOURCE ${LIBPJPROJECT_LINK_LIBRARIES})