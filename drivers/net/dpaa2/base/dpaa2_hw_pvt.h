/*-
 *   BSD LICENSE
 *
 *   Copyright (c) 2016 Freescale Semiconductor, Inc. All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Freescale Semiconductor, Inc nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _DPAA2_HW_PVT_H_
#define _DPAA2_HW_PVT_H_

#include <rte_memory.h>
#include <rte_atomic.h>

/* Core implementation files */
#include <fsl_dpbp.h>
#include <fsl_dpni.h>
#include <fsl_dpio.h>
#include <fsl_vfio.h>
#include <fsl_mc_sys.h>
#include <fsl_qbman_portal.h>

#define FALSE               0
#define TRUE                1
#ifndef false
#define false      FALSE
#endif
#ifndef true
#define true       TRUE
#endif
#define lower_32_bits(x) ((uint32_t)(x))
#define upper_32_bits(x) ((uint32_t)(((x) >> 16) >> 16))

#ifndef ETH_ADDR_LEN
#define ETH_ADDR_LEN			6
#endif
#ifndef ETH_VLAN_HLEN
#define ETH_VLAN_HLEN   4 /** < Vlan Header Length */
#endif

#define MAX_TX_RING_SLOTS	8
	/** <Maximum number of slots available in TX ring*/

#define DPAA2_DQRR_RING_SIZE	16
	/** <Maximum number of slots available in RX ring*/

#define MEMPOOL_F_HW_PKT_POOL 0x8000 /**< mpool flag to check offloaded pool */

#define MC_PORTAL_INDEX		0
#define NUM_DPIO_REGIONS	2
#define NUM_DQS_PER_QUEUE       2
#define MC_PORTALS_BASE_PADDR   0x00080C000000ULL
#define MC_PORTAL_STRIDE        0x10000
#define MC_PORTAL_SIZE		64
#define MC_PORTAL_ID_TO_PADDR(portal_id) \
(MC_PORTALS_BASE_PADDR + (portal_id) * MC_PORTAL_STRIDE)

#define DPAA2_MBUF_HW_ANNOTATION	64
#define DPAA2_FD_PTA_SIZE		64

#if (DPAA2_MBUF_HW_ANNOTATION + DPAA2_FD_PTA_SIZE) > RTE_PKTMBUF_HEADROOM
#error "Annotation requirement is more than RTE_PKTMBUF_HEADROOM"
#endif

/* we will re-use the HEADROOM for annotation in RX */
#define DPAA2_HW_BUF_RESERVE	0
#define DPAA2_PACKET_LAYOUT_ALIGN	64 /*changing from 256 */

struct dpaa2_dpio_dev {
	TAILQ_ENTRY(dpaa2_dpio_dev) next;
		/**< Pointer to Next device instance */
	uint16_t index; /**< Index of a instance in the list */
	rte_atomic16_t ref_count;
		/**< How many thread contexts are sharing this.*/
	struct fsl_mc_io *dpio; /** handle to DPIO portal object */
	uint16_t token;
	struct qbman_swp *sw_portal; /** SW portal object */
	const struct qbman_result *dqrr[4];
		/**< DQRR Entry for this SW portal */
	pthread_mutex_t lock; /** Required when Portal is shared */
	void *mc_portal; /**< MC Portal for configuring this device */
	uintptr_t qbman_portal_ce_paddr;
		/**< Physical address of Cache Enabled Area */
	uintptr_t ce_size; /**< Size of the CE region */
	uintptr_t qbman_portal_ci_paddr;
		/**< Physical address of Cache Inhibit Area */
	uintptr_t ci_size; /**< Size of the CI region */
	void *intr_handle;
	int32_t	vfio_fd; /**< File descriptor received via VFIO */
	int32_t hw_id; /**< An unique ID of this DPIO device instance */
};

struct queue_storage_info_t {
	struct qbman_result *dq_storage[NUM_DQS_PER_QUEUE];
	struct qbman_result *active_dqs;
	int active_dpio_id;
	int toggle;
};

struct dpaa2_queue {
	struct rte_mempool *mb_pool; /**< mbuf pool to populate RX ring. */
	void *dev;
	int32_t eventfd;	/*!< Event Fd of this queue */
	uint32_t fqid;		/*!< Unique ID of this queue */
	uint8_t tc_index;	/*!< traffic class identifier */
	uint16_t flow_id;	/*!< To be used by DPAA2 frmework */
	uint64_t rx_pkts;
	uint64_t tx_pkts;
	uint64_t err_pkts;
	union {
		struct queue_storage_info_t *q_storage;
		struct qbman_result *cscn;
	};
};

