include_directories(${GTEST_DIR}/)
include_directories(${GTEST_DIR}/src)
include_directories(${GTEST_DIR}/include)
include_directories(${GTEST_DIR}/make)

set(APP_SRC 
	${GTEST_DIR}/src/gtest-all.cc
	${GTEST_DIR}/src/gtest_main.cc
	topics_test.cc
	topic_item_test.cc
	)

add_executable(runTest ${APP_SRC})
target_link_libraries(runTest mqtt base co mqttpacket list ${SYS_LIBS}) 


