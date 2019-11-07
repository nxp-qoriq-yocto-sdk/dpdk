/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2018 Mellanox Technologies, Ltd
 */

#ifndef RTE_PMD_MLX5_FLOW_H_
#define RTE_PMD_MLX5_FLOW_H_

#include <netinet/in.h>
#include <sys/queue.h>
#include <stdalign.h>
#include <stdint.h>
#include <string.h>

/* Verbs header. */
/* ISO C doesn't support unnamed structs/unions, disabling -pedantic. */
#ifdef PEDANTIC
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include <infiniband/verbs.h>
#ifdef PEDANTIC
#pragma GCC diagnostic error "-Wpedantic"
#endif

#include <rte_atomic.h>
#include <rte_alarm.h>

#include "mlx5.h"
#include "mlx5_prm.h"

enum modify_reg {
	REG_A,
	REG_B,
	REG_C_0,
	REG_C_1,
	REG_C_2,
	REG_C_3,
	REG_C_4,
	REG_C_5,
	REG_C_6,
	REG_C_7,
};

/* Private rte flow items. */
enum mlx5_rte_flow_item_type {
	MLX5_RTE_FLOW_ITEM_TYPE_END = INT_MIN,
	MLX5_RTE_FLOW_ITEM_TYPE_TAG,
	MLX5_RTE_FLOW_ITEM_TYPE_TX_QUEUE,
};

/* Private rte flow actions. */
enum mlx5_rte_flow_action_type {
	MLX5_RTE_FLOW_ACTION_TYPE_END = INT_MIN,
	MLX5_RTE_FLOW_ACTION_TYPE_TAG,
};

/* Matches on selected register. */
struct mlx5_rte_flow_item_tag {
	uint16_t id;
	uint32_t data;
};

/* Modify selected register. */
struct mlx5_rte_flow_action_set_tag {
	uint16_t id;
	uint32_t data;
};

/* Matches on source queue. */
struct mlx5_rte_flow_item_tx_queue {
	uint32_t queue;
};

/* Pattern outer Layer bits. */
#define MLX5_FLOW_LAYER_OUTER_L2 (1u << 0)
#define MLX5_FLOW_LAYER_OUTER_L3_IPV4 (1u << 1)
#define MLX5_FLOW_LAYER_OUTER_L3_IPV6 (1u << 2)
#define MLX5_FLOW_LAYER_OUTER_L4_UDP (1u << 3)
#define MLX5_FLOW_LAYER_OUTER_L4_TCP (1u << 4)
#define MLX5_FLOW_LAYER_OUTER_VLAN (1u << 5)

/* Pattern inner Layer bits. */
#define MLX5_FLOW_LAYER_INNER_L2 (1u << 6)
#define MLX5_FLOW_LAYER_INNER_L3_IPV4 (1u << 7)
#define MLX5_FLOW_LAYER_INNER_L3_IPV6 (1u << 8)
#define MLX5_FLOW_LAYER_INNER_L4_UDP (1u << 9)
#define MLX5_FLOW_LAYER_INNER_L4_TCP (1u << 10)
#define MLX5_FLOW_LAYER_INNER_VLAN (1u << 11)

/* Pattern tunnel Layer bits. */
#define MLX5_FLOW_LAYER_VXLAN (1u << 12)
#define MLX5_FLOW_LAYER_VXLAN_GPE (1u << 13)
#define MLX5_FLOW_LAYER_GRE (1u << 14)
#define MLX5_FLOW_LAYER_MPLS (1u << 15)
/* List of tunnel Layer bits continued below. */

/* General pattern items bits. */
#define MLX5_FLOW_ITEM_METADATA (1u << 16)
#define MLX5_FLOW_ITEM_PORT_ID (1u << 17)
#define MLX5_FLOW_ITEM_TAG (1u << 18)

/* Pattern MISC bits. */
#define MLX5_FLOW_LAYER_ICMP (1u << 19)
#define MLX5_FLOW_LAYER_ICMP6 (1u << 20)
#define MLX5_FLOW_LAYER_GRE_KEY (1u << 21)

/* Pattern tunnel Layer bits (continued). */
#define MLX5_FLOW_LAYER_IPIP (1u << 21)
#define MLX5_FLOW_LAYER_IPV6_ENCAP (1u << 22)
#define MLX5_FLOW_LAYER_NVGRE (1u << 23)
#define MLX5_FLOW_LAYER_GENEVE (1u << 24)

