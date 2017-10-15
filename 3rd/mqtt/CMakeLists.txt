set(LIB_SRC 
	src/MQTTPacket/src/MQTTConnectClient.c
	src/MQTTPacket/src/MQTTConnectServer.c
	src/MQTTPacket/src/MQTTDeserializePublish.c
	src/MQTTPacket/src/MQTTFormat.c
	src/MQTTPacket/src/MQTTPacket.c
	src/MQTTPacket/src/MQTTSerializePublish.c
	src/MQTTPacket/src/MQTTSubscribeClient.c
	src/MQTTPacket/src/MQTTSubscribeServer.c
	src/MQTTPacket/src/MQTTUnsubscribeClient.c
	src/MQTTPacket/src/MQTTUnsubscribeServer.c
)

add_library(mqttpacket ${LIB_SRC})

