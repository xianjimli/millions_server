#include "gtest/gtest.h"
#include "mqtt/topics.h"

TEST(TopicItemTest, topic_item_basic) {
	worker_t w;
	topic_item_t item;
	const char* topic = "hello";
	ASSERT_EQ(true, topic_item_init(&item, topic));
	ASSERT_EQ(0, strcmp(item.topic, topic));

	ASSERT_EQ(true, topic_item_is_empty(&item));
	ASSERT_EQ(true, topic_item_add(&item, &w));
	ASSERT_EQ(false, topic_item_is_empty(&item));
	ASSERT_EQ(true, topic_item_has(&item, &w));
	ASSERT_EQ(true, topic_item_remove(&item, &w));
	ASSERT_EQ(true, topic_item_is_empty(&item));
	ASSERT_EQ(false, topic_item_has(&item, &w));
	ASSERT_EQ(true, topic_item_deinit(&item));	
}