/* Queue items. */
#define MLX5_FLOW_ITEM_TX_QUEUE (1u << 25)

/* Outer Masks. */
#define MLX5_FLOW_LAYER_OUTER_L3 \
	(MLX5_FLOW_LAYER_OUTER_L3_IPV4 | MLX5_FLOW_LAYER_OUTER_L3_IPV6)
#define MLX5_FLOW_LAYER_OUTER_L4 \
	(MLX5_FLOW_LAYER_OUTER_L4_UDP | MLX5_FLOW_LAYER_OUTER_L4_TCP)
#define MLX5_FLOW_LAYER_OUTER \
	(MLX5_FLOW_LAYER_OUTER_L2 | MLX5_FLOW_LAYER_OUTER_L3 | \
	 MLX5_FLOW_LAYER_OUTER_L4)

/* LRO support mask, i.e. flow contains IPv4/IPv6 and TCP. */
#define MLX5_FLOW_LAYER_IPV4_LRO \
	(MLX5_FLOW_LAYER_OUTER_L3_IPV4 | MLX5_FLOW_LAYER_OUTER_L4_TCP)
#define MLX5_FLOW_LAYER_IPV6_LRO \
	(MLX5_FLOW_LAYER_OUTER_L3_IPV6 | MLX5_FLOW_LAYER_OUTER_L4_TCP)

/* Tunnel Masks. */
#define MLX5_FLOW_LAYER_TUNNEL \
	(MLX5_FLOW_LAYER_VXLAN | MLX5_FLOW_LAYER_VXLAN_GPE | \
	 MLX5_FLOW_LAYER_GRE | MLX5_FLOW_LAYER_NVGRE | MLX5_FLOW_LAYER_MPLS | \
	 MLX5_FLOW_LAYER_IPIP | MLX5_FLOW_LAYER_IPV6_ENCAP | \
	 MLX5_FLOW_LAYER_GENEVE)

/* Inner Masks. */
#define MLX5_FLOW_LAYER_INNER_L3 \
	(MLX5_FLOW_LAYER_INNER_L3_IPV4 | MLX5_FLOW_LAYER_INNER_L3_IPV6)
#define MLX5_FLOW_LAYER_INNER_L4 \
	(MLX5_FLOW_LAYER_INNER_L4_UDP | MLX5_FLOW_LAYER_INNER_L4_TCP)
#define MLX5_FLOW_LAYER_INNER \
	(MLX5_FLOW_LAYER_INNER_L2 | MLX5_FLOW_LAYER_INNER_L3 | \
	 MLX5_FLOW_LAYER_INNER_L4)

/* Layer Masks. */
#define MLX5_FLOW_LAYER_L2 \
	(MLX5_FLOW_LAYER_OUTER_L2 | MLX5_FLOW_LAYER_INNER_L2)
#define MLX5_FLOW_LAYER_L3_IPV4 \
	(MLX5_FLOW_LAYER_OUTER_L3_IPV4 | MLX5_FLOW_LAYER_INNER_L3_IPV4)
#define MLX5_FLOW_LAYER_L3_IPV6 \
	(MLX5_FLOW_LAYER_OUTER_L3_IPV6 | MLX5_FLOW_LAYER_INNER_L3_IPV6)
#define MLX5_FLOW_LAYER_L3 \
	(MLX5_FLOW_LAYER_L3_IPV4 | MLX5_FLOW_LAYER_L3_IPV6)
#define MLX5_FLOW_LAYER_L4 \
	(MLX5_FLOW_LAYER_OUTER_L4 | MLX5_FLOW_LAYER_INNER_L4)

