/*
 * Copyright 2008-2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause or GPL-2.0+
 */

#ifndef __DESC_ALGO_H__
#define __DESC_ALGO_H__

#include "flib/rta.h"
#include "common.h"

/**
 * DOC: Algorithms - Shared Descriptor Constructors
 *
 * Shared descriptors for algorithms (i.e. not for protocols).
 */

/**
 * cnstr_shdsc_snow_f8 - SNOW/f8 (UEA2) as a shared descriptor
 * @descbuf: pointer to descriptor-under-construction buffer
 * @ps: if 36/40bit addressing is desired, this parameter must be true
 * @swap: must be true when core endianness doesn't match SEC endianness
 * @cipherdata: pointer to block cipher transform definitions
 * @dir: Cipher direction (DIR_ENC/DIR_DEC)
 * @count: UEA2 count value (32 bits)
 * @bearer: UEA2 bearer ID (5 bits)
 * @direction: UEA2 direction (1 bit)
 *
 * Return: size of descriptor written in words or negative number on error
 */
static inline int cnstr_shdsc_snow_f8(uint32_t *descbuf, bool ps, bool swap,
			 struct alginfo *cipherdata, uint8_t dir,
			 uint32_t count, uint8_t bearer, uint8_t direction)
{
	struct program prg;
	struct program *p = &prg;
	uint32_t ct = count;
	uint8_t br = bearer;
	uint8_t dr = direction;
	uint32_t context[2] = {ct, (br << 27) | (dr << 26)};

	PROGRAM_CNTXT_INIT(p, descbuf, 0);
	if (swap) {
		PROGRAM_SET_BSWAP(p);

		context[0] = swab32(context[0]);
		context[1] = swab32(context[1]);
	}

	if (ps)
		PROGRAM_SET_36BIT_ADDR(p);
	SHR_HDR(p, SHR_ALWAYS, 1, 0);

	KEY(p, KEY1, cipherdata->key_enc_flags, cipherdata->key,
	    cipherdata->keylen, INLINE_KEY(cipherdata));
	MATHB(p, SEQINSZ, SUB, MATH2, VSEQINSZ, 4, 0);
	MATHB(p, SEQINSZ, SUB, MATH2, VSEQOUTSZ, 4, 0);
	ALG_OPERATION(p, OP_ALG_ALGSEL_SNOW_F8, OP_ALG_AAI_F8,
		      OP_ALG_AS_INITFINAL, 0, dir);
	LOAD(p, (uintptr_t)context, CONTEXT1, 0, 8, IMMED | COPY);
	SEQFIFOLOAD(p, MSG1, 0, VLF | LAST1);
	SEQFIFOSTORE(p, MSG, 0, 0, VLF);

	return PROGRAM_FINALIZE(p);
}

/**
 * cnstr_shdsc_snow_f9 - SNOW/f9 (UIA2) as a shared descriptor
 * @descbuf: pointer to descriptor-under-construction buffer
 * @ps: if 36/40bit addressing is desired, this parameter must be true
 * @swap: must be true when core endianness doesn't match SEC endianness
 * @authdata: pointer to authentication transform definitions
 * @dir: cipher direction (DIR_ENC/DIR_DEC)
 * @count: UEA2 count value (32 bits)
 * @fresh: UEA2 fresh value ID (32 bits)
 * @direction: UEA2 direction (1 bit)
 * @datalen: size of data
 *
 * Return: size of descriptor written in words or negative number on error
 */
static inline int cnstr_shdsc_snow_f9(uint32_t *descbuf, bool ps, bool swap,
			 struct alginfo *authdata, uint8_t dir, uint32_t count,
			 uint32_t fresh, uint8_t direction, uint32_t datalen)
{
	struct program prg;
	struct program *p = &prg;
	uint64_t ct = count;
	uint64_t fr = fresh;
	uint64_t dr = direction;
	uint64_t context[2];

	context[0] = (ct << 32) | (dr << 26);
	context[1] = fr << 32;

	PROGRAM_CNTXT_INIT(p, descbuf, 0);
	if (swap) {
		PROGRAM_SET_BSWAP(p);

		context[0] = swab64(context[0]);
		context[1] = swab64(context[1]);
	}
	if (ps)
		PROGRAM_SET_36BIT_ADDR(p);
	SHR_HDR(p, SHR_ALWAYS, 1, 0);

	KEY(p, KEY2, authdata->key_enc_flags, authdata->key, authdata->keylen,
	    INLINE_KEY(authdata));
	MATHB(p, SEQINSZ, SUB, MATH2, VSEQINSZ, 4, 0);
	ALG_OPERATION(p, OP_ALG_ALGSEL_SNOW_F9, OP_ALG_AAI_F9,
		      OP_ALG_AS_INITFINAL, 0, dir);
	LOAD(p, (uintptr_t)context, CONTEXT2, 0, 16, IMMED | COPY);
	SEQFIFOLOAD(p, BIT_DATA, datalen, CLASS2 | LAST2);
	/* Save lower half of MAC out into a 32-bit sequence */
	SEQSTORE(p, CONTEXT2, 0, 4, 0);

	return PROGRAM_FINALIZE(p);
}

