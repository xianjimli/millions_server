#include "gtest/gtest.h"
#include "mqtt/topics.h"

static bool demo_worker_init(worker_t* w) {
    (void)w;
    return true;
}

static bool demo_worker_deinit(worker_t* w) {
    (void)w;

    return true;
}

static bool demo_worker_work(worker_t* w) {
    (void)w;
    return true;
}

static int s_send_bytes = 0;
static int demo_worker_write_n(worker_t* w, unsigned char* buf, int len) {
	s_send_bytes += len;

	return -1;
}

static worker_ops_t s_demo_worker_ops = {
    demo_worker_init,
    demo_worker_work,
    demo_worker_write_n,
    demo_worker_deinit
};

static worker_t* worker_deom_create() {
	worker_t* w = (worker_t*)calloc(1, sizeof(worker_t));
	w->ops = &s_demo_worker_ops;

	return w;
}

TEST(TopicsTest, topic_item_basic) {
	int i = 0;
	topics_t tree;
	char topic[128];
	const int n = 10000;
	const char* hello = "hello";
	ASSERT_EQ(true, topics_init(&tree));
	worker_t* workers[n];

	for(i = 0; i < n; i++) {
		workers[i] = worker_deom_create();
	}

	for(i = 0; i < n; i++) {
		worker_t* w = workers[i];

		snprintf(topic, sizeof(topic), "/topic/%d", i);
		ASSERT_EQ(topics_sub(&tree, topic, w) != NULL, true);
		ASSERT_EQ(topics_has_sub(&tree, topic, w), true);
		
		ASSERT_EQ(topics_sub(&tree, hello, w) != NULL, true);
		ASSERT_EQ(topics_has_sub(&tree, hello, w), true);
	}

	for(i = 0; i < n; i++) {
		worker_t* w = workers[i];

		snprintf(topic, sizeof(topic), "/topic/%d", i);
		ASSERT_EQ(topics_unsub(&tree, topic, w), true);
		ASSERT_EQ(topics_has_sub(&tree, topic, w), false);
	}
	
	for(i = 0; i < n; i++) {
		worker_t* w = workers[i];

		snprintf(topic, sizeof(topic), "/topic/%d", i);
		ASSERT_EQ(topics_sub(&tree, topic, w) != NULL, true);
		ASSERT_EQ(topics_has_sub(&tree, topic, w), true);
	}

	int send_bytes = 0;
	const char* payload = "world";
	const int payload_len = strlen(payload);

	for(i = 0; i < n; i++) {
		worker_t* w = workers[i];

		snprintf(topic, sizeof(topic), "/topic/%d", i);
		ASSERT_EQ(topics_pub(&tree, topic, payload, payload_len), true);
		send_bytes += payload_len;
		ASSERT_EQ(send_bytes, s_send_bytes);

		ASSERT_EQ(topics_pub(&tree, hello, payload, payload_len), true);
		send_bytes += n * payload_len;
		ASSERT_EQ(send_bytes, s_send_bytes);
	}

	for(i = 0; i < n; i++) {
		free(workers[i]);
	}

	ASSERT_EQ(true, topics_deinit(&tree));
}