/* Actions */
#define MLX5_FLOW_ACTION_DROP (1u << 0)
#define MLX5_FLOW_ACTION_QUEUE (1u << 1)
#define MLX5_FLOW_ACTION_RSS (1u << 2)
#define MLX5_FLOW_ACTION_FLAG (1u << 3)
#define MLX5_FLOW_ACTION_MARK (1u << 4)
#define MLX5_FLOW_ACTION_COUNT (1u << 5)
#define MLX5_FLOW_ACTION_PORT_ID (1u << 6)
#define MLX5_FLOW_ACTION_OF_POP_VLAN (1u << 7)
#define MLX5_FLOW_ACTION_OF_PUSH_VLAN (1u << 8)
#define MLX5_FLOW_ACTION_OF_SET_VLAN_VID (1u << 9)
#define MLX5_FLOW_ACTION_OF_SET_VLAN_PCP (1u << 10)
#define MLX5_FLOW_ACTION_SET_IPV4_SRC (1u << 11)
#define MLX5_FLOW_ACTION_SET_IPV4_DST (1u << 12)
#define MLX5_FLOW_ACTION_SET_IPV6_SRC (1u << 13)
#define MLX5_FLOW_ACTION_SET_IPV6_DST (1u << 14)
#define MLX5_FLOW_ACTION_SET_TP_SRC (1u << 15)
#define MLX5_FLOW_ACTION_SET_TP_DST (1u << 16)
#define MLX5_FLOW_ACTION_JUMP (1u << 17)
#define MLX5_FLOW_ACTION_SET_TTL (1u << 18)
#define MLX5_FLOW_ACTION_DEC_TTL (1u << 19)
#define MLX5_FLOW_ACTION_SET_MAC_SRC (1u << 20)
#define MLX5_FLOW_ACTION_SET_MAC_DST (1u << 21)
#define MLX5_FLOW_ACTION_VXLAN_ENCAP (1u << 22)
#define MLX5_FLOW_ACTION_VXLAN_DECAP (1u << 23)
#define MLX5_FLOW_ACTION_NVGRE_ENCAP (1u << 24)
#define MLX5_FLOW_ACTION_NVGRE_DECAP (1u << 25)
#define MLX5_FLOW_ACTION_RAW_ENCAP (1u << 26)
#define MLX5_FLOW_ACTION_RAW_DECAP (1u << 27)
#define MLX5_FLOW_ACTION_INC_TCP_SEQ (1u << 28)
#define MLX5_FLOW_ACTION_DEC_TCP_SEQ (1u << 29)
#define MLX5_FLOW_ACTION_INC_TCP_ACK (1u << 30)
#define MLX5_FLOW_ACTION_DEC_TCP_ACK (1u << 31)
#define MLX5_FLOW_ACTION_SET_TAG (1ull << 32)

#define MLX5_FLOW_FATE_ACTIONS \
	(MLX5_FLOW_ACTION_DROP | MLX5_FLOW_ACTION_QUEUE | \
	 MLX5_FLOW_ACTION_RSS | MLX5_FLOW_ACTION_JUMP)

#define MLX5_FLOW_FATE_ESWITCH_ACTIONS \
	(MLX5_FLOW_ACTION_DROP | MLX5_FLOW_ACTION_PORT_ID | \
	 MLX5_FLOW_ACTION_JUMP)

#define MLX5_FLOW_ENCAP_ACTIONS	(MLX5_FLOW_ACTION_VXLAN_ENCAP | \
				 MLX5_FLOW_ACTION_NVGRE_ENCAP | \
				 MLX5_FLOW_ACTION_RAW_ENCAP | \
				 MLX5_FLOW_ACTION_OF_PUSH_VLAN)

#define MLX5_FLOW_DECAP_ACTIONS	(MLX5_FLOW_ACTION_VXLAN_DECAP | \
				 MLX5_FLOW_ACTION_NVGRE_DECAP | \
				 MLX5_FLOW_ACTION_RAW_DECAP | \
				 MLX5_FLOW_ACTION_OF_POP_VLAN)

#define MLX5_FLOW_MODIFY_HDR_ACTIONS (MLX5_FLOW_ACTION_SET_IPV4_SRC | \
				      MLX5_FLOW_ACTION_SET_IPV4_DST | \
				      MLX5_FLOW_ACTION_SET_IPV6_SRC | \
				      MLX5_FLOW_ACTION_SET_IPV6_DST | \
				      MLX5_FLOW_ACTION_SET_TP_SRC | \
				      MLX5_FLOW_ACTION_SET_TP_DST | \
				      MLX5_FLOW_ACTION_SET_TTL | \
				      MLX5_FLOW_ACTION_DEC_TTL | \
				      MLX5_FLOW_ACTION_SET_MAC_SRC | \
				      MLX5_FLOW_ACTION_SET_MAC_DST | \
				      MLX5_FLOW_ACTION_INC_TCP_SEQ | \
				      MLX5_FLOW_ACTION_DEC_TCP_SEQ | \
				      MLX5_FLOW_ACTION_INC_TCP_ACK | \
				      MLX5_FLOW_ACTION_DEC_TCP_ACK | \
				      MLX5_FLOW_ACTION_OF_SET_VLAN_VID | \
				      MLX5_FLOW_ACTION_SET_TAG)

