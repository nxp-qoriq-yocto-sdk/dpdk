#   BSD LICENSE
#
#   Copyright(c) 2016 NXP. All rights reserved.
#   All rights reserved.
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
#     * Neither the name of NXP nor the names of its
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
RTE_SDK_DPAA=$(RTE_SDK)/drivers/common/dpaa

#
# library name
#
LIB = libdpaa_drivers.a

CFLAGS += -O3 -g
CFLAGS += $(WERROR_FLAGS)
CFLAGS +=-Wno-pointer-arith
CFLAGS +=-Wno-cast-qual
CFLAGS += -D _GNU_SOURCE

CFLAGS += -I$(RTE_SDK_DPAA)/
CFLAGS += -I$(RTE_SDK_DPAA)/include

LIBABIVER := 1

# all source are stored in SRCS-y
#
SRCS-$(CONFIG_RTE_LIBRTE_DPAA_PMD) += \
	drivers/fman/fman.c \
	drivers/of/of.c \
	drivers/process/process.c \
	drivers/qbman/bman_high.c \
	drivers/qbman/bman_driver.c \
	drivers/qbman/qman_high.c \
	drivers/qbman/qman_driver.c \
	drivers/qbman/dpa_alloc.c \
	drivers/qbman/dpa_sys.c \
	drivers/syscfg/usdpaa_netcfg_layer.c


include $(RTE_SDK)/mk/rte.lib.mk
