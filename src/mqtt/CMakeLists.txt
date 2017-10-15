
add_executable(mqttd main.cpp mqtt_service.cpp)

target_link_libraries(mqttd base co mqttpacket ${SYS_LIBS})
