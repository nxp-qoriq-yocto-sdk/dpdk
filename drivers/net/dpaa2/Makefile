#   BSD LICENSE
#
#   Copyright (c) 2016 Freescale Semiconductor, Inc. All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#     * Neither the name of Freescale Semiconductor, Inc nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

include $(RTE_SDK)/mk/rte.vars.mk
RTE_SDK_DPAA2=$(RTE_SDK)/drivers/common/dpaa2

#
# library name
#
LIB = librte_pmd_dpaa2.a

ifeq ($(CONFIG_RTE_LIBRTE_DPAA2_DEBUG_INIT),y)
CFLAGS += -O0 -g
CFLAGS += "-Wno-error"
else
CFLAGS += -O3
CFLAGS += $(WERROR_FLAGS)
endif
CFLAGS +=-Wno-strict-aliasing
CFLAGS +=-Wno-unused-function

CFLAGS += -I$(RTE_SDK)/drivers/net/dpaa2
CFLAGS += -I$(RTE_SDK)/drivers/net/dpaa2/base
CFLAGS += -I$(RTE_SDK_DPAA2)/mc
CFLAGS += -I$(RTE_SDK_DPAA2)/qbman/include
CFLAGS += -I$(RTE_SDK_DPAA2)/qbman/include/drivers
CFLAGS += -I$(RTE_SDK)/lib/librte_eal/common/include
CFLAGS += -I$(RTE_SDK)/lib/librte_eal/common/
CFLAGS += -I$(RTE_SDK)/lib/librte_ether
CFLAGS += -I$(RTE_SDK)/lib/librte_eal/linuxapp/eal

# versioning export map
EXPORT_MAP := rte_pmd_dpaa2_version.map

# library version
LIBABIVER := 1

# Core DPAA2 logic
SRCS-$(CONFIG_RTE_LIBRTE_DPAA2_PMD) += base/fsl_vfio.c
SRCS-$(CONFIG_RTE_LIBRTE_DPAA2_PMD) += base/dpaa2_hw_dpni.c
SRCS-$(CONFIG_RTE_LIBRTE_DPAA2_PMD) += base/dpaa2_hw_dpio.c
SRCS-$(CONFIG_RTE_LIBRTE_DPAA2_PMD) += base/dpaa2_hw_dpbp.c

# Interfaces with DPDK
SRCS-$(CONFIG_RTE_LIBRTE_DPAA2_PMD) += dpaa2_ethdev.c
SRCS-$(CONFIG_RTE_LIBRTE_DPAA2_PMD) += dpaa2_rxtx.c


# library dependencies
DEPDIRS-y += lib/librte_eal
DEPDIRS-y += drivers/common/dpaa/mc
DEPDIRS-y += drivers/common/dpaa/qbman

include $(RTE_SDK)/mk/rte.lib.mk