struct dpaa2_io_portal_t {
	struct dpaa2_dpio_dev *dpio_dev;
	struct dpaa2_dpio_dev *sec_dpio_dev;
	uint64_t net_tid;
	uint64_t sec_tid;
};

/*! Global per thread DPIO portal */
RTE_DECLARE_PER_LCORE(struct dpaa2_io_portal_t, _dpaa2_io);

#define DPAA2_PER_LCORE_DPIO RTE_PER_LCORE(_dpaa2_io).dpio_dev
#define DPAA2_PER_LCORE_PORTAL DPAA2_PER_LCORE_DPIO->sw_portal

#define DPAA2_PER_LCORE_SEC_DPIO RTE_PER_LCORE(_dpaa2_io).sec_dpio_dev
#define DPAA2_PER_LCORE_SEC_PORTAL DPAA2_PER_LCORE_SEC_DPIO->sw_portal

struct swp_active_dqs {
	struct qbman_result *global_active_dqs;
	uint64_t reserved[7];
};

#define NUM_MAX_SWP 64

extern struct swp_active_dqs global_active_dqs_list[NUM_MAX_SWP];

/*! Global MCP list */
extern void *(*mcp_ptr_list);

/* Refer to Table 7-3 in SEC BG */
struct qbman_fle {
	uint32_t addr_lo;
	uint32_t addr_hi;
	uint32_t length;
	/* FMT must be 00, MSB is final bit  */
	uint32_t fin_bpid_offset;
	uint32_t frc;
	uint32_t reserved[3]; /* Not used currently */
};

struct qbman_sge {
	uint32_t addr_lo;
	uint32_t addr_hi;
	uint32_t length;
	uint32_t fin_bpid_offset;
};

/* There are three types of frames: Single, Scatter Gather and Frame Lists */
enum qbman_fd_format {
	qbman_fd_single = 0,
	qbman_fd_list,
	qbman_fd_sg
};

/* Maximum release/acquire from QBMAN */
#define DPAA2_MBUF_MAX_ACQ_REL	7

#define MAX_BPID 256

/*Macros to define operations on FD*/
#define DPAA2_SET_FD_ADDR(fd, addr) do {			\
	fd->simple.addr_lo = lower_32_bits((uint64_t)addr);	\
	fd->simple.addr_hi = upper_32_bits((uint64_t)addr);	\
} while (0)
#define DPAA2_SET_FD_LEN(fd, length)	fd->simple.len = length
#define DPAA2_SET_FD_BPID(fd, bpid)	(fd->simple.bpid_offset |= bpid)
#define DPAA2_SET_FD_IVP(fd)   ((fd->simple.bpid_offset |= 0x00004000))
#define DPAA2_SET_FD_OFFSET(fd, offset)	\
	((fd->simple.bpid_offset |= (uint32_t)(offset) << 16))
#define DPAA2_SET_FD_INTERNAL_JD(fd, len) fd->simple.frc = (0x80000000 | (len))
#define DPAA2_SET_FD_FRC(fd, frc)	fd->simple.frc = frc
#define DPAA2_RESET_FD_CTRL(fd)	fd->simple.ctrl = 0

#define	DPAA2_SET_FD_ASAL(fd, asal)	(fd->simple.ctrl |= (asal << 16))
#define DPAA2_SET_FD_FLC(fd, addr)	do { \
	fd->simple.flc_lo = lower_32_bits((uint64_t)addr);	\
	fd->simple.flc_hi = upper_32_bits((uint64_t)addr);	\
} while (0)
#define DPAA2_SET_FLE_INTERNAL_JD(fle, len) (fle->frc = (0x80000000 | (len)))
#define DPAA2_GET_FLE_ADDR(fle)					\
	(uint64_t)((((uint64_t)(fle->addr_hi)) << 32) + fle->addr_lo)