/**
 * cnstr_shdsc_blkcipher - block cipher transformation
 * @descbuf: pointer to descriptor-under-construction buffer
 * @ps: if 36/40bit addressing is desired, this parameter must be true
 * @swap: must be true when core endianness doesn't match SEC endianness
 * @cipherdata: pointer to block cipher transform definitions
 * @iv: IV data; if NULL, "ivlen" bytes from the input frame will be read as IV
 * @ivlen: IV length
 * @dir: DIR_ENC/DIR_DEC
 *
 * Return: size of descriptor written in words or negative number on error
 */
static inline int cnstr_shdsc_blkcipher(uint32_t *descbuf, bool ps, bool swap,
			       struct alginfo *cipherdata, uint8_t *iv,
			       uint32_t ivlen, uint8_t dir)
{
	struct program prg;
	struct program *p = &prg;
	const bool is_aes_dec = (dir == DIR_DEC) &&
				(cipherdata->algtype == OP_ALG_ALGSEL_AES);
	LABEL(keyjmp);
	LABEL(skipdk);
	REFERENCE(pkeyjmp);
	REFERENCE(pskipdk);

	PROGRAM_CNTXT_INIT(p, descbuf, 0);
	if (swap)
		PROGRAM_SET_BSWAP(p);
	if (ps)
		PROGRAM_SET_36BIT_ADDR(p);
	SHR_HDR(p, SHR_SERIAL, 1, SC);

	pkeyjmp = JUMP(p, keyjmp, LOCAL_JUMP, ALL_TRUE, SHRD);
	/* Insert Key */
	KEY(p, KEY1, cipherdata->key_enc_flags, cipherdata->key,
	    cipherdata->keylen, INLINE_KEY(cipherdata));

	if (is_aes_dec) {
		ALG_OPERATION(p, cipherdata->algtype, cipherdata->algmode,
			      OP_ALG_AS_INITFINAL, ICV_CHECK_DISABLE, dir);

		pskipdk = JUMP(p, skipdk, LOCAL_JUMP, ALL_TRUE, 0);
	}
	SET_LABEL(p, keyjmp);

	if (is_aes_dec) {
		ALG_OPERATION(p, OP_ALG_ALGSEL_AES, cipherdata->algmode |
			      OP_ALG_AAI_DK, OP_ALG_AS_INITFINAL,
			      ICV_CHECK_DISABLE, dir);
		SET_LABEL(p, skipdk);
	} else {
		ALG_OPERATION(p, cipherdata->algtype, cipherdata->algmode,
			      OP_ALG_AS_INITFINAL, ICV_CHECK_DISABLE, dir);
	}

	if (iv)
		/* IV load, convert size */
		LOAD(p, (uintptr_t)iv, CONTEXT1, 0, ivlen, IMMED | COPY);
	else
		/* IV is present first before the actual message */
		SEQLOAD(p, CONTEXT1, 0, ivlen, 0);

	MATHB(p, SEQINSZ, SUB, MATH2, VSEQINSZ, 4, 0);
	MATHB(p, SEQINSZ, SUB, MATH2, VSEQOUTSZ, 4, 0);

	/* Insert sequence load/store with VLF */
	SEQFIFOLOAD(p, MSG1, 0, VLF | LAST1);
	SEQFIFOSTORE(p, MSG, 0, 0, VLF);

	PATCH_JUMP(p, pkeyjmp, keyjmp);
	if (is_aes_dec)
		PATCH_JUMP(p, pskipdk, skipdk);

	return PROGRAM_FINALIZE(p);
}

/**
 * cnstr_shdsc_hmac - HMAC shared
 * @descbuf: pointer to descriptor-under-construction buffer
 * @ps: if 36/40bit addressing is desired, this parameter must be true
 * @swap: must be true when core endianness doesn't match SEC endianness
 * @authdata: pointer to authentication transform definitions;
 *            message digest algorithm: OP_ALG_ALGSEL_MD5/ SHA1-512.
 * @do_icv: 0 if ICV checking is not desired, any other value if ICV checking
 *          is needed for all the packets processed by this shared descriptor
 * @trunc_len: Length of the truncated ICV to be written in the output buffer, 0
 *             if no truncation is needed
 *
 * Note: There's no support for keys longer than the corresponding digest size,
 * according to the selected algorithm.
 *
 * Return: size of descriptor written in words or negative number on error
 */