#define MLX5_FLOW_VLAN_ACTIONS (MLX5_FLOW_ACTION_OF_POP_VLAN | \
				MLX5_FLOW_ACTION_OF_PUSH_VLAN)

#ifndef IPPROTO_MPLS
#define IPPROTO_MPLS 137
#endif

/* UDP port number for MPLS */
#define MLX5_UDP_PORT_MPLS 6635

/* UDP port numbers for VxLAN. */
#define MLX5_UDP_PORT_VXLAN 4789
#define MLX5_UDP_PORT_VXLAN_GPE 4790

/* UDP port numbers for GENEVE. */
#define MLX5_UDP_PORT_GENEVE 6081

/* Priority reserved for default flows. */
#define MLX5_FLOW_PRIO_RSVD ((uint32_t)-1)

/*
 * Number of sub priorities.
 * For each kind of pattern matching i.e. L2, L3, L4 to have a correct
 * matching on the NIC (firmware dependent) L4 most have the higher priority
 * followed by L3 and ending with L2.
 */
#define MLX5_PRIORITY_MAP_L2 2
#define MLX5_PRIORITY_MAP_L3 1
#define MLX5_PRIORITY_MAP_L4 0
#define MLX5_PRIORITY_MAP_MAX 3

/* Valid layer type for IPV4 RSS. */
#define MLX5_IPV4_LAYER_TYPES \
	(ETH_RSS_IPV4 | ETH_RSS_FRAG_IPV4 | \
	 ETH_RSS_NONFRAG_IPV4_TCP | ETH_RSS_NONFRAG_IPV4_UDP | \
	 ETH_RSS_NONFRAG_IPV4_OTHER)

/* IBV hash source bits  for IPV4. */
#define MLX5_IPV4_IBV_RX_HASH (IBV_RX_HASH_SRC_IPV4 | IBV_RX_HASH_DST_IPV4)

/* Valid layer type for IPV6 RSS. */
#define MLX5_IPV6_LAYER_TYPES \
	(ETH_RSS_IPV6 | ETH_RSS_FRAG_IPV6 | ETH_RSS_NONFRAG_IPV6_TCP | \
	 ETH_RSS_NONFRAG_IPV6_UDP | ETH_RSS_IPV6_EX  | ETH_RSS_IPV6_TCP_EX | \
	 ETH_RSS_IPV6_UDP_EX | ETH_RSS_NONFRAG_IPV6_OTHER)

/* IBV hash source bits  for IPV6. */
#define MLX5_IPV6_IBV_RX_HASH (IBV_RX_HASH_SRC_IPV6 | IBV_RX_HASH_DST_IPV6)


/* Geneve header first 16Bit */
#define MLX5_GENEVE_VER_MASK 0x3
#define MLX5_GENEVE_VER_SHIFT 14
#define MLX5_GENEVE_VER_VAL(a) \
		(((a) >> (MLX5_GENEVE_VER_SHIFT)) & (MLX5_GENEVE_VER_MASK))
#define MLX5_GENEVE_OPTLEN_MASK 0x3F
#define MLX5_GENEVE_OPTLEN_SHIFT 7
#define MLX5_GENEVE_OPTLEN_VAL(a) \
	    (((a) >> (MLX5_GENEVE_OPTLEN_SHIFT)) & (MLX5_GENEVE_OPTLEN_MASK))
#define MLX5_GENEVE_OAMF_MASK 0x1
#define MLX5_GENEVE_OAMF_SHIFT 7
#define MLX5_GENEVE_OAMF_VAL(a) \
		(((a) >> (MLX5_GENEVE_OAMF_SHIFT)) & (MLX5_GENEVE_OAMF_MASK))
#define MLX5_GENEVE_CRITO_MASK 0x1
#define MLX5_GENEVE_CRITO_SHIFT 6
#define MLX5_GENEVE_CRITO_VAL(a) \
		(((a) >> (MLX5_GENEVE_CRITO_SHIFT)) & (MLX5_GENEVE_CRITO_MASK))
