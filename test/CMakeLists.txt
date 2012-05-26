set(SRC_LIST testlibxmrservice.c)

include_directories(${GOBJECT_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(testlibxmrservice ${SRC_LIST})
target_link_libraries(testlibxmrservice xmrservice ${GOBJECT_LIBRARIES})