static inline int cnstr_shdsc_hmac(uint32_t *descbuf, bool ps, bool swap,
				   struct alginfo *authdata, uint8_t do_icv,
				   uint8_t trunc_len)
{
	struct program prg;
	struct program *p = &prg;
	uint8_t storelen, opicv, dir;
	LABEL(keyjmp);
	LABEL(jmpprecomp);
	REFERENCE(pkeyjmp);
	REFERENCE(pjmpprecomp);

	/* Compute fixed-size store based on alg selection */
	switch (authdata->algtype) {
	case OP_ALG_ALGSEL_MD5:
		storelen = 16;
		break;
	case OP_ALG_ALGSEL_SHA1:
		storelen = 20;
		break;
	case OP_ALG_ALGSEL_SHA224:
		storelen = 28;
		break;
	case OP_ALG_ALGSEL_SHA256:
		storelen = 32;
		break;
	case OP_ALG_ALGSEL_SHA384:
		storelen = 48;
		break;
	case OP_ALG_ALGSEL_SHA512:
		storelen = 64;
		break;
	default:
		return -EINVAL;
	}

	trunc_len = trunc_len && (trunc_len < storelen) ? trunc_len : storelen;

	opicv = do_icv ? ICV_CHECK_ENABLE : ICV_CHECK_DISABLE;
	dir = do_icv ? DIR_DEC : DIR_ENC;

	PROGRAM_CNTXT_INIT(p, descbuf, 0);
	if (swap)
		PROGRAM_SET_BSWAP(p);
	if (ps)
		PROGRAM_SET_36BIT_ADDR(p);
	SHR_HDR(p, SHR_SERIAL, 1, SC);

	pkeyjmp = JUMP(p, keyjmp, LOCAL_JUMP, ALL_TRUE, SHRD);
	KEY(p, KEY2, authdata->key_enc_flags, authdata->key, storelen,
	    INLINE_KEY(authdata));

	/* Do operation */
	ALG_OPERATION(p, authdata->algtype, OP_ALG_AAI_HMAC,
		      OP_ALG_AS_INITFINAL, opicv, dir);

	pjmpprecomp = JUMP(p, jmpprecomp, LOCAL_JUMP, ALL_TRUE, 0);
	SET_LABEL(p, keyjmp);

	ALG_OPERATION(p, authdata->algtype, OP_ALG_AAI_HMAC_PRECOMP,
		      OP_ALG_AS_INITFINAL, opicv, dir);

	SET_LABEL(p, jmpprecomp);

	/* compute sequences */
	if (opicv == ICV_CHECK_ENABLE)
		MATHB(p, SEQINSZ, SUB, trunc_len, VSEQINSZ, 4, IMMED2);
	else
		MATHB(p, SEQINSZ, SUB, MATH2, VSEQINSZ, 4, 0);

	/* Do load (variable length) */
	SEQFIFOLOAD(p, MSG2, 0, VLF | LAST2);

	if (opicv == ICV_CHECK_ENABLE)
		SEQFIFOLOAD(p, ICV2, trunc_len, LAST2);
	else
		SEQSTORE(p, CONTEXT2, 0, trunc_len, 0);

	PATCH_JUMP(p, pkeyjmp, keyjmp);
	PATCH_JUMP(p, pjmpprecomp, jmpprecomp);

	return PROGRAM_FINALIZE(p);
}

/**
 * cnstr_shdsc_kasumi_f8 - KASUMI F8 (Confidentiality) as a shared descriptor
 *                         (ETSI "Document 1: f8 and f9 specification")
 * @descbuf: pointer to descriptor-under-construction buffer
 * @ps: if 36/40bit addressing is desired, this parameter must be true
 * @swap: must be true when core endianness doesn't match SEC endianness
 * @cipherdata: pointer to block cipher transform definitions
 * @dir: cipher direction (DIR_ENC/DIR_DEC)
 * @count: count value (32 bits)
 * @bearer: bearer ID (5 bits)
 * @direction: direction (1 bit)
 *
 * Return: size of descriptor written in words or negative number on error
 */
