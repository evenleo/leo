###################################################################
#SRC CMAKELISTS
###################################################################

file(GLOB_RECURSE srcs ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE headers ${PROJECT_SOURCE_DIR}/src/*.h)

add_library(leo STATIC ${srcs})
target_link_libraries(leo ${CMAKE_DL_LIBS}) 	#dlsym
target_link_libraries(leo ${PROTOBUF_LIBRARIES}) #protobuf
# target_link_libraries(leo ${ZLIB_LIBRARIES})	#zlib


install(TARGETS leo
        ARCHIVE DESTINATION /usr/lib
        LIBRARY DESTINATION /usr/lib)
install(FILES ${headers}
        DESTINATION /usr/local/include/leo)