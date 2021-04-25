/*
 * Copyright (c) 2019-2021 Siddharth Chandrasekaran <siddharth@embedjournal.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

static int test_cp_build_packet(struct osdp_pd *p, uint8_t *buf, int len, int maxlen)
{
	int cmd_len;
	uint8_t cmd_buf[128];

	if (len > 128) {
		printf("cmd_buf len err - %d/%d\n", len, 128);
		return -1;
	}
	cmd_len = len;
	memcpy(cmd_buf, buf, len);

	if ((len = osdp_phy_packet_init(p, buf, maxlen)) < 0) {
		printf("failed to phy_build_packet_head\n");
		return -1;
	}
	memcpy(buf + len, cmd_buf, cmd_len);
	len += cmd_len;
	if ((len = osdp_phy_packet_finalize(p, buf, len, maxlen)) < 0) {
		printf("failed to build command\n");
		return -1;
	}
	return len;
}

int test_cp_build_packet_poll(struct osdp *ctx)
{
	int len;
	struct osdp_pd *p = GET_CURRENT_PD(ctx);
	uint8_t packet[512] = { CMD_POLL };
	uint8_t expected[] = {
#ifndef CONFIG_OSDP_SKIP_MARK_BYTE
		0xff,
#endif
		0x53, 0x65, 0x08, 0x00, 0x04, 0x60, 0x60, 0x90
	};

	printf("Testing cp_build_packet(CMD_POLL) -- ");
	if ((len = test_cp_build_packet(p, packet, 1, 512)) < 0) {
		printf("error!\n");
	}
	CHECK_ARRAY(packet, len, expected);
	printf("success!\n");
	return 0;
}

int test_cp_build_packet_id(struct osdp *ctx)
{
	int len;
	struct osdp_pd *p = GET_CURRENT_PD(ctx);
	uint8_t packet[512] = { CMD_ID, 0x00 };
	uint8_t expected[] = {
#ifndef CONFIG_OSDP_SKIP_MARK_BYTE
		0xff,
#endif
		0x53, 0x65, 0x09, 0x00, 0x05, 0x61, 0x00, 0xe9, 0x4d
	};

	printf("Testing cp_build_packet(CMD_ID) -- ");
	if ((len = test_cp_build_packet(p, packet, 2, 512)) < 0) {
		printf("error!\n");
		return -1;
	}
	CHECK_ARRAY(packet, len, expected);
	printf("success!\n");
	return 0;
}

int test_phy_decode_packet_ack(struct osdp *ctx)
{
	uint8_t *buf;
	int len, err;
	struct osdp_pd *p = GET_CURRENT_PD(ctx);
	uint8_t packet[] = {
#ifndef CONFIG_OSDP_SKIP_MARK_BYTE
		0xff,
#endif
		0x53, 0xe5, 0x08, 0x00, 0x05, 0x40, 0xe3, 0xa5
	};
	uint8_t expected[] = { REPLY_ACK };

	printf("Testing phy_decode_packet(REPLY_ACK) -- ");
	err = osdp_phy_check_packet(p, packet, sizeof(packet), &len);
	if (err) {
		printf("failed!\n");
		return -1;
	}
	if ((len = osdp_phy_decode_packet(p, packet, len, &buf)) < 0) {
		printf("failed!\n");
		return -1;
	}
	CHECK_ARRAY(buf, len, expected);
	printf("success!\n");
	return 0;
}

int test_cp_phy_setup(struct test *t)
{
	/* mock application data */
	osdp_pd_info_t info = {
		.address = 101,
		.baud_rate = 9600,
		.flags = 0,
		.channel.data = NULL,
		.channel.send = NULL,
		.channel.recv = NULL,
		.channel.flush = NULL,
		.scbk = NULL,
	};
	osdp_logger_init(t->loglevel, printf);
	struct osdp *ctx = (struct osdp *) osdp_cp_setup(1, &info, NULL);
	if (ctx == NULL) {
		printf("   init failed!\n");
		return -1;
	}
	SET_CURRENT_PD(ctx, 0);
	t->mock_data = (void *)ctx;
	return 0;
}

void test_cp_phy_teardown(struct test *t)
{
	osdp_cp_teardown(t->mock_data);
}

void run_cp_phy_tests(struct test *t)
{
	if (test_cp_phy_setup(t))
		return;

	DO_TEST(t, test_cp_build_packet_poll);
	DO_TEST(t, test_cp_build_packet_id);
	DO_TEST(t, test_phy_decode_packet_ack);

	test_cp_phy_teardown(t);
}