#define DPAA2_SET_FLE_ADDR(fle, addr) do { \
	fle->addr_lo = lower_32_bits((uint64_t)addr);     \
	fle->addr_hi = upper_32_bits((uint64_t)addr);	  \
} while (0)
#define DPAA2_SET_FLE_OFFSET(fle, offset) \
	((fle)->fin_bpid_offset |= (uint32_t)(offset) << 16)
#define DPAA2_SET_FLE_BPID(fle, bpid) ((fle)->fin_bpid_offset |= (uint64_t)bpid)
#define DPAA2_GET_FLE_BPID(fle) ((fle)->fin_bpid_offset & 0x000000ff)
#define DPAA2_SET_FLE_FIN(fle)	((fle)->fin_bpid_offset |= (uint64_t)1 << 31)
#define DPAA2_SET_FLE_IVP(fle)   (((fle)->fin_bpid_offset |= 0x00004000))
#define DPAA2_SET_FD_COMPOUND_FMT(fd)	\
	((fd)->simple.bpid_offset |= (uint32_t)1 << 28)
#define DPAA2_GET_FD_ADDR(fd)	\
((uint64_t)((((uint64_t)((fd)->simple.addr_hi)) << 32) + (fd)->simple.addr_lo))

#define DPAA2_GET_FD_LEN(fd)	((fd)->simple.len)
#define DPAA2_GET_FD_BPID(fd)	((fd)->simple.bpid_offset & 0x00003FFF)
#define DPAA2_GET_FD_IVP(fd)   (((fd)->simple.bpid_offset & 0x00004000) >> 14)
#define DPAA2_GET_FD_OFFSET(fd)	((fd->simple.bpid_offset & 0x0FFF0000) >> 16)
#define DPAA2_GET_FD_FRC(fd)	((fd)->simple.frc)
#define DPAA2_GET_FD_FLC(fd)	\
((uint64_t)((((uint64_t)((fd)->simple.flc_hi)) << 32) + (fd)->simple.flc_lo))

#define DPAA2_GET_FLE_OFFSET(fle) (((fle)->fin_bpid_offset & 0x0FFF0000) >> 16)
#define DPAA2_SET_FLE_SG_EXT(fle) ((fle)->fin_bpid_offset |= (uint64_t)1 << 29)
#define DPAA2_IS_SET_FLE_SG_EXT(fle)	\
	(((fle)->fin_bpid_offset & ((uint64_t)1 << 29)) ? 1 : 0)

#define DPAA2_INLINE_MBUF_FROM_BUF(buf, meta_data_size) \
	((struct rte_mbuf *)((uint64_t)buf - meta_data_size))
#define DPAA2_BUF_FROM_INLINE_MBUF(mbuf, meta_data_size) \
	((uint8_t *)((uint64_t)mbuf + meta_data_size))

#define DPAA2_ASAL_VAL (DPAA2_MBUF_HW_ANNOTATION / 64)

#define DPAA2_FD_SET_FORMAT(fd, format)	do {				\
		(fd)->simple.bpid_offset &= 0xCFFFFFFF;			\
		(fd)->simple.bpid_offset |= (uint32_t)format << 28;	\
} while (0)
#define DPAA2_FD_GET_FORMAT(fd)	(((fd)->simple.bpid_offset >> 28) & 0x3)

#define DPAA2_SG_SET_FINAL(sg, fin)	do {				\
		(sg)->fin_bpid_offset &= 0x7FFFFFFF;			\
		(sg)->fin_bpid_offset |= (uint32_t)fin << 31;		\
} while (0)
#define DPAA2_SG_IS_FINAL(sg) (!!((sg)->fin_bpid_offset >> 31))

/*Macros to define QBMAN enqueue options */
#define DPAA2_ETH_EQ_DISABLE		0	/*!< Dont Enqueue the Frame */
#define DPAA2_ETH_EQ_RESP_ON_SUCC	1	/*!< Enqueue the Frame with
						 * response after success
						 */
#define DPAA2_ETH_EQ_RESP_ON_FAIL	2	/*!< Enqueue the Frame with
						 * response after failure
						 */
#define DPAA2_ETH_EQ_NO_RESP		3	/*!< Enqueue the Frame without
						 * response
						 */
/* Only Enqueue Error responses will be
 * pushed on FQID_ERR of Enqueue FQ
 */
#define DPAA2_EQ_RESP_ERR_FQ		0
/* All Enqueue responses will be pushed on address
 * set with qbman_eq_desc_set_response
 */