#define MLX5_GENEVE_RSVD_MASK 0x3F
#define MLX5_GENEVE_RSVD_VAL(a) ((a) & (MLX5_GENEVE_RSVD_MASK))
/*
 * The length of the Geneve options fields, expressed in four byte multiples,
 * not including the eight byte fixed tunnel.
 */
#define MLX5_GENEVE_OPT_LEN_0 14
#define MLX5_GENEVE_OPT_LEN_1 63

enum mlx5_flow_drv_type {
	MLX5_FLOW_TYPE_MIN,
	MLX5_FLOW_TYPE_DV,
	MLX5_FLOW_TYPE_VERBS,
	MLX5_FLOW_TYPE_MAX,
};

/* Matcher PRM representation */
struct mlx5_flow_dv_match_params {
	size_t size;
	/**< Size of match value. Do NOT split size and key! */
	uint32_t buf[MLX5_ST_SZ_DW(fte_match_param)];
	/**< Matcher value. This value is used as the mask or as a key. */
};

/* Matcher structure. */
struct mlx5_flow_dv_matcher {
	LIST_ENTRY(mlx5_flow_dv_matcher) next;
	/* Pointer to the next element. */
	rte_atomic32_t refcnt; /**< Reference counter. */
	void *matcher_object; /**< Pointer to DV matcher */
	uint16_t crc; /**< CRC of key. */
	uint16_t priority; /**< Priority of matcher. */
	uint8_t egress; /**< Egress matcher. */
	uint8_t transfer; /**< 1 if the flow is E-Switch flow. */
	uint32_t group; /**< The matcher group. */
	struct mlx5_flow_dv_match_params mask; /**< Matcher mask. */
};

#define MLX5_ENCAP_MAX_LEN 132

/* Encap/decap resource structure. */
struct mlx5_flow_dv_encap_decap_resource {
	LIST_ENTRY(mlx5_flow_dv_encap_decap_resource) next;
	/* Pointer to next element. */
	rte_atomic32_t refcnt; /**< Reference counter. */
	void *verbs_action;
	/**< Verbs encap/decap action object. */
	uint8_t buf[MLX5_ENCAP_MAX_LEN];
	size_t size;
	uint8_t reformat_type;
	uint8_t ft_type;
	uint64_t flags; /**< Flags for RDMA API. */
};

/* Tag resource structure. */
struct mlx5_flow_dv_tag_resource {
	LIST_ENTRY(mlx5_flow_dv_tag_resource) next;
	/* Pointer to next element. */
	rte_atomic32_t refcnt; /**< Reference counter. */
	void *action;
	/**< Verbs tag action object. */
	uint32_t tag; /**< the tag value. */
};

/* Number of modification commands. */
#define MLX5_MODIFY_NUM 8

/* Modify resource structure */
struct mlx5_flow_dv_modify_hdr_resource {
	LIST_ENTRY(mlx5_flow_dv_modify_hdr_resource) next;
	/* Pointer to next element. */
	rte_atomic32_t refcnt; /**< Reference counter. */
	struct ibv_flow_action *verbs_action;
	/**< Verbs modify header action object. */
	uint8_t ft_type; /**< Flow table type, Rx or Tx. */
	uint32_t actions_num; /**< Number of modification actions. */
	struct mlx5_modification_cmd actions[MLX5_MODIFY_NUM];
	/**< Modification actions. */
	uint64_t flags; /**< Flags for RDMA API. */
};

/* Jump action resource structure. */
struct mlx5_flow_dv_jump_tbl_resource {
	LIST_ENTRY(mlx5_flow_dv_jump_tbl_resource) next;
	/* Pointer to next element. */
	rte_atomic32_t refcnt; /**< Reference counter. */
	void *action; /**< Pointer to the rdma core action. */
	uint8_t ft_type; /**< Flow table type, Rx or Tx. */
	struct mlx5_flow_tbl_resource *tbl; /**< The target table. */
};

/* Port ID resource structure. */
struct mlx5_flow_dv_port_id_action_resource {
	LIST_ENTRY(mlx5_flow_dv_port_id_action_resource) next;
	/* Pointer to next element. */
	rte_atomic32_t refcnt; /**< Reference counter. */
	void *action;
	/**< Verbs tag action object. */
	uint32_t port_id; /**< Port ID value. */
};