static inline int cnstr_shdsc_kasumi_f8(uint32_t *descbuf, bool ps, bool swap,
			   struct alginfo *cipherdata, uint8_t dir,
			   uint32_t count, uint8_t bearer, uint8_t direction)
{
	struct program prg;
	struct program *p = &prg;
	uint64_t ct = count;
	uint64_t br = bearer;
	uint64_t dr = direction;
	uint32_t context[2] = { ct, (br << 27) | (dr << 26) };

	PROGRAM_CNTXT_INIT(p, descbuf, 0);
	if (swap) {
		PROGRAM_SET_BSWAP(p);

		context[0] = swab32(context[0]);
		context[1] = swab32(context[1]);
	}
	if (ps)
		PROGRAM_SET_36BIT_ADDR(p);
	SHR_HDR(p, SHR_ALWAYS, 1, 0);

	KEY(p, KEY1, cipherdata->key_enc_flags, cipherdata->key,
	    cipherdata->keylen, INLINE_KEY(cipherdata));
	MATHB(p, SEQINSZ, SUB, MATH2, VSEQINSZ, 4, 0);
	MATHB(p, SEQINSZ, SUB, MATH2, VSEQOUTSZ, 4, 0);
	ALG_OPERATION(p, OP_ALG_ALGSEL_KASUMI, OP_ALG_AAI_F8,
		      OP_ALG_AS_INITFINAL, 0, dir);
	LOAD(p, (uintptr_t)context, CONTEXT1, 0, 8, IMMED | COPY);
	SEQFIFOLOAD(p, MSG1, 0, VLF | LAST1);
	SEQFIFOSTORE(p, MSG, 0, 0, VLF);

	return PROGRAM_FINALIZE(p);
}

/**
 * cnstr_shdsc_kasumi_f9 -  KASUMI F9 (Integrity) as a shared descriptor
 *                          (ETSI "Document 1: f8 and f9 specification")
 * @descbuf: pointer to descriptor-under-construction buffer
 * @ps: if 36/40bit addressing is desired, this parameter must be true
 * @swap: must be true when core endianness doesn't match SEC endianness
 * @authdata: pointer to authentication transform definitions
 * @dir: cipher direction (DIR_ENC/DIR_DEC)
 * @count: count value (32 bits)
 * @fresh: fresh value ID (32 bits)
 * @direction: direction (1 bit)
 * @datalen: size of data
 *
 * Return: size of descriptor written in words or negative number on error
 */
static inline int cnstr_shdsc_kasumi_f9(uint32_t *descbuf, bool ps, bool swap,
			   struct alginfo *authdata, uint8_t dir,
			   uint32_t count, uint32_t fresh, uint8_t direction,
			   uint32_t datalen)
{
	struct program prg;
	struct program *p = &prg;
	uint16_t ctx_offset = 16;
	uint32_t context[6] = {count, direction << 26, fresh, 0, 0, 0};

	PROGRAM_CNTXT_INIT(p, descbuf, 0);
	if (swap) {
		PROGRAM_SET_BSWAP(p);

		context[0] = swab32(context[0]);
		context[1] = swab32(context[1]);
		context[2] = swab32(context[2]);
	}
	if (ps)
		PROGRAM_SET_36BIT_ADDR(p);
	SHR_HDR(p, SHR_ALWAYS, 1, 0);

	KEY(p, KEY1, authdata->key_enc_flags, authdata->key, authdata->keylen,
	    INLINE_KEY(authdata));
	MATHB(p, SEQINSZ, SUB, MATH2, VSEQINSZ, 4, 0);
	ALG_OPERATION(p, OP_ALG_ALGSEL_KASUMI, OP_ALG_AAI_F9,
		      OP_ALG_AS_INITFINAL, 0, dir);
	LOAD(p, (uintptr_t)context, CONTEXT1, 0, 24, IMMED | COPY);
	SEQFIFOLOAD(p, BIT_DATA, datalen, CLASS1 | LAST1);
	/* Save output MAC of DWORD 2 into a 32-bit sequence */
	SEQSTORE(p, CONTEXT1, ctx_offset, 4, 0);

	return PROGRAM_FINALIZE(p);
}

/**
 * cnstr_shdsc_crc - CRC32 Accelerator (IEEE 802 CRC32 protocol mode)
 * @descbuf: pointer to descriptor-under-construction buffer
 * @swap: must be true when core endianness doesn't match SEC endianness
 *
 * Return: size of descriptor written in words or negative number on error
 */
static inline int cnstr_shdsc_crc(uint32_t *descbuf, bool swap)
{
	struct program prg;
	struct program *p = &prg;

	PROGRAM_CNTXT_INIT(p, descbuf, 0);
	if (swap)
		PROGRAM_SET_BSWAP(p);

	SHR_HDR(p, SHR_ALWAYS, 1, 0);

	MATHB(p, SEQINSZ, SUB, MATH2, VSEQINSZ, 4, 0);
	ALG_OPERATION(p, OP_ALG_ALGSEL_CRC,
		      OP_ALG_AAI_802 | OP_ALG_AAI_DOC,
		      OP_ALG_AS_FINALIZE, 0, DIR_ENC);
	SEQFIFOLOAD(p, MSG2, 0, VLF | LAST2);
	SEQSTORE(p, CONTEXT2, 0, 4, 0);

	return PROGRAM_FINALIZE(p);
}

#endif /* __DESC_ALGO_H__ */