#define DPAA2_EQ_RESP_ALWAYS		1

int
dpaa2_alloc_dq_storage(struct queue_storage_info_t *q_storage);

void
dpaa2_free_dq_storage(struct queue_storage_info_t *q_storage);

#ifdef RTE_LIBRTE_DPAA2_USE_PHYS_IOVA
/* todo - this is costly, need to write a fast coversion routine */
static void *dpaa2_mem_ptov(phys_addr_t paddr)
{
	const struct rte_memseg *memseg = rte_eal_get_physmem_layout();
	int i;

	for (i = 0; i < RTE_MAX_MEMSEG && memseg[i].addr_64 != 0; i++) {
		if (paddr >= memseg[i].phys_addr &&
		   (char *)paddr < (char *)memseg[i].phys_addr + memseg[i].len)
			return (void *)(memseg[i].addr_64
				+ (paddr - memseg[i].phys_addr));
	}
	return NULL;
}

static phys_addr_t dpaa2_mem_vtop(uint64_t vaddr)
{
	const struct rte_memseg *memseg = rte_eal_get_physmem_layout();
	int i;

	for (i = 0; i < RTE_MAX_MEMSEG && memseg[i].addr_64 != 0; i++) {
		if (vaddr >= memseg[i].addr_64 &&
		    vaddr < memseg[i].addr_64 + memseg[i].len)
			return memseg[i].phys_addr
				+ (vaddr - memseg[i].addr_64);
	}
	return (phys_addr_t)(NULL);
}

/**
 * When we are using Physical addresses as IO Virtual Addresses,
 * we call conversion routines nadk_mem_vtop & nadk_mem_ptov wherever required.
 * These routines are called with help of below MACRO's
 */

#define DPAA2_MBUF_VADDR_TO_IOVA(mbuf) (mbuf->buf_physaddr)
#define DPAA2_OP_VADDR_TO_IOVA(op) (op->phys_addr)

/**
 * macro to convert Virtual address to IOVA
 */
#define DPAA2_VADDR_TO_IOVA(_vaddr) dpaa2_mem_vtop((uint64_t)(_vaddr))

/**
 * macro to convert IOVA to Virtual address
 */
#define DPAA2_IOVA_TO_VADDR(_iova) dpaa2_mem_ptov((phys_addr_t)(_iova))

/**
 * macro to convert modify the memory containing Virtual address to IOVA
 */
#define DPAA2_MODIFY_VADDR_TO_IOVA(_mem, _type) \
	{_mem = (_type)(dpaa2_mem_vtop((uint64_t)(_mem))); }

/**
 * macro to convert modify the memory containing IOVA to Virtual address
 */
#define DPAA2_MODIFY_IOVA_TO_VADDR(_mem, _type) \
	{_mem = (_type)(dpaa2_mem_ptov((phys_addr_t)(_mem))); }

#else	/* RTE_LIBRTE_DPAA2_USE_PHYS_IOVA */

#define DPAA2_MBUF_VADDR_TO_IOVA(mbuf) (mbuf->buf_addr)
#define DPAA2_OP_VADDR_TO_IOVA(op) (op)

#define DPAA2_VADDR_TO_IOVA(_vaddr) (_vaddr)
#define DPAA2_IOVA_TO_VADDR(_iova) (_iova)
#define DPAA2_MODIFY_VADDR_TO_IOVA(_mem, _type)
#define DPAA2_MODIFY_IOVA_TO_VADDR(_mem, _type)

#endif /* RTE_LIBRTE_DPAA2_USE_PHYS_IOVA */

static inline
int check_swp_active_dqs(uint16_t dpio_dev_index)
{
	if (global_active_dqs_list[dpio_dev_index].global_active_dqs != NULL)
		return 1;
	return 0;
}

static inline
void clear_swp_active_dqs(uint16_t dpio_dev_index)
{
	global_active_dqs_list[dpio_dev_index].global_active_dqs = NULL;
}

static inline
struct qbman_result *get_swp_active_dqs(uint16_t dpio_dev_index)
{
	return global_active_dqs_list[dpio_dev_index].global_active_dqs;
}

static inline
void set_swp_active_dqs(uint16_t dpio_dev_index, struct qbman_result *dqs)
{
	global_active_dqs_list[dpio_dev_index].global_active_dqs = dqs;
}

#endif