/* Push VLAN action resource structure */
struct mlx5_flow_dv_push_vlan_action_resource {
	LIST_ENTRY(mlx5_flow_dv_push_vlan_action_resource) next;
	/* Pointer to next element. */
	rte_atomic32_t refcnt; /**< Reference counter. */
	void *action; /**< Direct verbs action object. */
	uint8_t ft_type; /**< Flow table type, Rx, Tx or FDB. */
	rte_be32_t vlan_tag; /**< VLAN tag value. */
};

/*
 * Max number of actions per DV flow.
 * See CREATE_FLOW_MAX_FLOW_ACTIONS_SUPPORTED
 * In rdma-core file providers/mlx5/verbs.c
 */
#define MLX5_DV_MAX_NUMBER_OF_ACTIONS 8

/* DV flows structure. */
struct mlx5_flow_dv {
	uint64_t hash_fields; /**< Fields that participate in the hash. */
	struct mlx5_hrxq *hrxq; /**< Hash Rx queues. */
	/* Flow DV api: */
	struct mlx5_flow_dv_matcher *matcher; /**< Cache to matcher. */
	struct mlx5_flow_dv_match_params value;
	/**< Holds the value that the packet is compared to. */
	struct mlx5_flow_dv_encap_decap_resource *encap_decap;
	/**< Pointer to encap/decap resource in cache. */
	struct mlx5_flow_dv_modify_hdr_resource *modify_hdr;
	/**< Pointer to modify header resource in cache. */
	struct ibv_flow *flow; /**< Installed flow. */
	struct mlx5_flow_dv_jump_tbl_resource *jump;
	/**< Pointer to the jump action resource. */
	struct mlx5_flow_dv_port_id_action_resource *port_id_action;
	/**< Pointer to port ID action resource. */
	struct mlx5_vf_vlan vf_vlan;
	/**< Structure for VF VLAN workaround. */
	struct mlx5_flow_dv_push_vlan_action_resource *push_vlan_res;
	/**< Pointer to push VLAN action resource in cache. */
#ifdef HAVE_IBV_FLOW_DV_SUPPORT
	void *actions[MLX5_DV_MAX_NUMBER_OF_ACTIONS];
	/**< Action list. */
#endif
	int actions_n; /**< number of actions. */
};

/* Verbs specification header. */
struct ibv_spec_header {
	enum ibv_flow_spec_type type;
	uint16_t size;
};

/** Handles information leading to a drop fate. */
struct mlx5_flow_verbs {
	LIST_ENTRY(mlx5_flow_verbs) next;
	unsigned int size; /**< Size of the attribute. */
	struct {
		struct ibv_flow_attr *attr;
		/**< Pointer to the Specification buffer. */
		uint8_t *specs; /**< Pointer to the specifications. */
	};
	struct ibv_flow *flow; /**< Verbs flow pointer. */
	struct mlx5_hrxq *hrxq; /**< Hash Rx queue object. */
	uint64_t hash_fields; /**< Verbs hash Rx queue hash fields. */
	struct mlx5_vf_vlan vf_vlan;
	/**< Structure for VF VLAN workaround. */
};

/** Device flow structure. */
struct mlx5_flow {
	LIST_ENTRY(mlx5_flow) next;
	struct rte_flow *flow; /**< Pointer to the main flow. */
	uint64_t layers;
	/**< Bit-fields of present layers, see MLX5_FLOW_LAYER_*. */
	uint64_t actions;
	/**< Bit-fields of detected actions, see MLX5_FLOW_ACTION_*. */
	union {
#ifdef HAVE_IBV_FLOW_DV_SUPPORT
		struct mlx5_flow_dv dv;
#endif
		struct mlx5_flow_verbs verbs;
	};
	bool external; /**< true if the flow is created external to PMD. */
};

