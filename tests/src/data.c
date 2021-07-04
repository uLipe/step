/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <sdp/sdp.h>
#include <sdp/filter.h>
#include <sdp/measurement/measurement.h>
#include <sdp/node.h>
#include "data.h"

struct sdp_test_data_pnode_cb_stats {
	uint32_t evaluate;
	uint32_t matched;
	uint32_t start;
	uint32_t run;
	uint32_t stop;
	uint32_t error;
} sdp_test_data_cb_stats = { 0 };

bool node_evaluate(struct sdp_measurement *mes, void *cfg)
{
	/* Overrides the filter engine when evaluating this node. */
	sdp_test_data_cb_stats.evaluate++;

	return true;
}

bool node_matched(struct sdp_measurement *mes, void *cfg)
{
	/* Fires when the filter engine has indicated a match for this node. */
	sdp_test_data_cb_stats.matched++;

	return true;
}

int node_start(struct sdp_measurement *mes, void *cfg)
{
	/* Fires before the node runs. */
	sdp_test_data_cb_stats.start++;

	return 0;
}

int node_run(struct sdp_measurement *mes, void *cfg)
{
	/* Node logic implementation. */
	sdp_test_data_cb_stats.run++;

	return 0;
}

int node_stop(struct sdp_measurement *mes, void *cfg)
{
	/* Fires when the node has been successfully run. */
	sdp_test_data_cb_stats.stop++;

	return 0;
}

void node_error(struct sdp_measurement *mes, void *cfg, int error)
{
	/* Fires when an error occurs running this node. */
	sdp_test_data_cb_stats.error++;
}

/* Test processor node. */
struct sdp_node sdp_test_data_pnode = {
	/* Matches if DS filter field = 0x26 OR 0x226 AND bit 26 is set. */
	.filters = {
		.count = 3,
		.chain = (struct sdp_filter[]){
			{
				.exact_match = SDP_MES_TYPE_TEMPERATURE,
				.exact_match_mask = 0xFFFF0000,
			},
			{
				.op = SDP_FILTER_OP_OR,
				.exact_match = SDP_MES_TYPE_TEMPERATURE +
					       (SDP_MES_EXT_TYPE_TEMP_DIE << 8),
				.exact_match_mask = 0xFFFF0000,
			},
			{
				.op = SDP_FILTER_OP_AND,
				.bit_match = {
					/* Bit 26 = EPOCH32 timestamp */
					.mask = 0xFBFFFFFF,
					.set = (1 << 26),
				}
			},
		},
	},

	/* Callbacks */
	.callbacks = {
		.evaluate_handler = node_evaluate,
		.matched_handler = node_matched,
		.start_handler = node_start,
		.stop_handler = node_stop,
		.run_handler = node_run,
		.error_handler = node_error,
	},

	/* Config settings */
	.config = (void *)0x12345678,
};

/* Die temperature with 32-bit timestamp payload. */
struct {
	float temp_c;
	uint32_t timestamp;
} sdp_test_data_dietemp_payload = {
	.temp_c = 32.0F,
	.timestamp = 1624305803,        /* Monday, June 21, 2021 8:03:23 PM */
};

/* Test die temp measurement, with timestamp. */
struct sdp_measurement sdp_test_mes_dietemp = {
	/* Measurement metadata. */
	.header = {
		/* Filter word. */
		.filter = {
			.base_type = SDP_MES_TYPE_TEMPERATURE,
			.ext_type = SDP_MES_EXT_TYPE_TEMP_DIE,
			.flags = {
				.data_format = SDP_MES_FORMAT_NONE,
				.encoding = SDP_MES_ENCODING_NONE,
				.compression = SDP_MES_COMPRESSION_NONE,
				.timestamp = SDP_MES_TIMESTAMP_EPOCH_32,
			},
		},
		/* SI Unit word. */
		.unit = {
			.si_unit = SDP_MES_UNIT_SI_DEGREE_CELSIUS,
			.scale_factor = SDP_MES_SI_SCALE_NONE,
			.ctype = SDP_MES_UNIT_CTYPE_IEEE754_FLOAT32,
		},
		/* Source/Len word. */
		.srclen = {
			.len = sizeof(sdp_test_data_dietemp_payload),
			.fragment = 0,
			.samples = 0,
			.sourceid = 10,
		},
	},

	/* Die temperature in C plus 32-bit epoch timestamp. */
	.payload = &sdp_test_data_dietemp_payload,
};