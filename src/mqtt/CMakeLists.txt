set(LIB_SRC mqtt_service.cpp topics.cpp topic_item.cpp)
add_library(mqtt ${LIB_SRC})

add_executable(mqttd main.cpp)
target_link_libraries(mqttd mqtt base co mqttpacket list ${SYS_LIBS})