/* Flow structure. */
struct rte_flow {
	TAILQ_ENTRY(rte_flow) next; /**< Pointer to the next flow structure. */
	enum mlx5_flow_drv_type drv_type; /**< Driver type. */
	struct mlx5_flow_counter *counter; /**< Holds flow counter. */
	struct mlx5_flow_dv_tag_resource *tag_resource;
	/**< pointer to the tag action. */
	struct rte_flow_action_rss rss;/**< RSS context. */
	uint8_t key[MLX5_RSS_HASH_KEY_LEN]; /**< RSS hash key. */
	uint16_t (*queue)[]; /**< Destination queues to redirect traffic to. */
	LIST_HEAD(dev_flows, mlx5_flow) dev_flows;
	/**< Device flows that are part of the flow. */
	struct mlx5_fdir *fdir; /**< Pointer to associated FDIR if any. */
	uint8_t ingress; /**< 1 if the flow is ingress. */
	uint32_t group; /**< The group index. */
	uint8_t transfer; /**< 1 if the flow is E-Switch flow. */
	uint32_t hairpin_flow_id; /**< The flow id used for hairpin. */
};

typedef int (*mlx5_flow_validate_t)(struct rte_eth_dev *dev,
				    const struct rte_flow_attr *attr,
				    const struct rte_flow_item items[],
				    const struct rte_flow_action actions[],
				    bool external,
				    struct rte_flow_error *error);
typedef struct mlx5_flow *(*mlx5_flow_prepare_t)
	(const struct rte_flow_attr *attr, const struct rte_flow_item items[],
	 const struct rte_flow_action actions[], struct rte_flow_error *error);
typedef int (*mlx5_flow_translate_t)(struct rte_eth_dev *dev,
				     struct mlx5_flow *dev_flow,
				     const struct rte_flow_attr *attr,
				     const struct rte_flow_item items[],
				     const struct rte_flow_action actions[],
				     struct rte_flow_error *error);
typedef int (*mlx5_flow_apply_t)(struct rte_eth_dev *dev, struct rte_flow *flow,
				 struct rte_flow_error *error);
typedef void (*mlx5_flow_remove_t)(struct rte_eth_dev *dev,
				   struct rte_flow *flow);
typedef void (*mlx5_flow_destroy_t)(struct rte_eth_dev *dev,
				    struct rte_flow *flow);
typedef int (*mlx5_flow_query_t)(struct rte_eth_dev *dev,
				 struct rte_flow *flow,
				 const struct rte_flow_action *actions,
				 void *data,
				 struct rte_flow_error *error);
struct mlx5_flow_driver_ops {
	mlx5_flow_validate_t validate;
	mlx5_flow_prepare_t prepare;
	mlx5_flow_translate_t translate;
	mlx5_flow_apply_t apply;
	mlx5_flow_remove_t remove;
	mlx5_flow_destroy_t destroy;
	mlx5_flow_query_t query;
};

#define MLX5_CNT_CONTAINER(sh, batch, thread) (&(sh)->cmng.ccont \
	[(((sh)->cmng.mhi[batch] >> (thread)) & 0x1) * 2 + (batch)])
#define MLX5_CNT_CONTAINER_UNUSED(sh, batch, thread) (&(sh)->cmng.ccont \
	[(~((sh)->cmng.mhi[batch] >> (thread)) & 0x1) * 2 + (batch)])

/* mlx5_flow.c */

struct mlx5_flow_id_pool *mlx5_flow_id_pool_alloc(void);
void mlx5_flow_id_pool_release(struct mlx5_flow_id_pool *pool);
uint32_t mlx5_flow_id_get(struct mlx5_flow_id_pool *pool, uint32_t *id);
uint32_t mlx5_flow_id_release(struct mlx5_flow_id_pool *pool,
			      uint32_t id);
int mlx5_flow_group_to_table(const struct rte_flow_attr *attributes,
			     bool external, uint32_t group, uint32_t *table,
			     struct rte_flow_error *error);
uint64_t mlx5_flow_hashfields_adjust(struct mlx5_flow *dev_flow, int tunnel,
				     uint64_t layer_types,
				     uint64_t hash_fields);
uint32_t mlx5_flow_adjust_priority(struct rte_eth_dev *dev, int32_t priority,
				   uint32_t subpriority);
const struct rte_flow_action *mlx5_flow_find_action
					(const struct rte_flow_action *actions,
					 enum rte_flow_action_type action);
int mlx5_flow_validate_action_count(struct rte_eth_dev *dev,
				    const struct rte_flow_attr *attr,
				    struct rte_flow_error *error);
int mlx5_flow_validate_action_drop(uint64_t action_flags,
				   const struct rte_flow_attr *attr,
				   struct rte_flow_error *error);
int mlx5_flow_validate_action_flag(uint64_t action_flags,
				   const struct rte_flow_attr *attr,
				   struct rte_flow_error *error);
int mlx5_flow_validate_action_mark(const struct rte_flow_action *action,
				   uint64_t action_flags,
				   const struct rte_flow_attr *attr,
				   struct rte_flow_error *error);
int mlx5_flow_validate_action_queue(const struct rte_flow_action *action,
				    uint64_t action_flags,
				    struct rte_eth_dev *dev,
				    const struct rte_flow_attr *attr,
				    struct rte_flow_error *error);
int mlx5_flow_validate_action_rss(const struct rte_flow_action *action,
				  uint64_t action_flags,
				  struct rte_eth_dev *dev,
				  const struct rte_flow_attr *attr,
				  uint64_t item_flags,
				  struct rte_flow_error *error);
int mlx5_flow_validate_attributes(struct rte_eth_dev *dev,
				  const struct rte_flow_attr *attributes,
				  struct rte_flow_error *error);
int mlx5_flow_item_acceptable(const struct rte_flow_item *item,
			      const uint8_t *mask,
			      const uint8_t *nic_mask,
			      unsigned int size,
			      struct rte_flow_error *error);
int mlx5_flow_validate_item_eth(const struct rte_flow_item *item,
				uint64_t item_flags,
				struct rte_flow_error *error);
int mlx5_flow_validate_item_gre(const struct rte_flow_item *item,
				uint64_t item_flags,
				uint8_t target_protocol,
				struct rte_flow_error *error);
int mlx5_flow_validate_item_gre_key(const struct rte_flow_item *item,
				    uint64_t item_flags,
				    const struct rte_flow_item *gre_item,
				    struct rte_flow_error *error);
int mlx5_flow_validate_item_ipv4(const struct rte_flow_item *item,
				 uint64_t item_flags,
				 uint64_t last_item,
				 uint16_t ether_type,
				 const struct rte_flow_item_ipv4 *acc_mask,
				 struct rte_flow_error *error);
int mlx5_flow_validate_item_ipv6(const struct rte_flow_item *item,
				 uint64_t item_flags,
				 uint64_t last_item,
				 uint16_t ether_type,
				 const struct rte_flow_item_ipv6 *acc_mask,
				 struct rte_flow_error *error);
int mlx5_flow_validate_item_mpls(struct rte_eth_dev *dev,
				 const struct rte_flow_item *item,
				 uint64_t item_flags,
				 uint64_t prev_layer,
				 struct rte_flow_error *error);
int mlx5_flow_validate_item_tcp(const struct rte_flow_item *item,
				uint64_t item_flags,
				uint8_t target_protocol,
				const struct rte_flow_item_tcp *flow_mask,
				struct rte_flow_error *error);
int mlx5_flow_validate_item_udp(const struct rte_flow_item *item,
				uint64_t item_flags,
				uint8_t target_protocol,
				struct rte_flow_error *error);
int mlx5_flow_validate_item_vlan(const struct rte_flow_item *item,
				 uint64_t item_flags,
				 struct rte_eth_dev *dev,
				 struct rte_flow_error *error);
int mlx5_flow_validate_item_vxlan(const struct rte_flow_item *item,
				  uint64_t item_flags,
				  struct rte_flow_error *error);
int mlx5_flow_validate_item_vxlan_gpe(const struct rte_flow_item *item,
				      uint64_t item_flags,
				      struct rte_eth_dev *dev,
				      struct rte_flow_error *error);
int mlx5_flow_validate_item_icmp(const struct rte_flow_item *item,
				 uint64_t item_flags,
				 uint8_t target_protocol,
				 struct rte_flow_error *error);
int mlx5_flow_validate_item_icmp6(const struct rte_flow_item *item,
				   uint64_t item_flags,
				   uint8_t target_protocol,
				   struct rte_flow_error *error);
int mlx5_flow_validate_item_nvgre(const struct rte_flow_item *item,
				  uint64_t item_flags,
				  uint8_t target_protocol,
				  struct rte_flow_error *error);
int mlx5_flow_validate_item_geneve(const struct rte_flow_item *item,
				   uint64_t item_flags,
				   struct rte_eth_dev *dev,
				   struct rte_flow_error *error);
#endif /* RTE_PMD_MLX5_FLOW_H_ */
