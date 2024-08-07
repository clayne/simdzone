/*
 * base64.h -- Fast Base64 stream decoder
 *
 * Copyright (c) 2005-2007, Nick Galbreath.
 * Copyright (c) 2015-2018, Wojciech Muła.
 * Copyright (c) 2016-2017, Matthieu Darbois.
 * Copyright (c) 2013-2022, Alfred Klomp.
 * Copyright (c) 2022, NLnet Labs. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 */
#ifndef BASE64_H
#define BASE64_H

// Modified version of https://github.com/aklomp/base64

struct base64_state {
	int eof;
	int bytes;
	unsigned char carry;
};

#include <stdint.h>
#define CHAR62 '+'
#define CHAR63 '/'
#define CHARPAD '='

// End-of-file definitions.
// Almost end-of-file when waiting for the last '=' character:
#define BASE64_AEOF 1
// End-of-file when stream end has been reached or invalid input provided:
#define BASE64_EOF 2

// In the lookup table below, note that the value for '=' (character 61) is
// 254, not 255. This character is used for in-band signaling of the end of
// the datastream, and we will use that later. The characters A-Z, a-z, 0-9
// and + / are mapped to their "decoded" values. The other bytes all map to
// the value 255, which flags them as "invalid input".

static const uint8_t
base64_table_dec_8bit[] =
{
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		//   0..15
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		//  16..31
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,		//  32..47
   52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 254, 255, 255,		//  48..63
  255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,		//  64..79
   15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,		//  80..95
  255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,		//  96..111
   41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255,		// 112..127
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// 128..143
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
};


#if BYTE_ORDER == LITTLE_ENDIAN

/* SPECIAL DECODE TABLES FOR LITTLE ENDIAN (INTEL) CPUS */

static const uint32_t base64_table_dec_32bit_d0[256] = {
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x000000f8, 0xffffffff, 0xffffffff, 0xffffffff, 0x000000fc,
0x000000d0, 0x000000d4, 0x000000d8, 0x000000dc, 0x000000e0, 0x000000e4,
0x000000e8, 0x000000ec, 0x000000f0, 0x000000f4, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000,
0x00000004, 0x00000008, 0x0000000c, 0x00000010, 0x00000014, 0x00000018,
0x0000001c, 0x00000020, 0x00000024, 0x00000028, 0x0000002c, 0x00000030,
0x00000034, 0x00000038, 0x0000003c, 0x00000040, 0x00000044, 0x00000048,
0x0000004c, 0x00000050, 0x00000054, 0x00000058, 0x0000005c, 0x00000060,
0x00000064, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x00000068, 0x0000006c, 0x00000070, 0x00000074, 0x00000078,
0x0000007c, 0x00000080, 0x00000084, 0x00000088, 0x0000008c, 0x00000090,
0x00000094, 0x00000098, 0x0000009c, 0x000000a0, 0x000000a4, 0x000000a8,
0x000000ac, 0x000000b0, 0x000000b4, 0x000000b8, 0x000000bc, 0x000000c0,
0x000000c4, 0x000000c8, 0x000000cc, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};


static const uint32_t base64_table_dec_32bit_d1[256] = {
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x0000e003, 0xffffffff, 0xffffffff, 0xffffffff, 0x0000f003,
0x00004003, 0x00005003, 0x00006003, 0x00007003, 0x00008003, 0x00009003,
0x0000a003, 0x0000b003, 0x0000c003, 0x0000d003, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000,
0x00001000, 0x00002000, 0x00003000, 0x00004000, 0x00005000, 0x00006000,
0x00007000, 0x00008000, 0x00009000, 0x0000a000, 0x0000b000, 0x0000c000,
0x0000d000, 0x0000e000, 0x0000f000, 0x00000001, 0x00001001, 0x00002001,
0x00003001, 0x00004001, 0x00005001, 0x00006001, 0x00007001, 0x00008001,
0x00009001, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x0000a001, 0x0000b001, 0x0000c001, 0x0000d001, 0x0000e001,
0x0000f001, 0x00000002, 0x00001002, 0x00002002, 0x00003002, 0x00004002,
0x00005002, 0x00006002, 0x00007002, 0x00008002, 0x00009002, 0x0000a002,
0x0000b002, 0x0000c002, 0x0000d002, 0x0000e002, 0x0000f002, 0x00000003,
0x00001003, 0x00002003, 0x00003003, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};


static const uint32_t base64_table_dec_32bit_d2[256] = {
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x00800f00, 0xffffffff, 0xffffffff, 0xffffffff, 0x00c00f00,
0x00000d00, 0x00400d00, 0x00800d00, 0x00c00d00, 0x00000e00, 0x00400e00,
0x00800e00, 0x00c00e00, 0x00000f00, 0x00400f00, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000,
0x00400000, 0x00800000, 0x00c00000, 0x00000100, 0x00400100, 0x00800100,
0x00c00100, 0x00000200, 0x00400200, 0x00800200, 0x00c00200, 0x00000300,
0x00400300, 0x00800300, 0x00c00300, 0x00000400, 0x00400400, 0x00800400,
0x00c00400, 0x00000500, 0x00400500, 0x00800500, 0x00c00500, 0x00000600,
0x00400600, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x00800600, 0x00c00600, 0x00000700, 0x00400700, 0x00800700,
0x00c00700, 0x00000800, 0x00400800, 0x00800800, 0x00c00800, 0x00000900,
0x00400900, 0x00800900, 0x00c00900, 0x00000a00, 0x00400a00, 0x00800a00,
0x00c00a00, 0x00000b00, 0x00400b00, 0x00800b00, 0x00c00b00, 0x00000c00,
0x00400c00, 0x00800c00, 0x00c00c00, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};


static const uint32_t base64_table_dec_32bit_d3[256] = {
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x003e0000, 0xffffffff, 0xffffffff, 0xffffffff, 0x003f0000,
0x00340000, 0x00350000, 0x00360000, 0x00370000, 0x00380000, 0x00390000,
0x003a0000, 0x003b0000, 0x003c0000, 0x003d0000, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000,
0x00010000, 0x00020000, 0x00030000, 0x00040000, 0x00050000, 0x00060000,
0x00070000, 0x00080000, 0x00090000, 0x000a0000, 0x000b0000, 0x000c0000,
0x000d0000, 0x000e0000, 0x000f0000, 0x00100000, 0x00110000, 0x00120000,
0x00130000, 0x00140000, 0x00150000, 0x00160000, 0x00170000, 0x00180000,
0x00190000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x001a0000, 0x001b0000, 0x001c0000, 0x001d0000, 0x001e0000,
0x001f0000, 0x00200000, 0x00210000, 0x00220000, 0x00230000, 0x00240000,
0x00250000, 0x00260000, 0x00270000, 0x00280000, 0x00290000, 0x002a0000,
0x002b0000, 0x002c0000, 0x002d0000, 0x002e0000, 0x002f0000, 0x00300000,
0x00310000, 0x00320000, 0x00330000, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

#elif BYTE_ORDER == BIG_ENDIAN

/* SPECIAL DECODE TABLES FOR BIG ENDIAN (IBM/MOTOROLA/SUN) CPUS */

const uint32_t base64_table_dec_32bit_d0[256] = {
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xf8000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xfc000000,
0xd0000000, 0xd4000000, 0xd8000000, 0xdc000000, 0xe0000000, 0xe4000000,
0xe8000000, 0xec000000, 0xf0000000, 0xf4000000, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000,
0x04000000, 0x08000000, 0x0c000000, 0x10000000, 0x14000000, 0x18000000,
0x1c000000, 0x20000000, 0x24000000, 0x28000000, 0x2c000000, 0x30000000,
0x34000000, 0x38000000, 0x3c000000, 0x40000000, 0x44000000, 0x48000000,
0x4c000000, 0x50000000, 0x54000000, 0x58000000, 0x5c000000, 0x60000000,
0x64000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x68000000, 0x6c000000, 0x70000000, 0x74000000, 0x78000000,
0x7c000000, 0x80000000, 0x84000000, 0x88000000, 0x8c000000, 0x90000000,
0x94000000, 0x98000000, 0x9c000000, 0xa0000000, 0xa4000000, 0xa8000000,
0xac000000, 0xb0000000, 0xb4000000, 0xb8000000, 0xbc000000, 0xc0000000,
0xc4000000, 0xc8000000, 0xcc000000, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t base64_table_dec_32bit_d1[256] = {
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x03e00000, 0xffffffff, 0xffffffff, 0xffffffff, 0x03f00000,
0x03400000, 0x03500000, 0x03600000, 0x03700000, 0x03800000, 0x03900000,
0x03a00000, 0x03b00000, 0x03c00000, 0x03d00000, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000,
0x00100000, 0x00200000, 0x00300000, 0x00400000, 0x00500000, 0x00600000,
0x00700000, 0x00800000, 0x00900000, 0x00a00000, 0x00b00000, 0x00c00000,
0x00d00000, 0x00e00000, 0x00f00000, 0x01000000, 0x01100000, 0x01200000,
0x01300000, 0x01400000, 0x01500000, 0x01600000, 0x01700000, 0x01800000,
0x01900000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x01a00000, 0x01b00000, 0x01c00000, 0x01d00000, 0x01e00000,
0x01f00000, 0x02000000, 0x02100000, 0x02200000, 0x02300000, 0x02400000,
0x02500000, 0x02600000, 0x02700000, 0x02800000, 0x02900000, 0x02a00000,
0x02b00000, 0x02c00000, 0x02d00000, 0x02e00000, 0x02f00000, 0x03000000,
0x03100000, 0x03200000, 0x03300000, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t base64_table_dec_32bit_d2[256] = {
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x000f8000, 0xffffffff, 0xffffffff, 0xffffffff, 0x000fc000,
0x000d0000, 0x000d4000, 0x000d8000, 0x000dc000, 0x000e0000, 0x000e4000,
0x000e8000, 0x000ec000, 0x000f0000, 0x000f4000, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000,
0x00004000, 0x00008000, 0x0000c000, 0x00010000, 0x00014000, 0x00018000,
0x0001c000, 0x00020000, 0x00024000, 0x00028000, 0x0002c000, 0x00030000,
0x00034000, 0x00038000, 0x0003c000, 0x00040000, 0x00044000, 0x00048000,
0x0004c000, 0x00050000, 0x00054000, 0x00058000, 0x0005c000, 0x00060000,
0x00064000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x00068000, 0x0006c000, 0x00070000, 0x00074000, 0x00078000,
0x0007c000, 0x00080000, 0x00084000, 0x00088000, 0x0008c000, 0x00090000,
0x00094000, 0x00098000, 0x0009c000, 0x000a0000, 0x000a4000, 0x000a8000,
0x000ac000, 0x000b0000, 0x000b4000, 0x000b8000, 0x000bc000, 0x000c0000,
0x000c4000, 0x000c8000, 0x000cc000, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};


const uint32_t base64_table_dec_32bit_d3[256] = {
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x00003e00, 0xffffffff, 0xffffffff, 0xffffffff, 0x00003f00,
0x00003400, 0x00003500, 0x00003600, 0x00003700, 0x00003800, 0x00003900,
0x00003a00, 0x00003b00, 0x00003c00, 0x00003d00, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000,
0x00000100, 0x00000200, 0x00000300, 0x00000400, 0x00000500, 0x00000600,
0x00000700, 0x00000800, 0x00000900, 0x00000a00, 0x00000b00, 0x00000c00,
0x00000d00, 0x00000e00, 0x00000f00, 0x00001000, 0x00001100, 0x00001200,
0x00001300, 0x00001400, 0x00001500, 0x00001600, 0x00001700, 0x00001800,
0x00001900, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0x00001a00, 0x00001b00, 0x00001c00, 0x00001d00, 0x00001e00,
0x00001f00, 0x00002000, 0x00002100, 0x00002200, 0x00002300, 0x00002400,
0x00002500, 0x00002600, 0x00002700, 0x00002800, 0x00002900, 0x00002a00,
0x00002b00, 0x00002c00, 0x00002d00, 0x00002e00, 0x00002f00, 0x00003000,
0x00003100, 0x00003200, 0x00003300, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

#else

# error "byte order unknown"

#endif // LITTLE_ENDIAN

static really_inline int
dec_loop_generic_32_inner (const uint8_t **s, uint8_t **o, size_t *rounds)
{
	const uint32_t str
		= base64_table_dec_32bit_d0[(*s)[0]]
		| base64_table_dec_32bit_d1[(*s)[1]]
		| base64_table_dec_32bit_d2[(*s)[2]]
		| base64_table_dec_32bit_d3[(*s)[3]];

#if BYTE_ORDER == LITTLE_ENDIAN

	// LUTs for little-endian set MSB in case of invalid character:
	if (str & UINT32_C(0x80000000)) {
		return 0;
	}
#else
	// LUTs for big-endian set LSB in case of invalid character:
	if (str & UINT32_C(1)) {
		return 0;
	}
#endif
	// Store the output:
	memcpy(*o, &str, sizeof (str));

	*s += 4;
	*o += 3;
	*rounds -= 1;

	return 1;
}

static really_inline void
dec_loop_generic_32 (const uint8_t **s, size_t *slen, uint8_t **o, size_t *olen)
{
	if (*slen < 8) {
		return;
	}

	// Process blocks of 4 bytes per round. Because one extra zero byte is
	// written after the output, ensure that there will be at least 4 bytes
	// of input data left to cover the gap. (Two data bytes and up to two
	// end-of-string markers.)
	size_t rounds = (*slen - 4) / 4;

	*slen -= rounds * 4;	// 4 bytes consumed per round
	*olen += rounds * 3;	// 3 bytes produced per round

	do {
		if (rounds >= 8) {
			if (dec_loop_generic_32_inner(s, o, &rounds) &&
			    dec_loop_generic_32_inner(s, o, &rounds) &&
			    dec_loop_generic_32_inner(s, o, &rounds) &&
			    dec_loop_generic_32_inner(s, o, &rounds) &&
			    dec_loop_generic_32_inner(s, o, &rounds) &&
			    dec_loop_generic_32_inner(s, o, &rounds) &&
			    dec_loop_generic_32_inner(s, o, &rounds) &&
			    dec_loop_generic_32_inner(s, o, &rounds)) {
				continue;
			}
			break;
		}
		if (rounds >= 4) {
			if (dec_loop_generic_32_inner(s, o, &rounds) &&
			    dec_loop_generic_32_inner(s, o, &rounds) &&
			    dec_loop_generic_32_inner(s, o, &rounds) &&
			    dec_loop_generic_32_inner(s, o, &rounds)) {
				continue;
			}
			break;
		}
		if (rounds >= 2) {
			if (dec_loop_generic_32_inner(s, o, &rounds) &&
			    dec_loop_generic_32_inner(s, o, &rounds)) {
				continue;
			}
			break;
		}
		dec_loop_generic_32_inner(s, o, &rounds);
		break;

	} while (rounds > 0);

	// Adjust for any rounds that were skipped:
	*slen += rounds * 4;
	*olen -= rounds * 3;
}

nonnull((1,2,4,5))
static really_inline int base64_stream_decode(
  struct base64_state *state,
  const char *src,
  size_t srclen,
  uint8_t *out,
  size_t *outlen)
{
  int ret = 0;
  const uint8_t *s = (const uint8_t *) src;
  uint8_t *o = (uint8_t *) out;
  uint8_t q;

  // Use local temporaries to avoid cache thrashing:
  size_t olen = 0;
  size_t slen = srclen;
  struct base64_state st;
  st.eof = state->eof;
  st.bytes = state->bytes;
  st.carry = state->carry;

  // If we previously saw an EOF or an invalid character, bail out:
  if (st.eof) {
    *outlen = 0;
    ret = 0;
    // If there was a trailing '=' to check, check it:
    if (slen && (st.eof == BASE64_AEOF)) {
      state->bytes = 0;
      state->eof = BASE64_EOF;
      ret = ((base64_table_dec_8bit[*s++] == 254) && (slen == 1)) ? 1 : 0;
    }
    return ret;
  }

  // Turn four 6-bit numbers into three bytes:
  // out[0] = 11111122
  // out[1] = 22223333
  // out[2] = 33444444
  
  // Duff's device again:
  switch (st.bytes)
  {
#if defined(__SUNPRO_C)
#pragma error_messages(off, E_STATEMENT_NOT_REACHED)
#endif
    for (;;)
#if defined(__SUNPRO_C)
#pragma error_messages(default, E_STATEMENT_NOT_REACHED)
#endif
    {
    case 0:
      dec_loop_generic_32(&s, &slen, &o, &olen);
      if (slen-- == 0) {
        ret = 1;
        break;
      }
      if ((q = base64_table_dec_8bit[*s++]) >= 254) {
        st.eof = BASE64_EOF;
        // Treat character '=' as invalid for byte 0:
        break;
      }
      st.carry = (uint8_t)(q << 2);
      st.bytes++;

      // fallthrough

    case 1:
      if (slen-- == 0) {
        ret = 1;
        break;
      }
      if ((q = base64_table_dec_8bit[*s++]) >= 254) {
        st.eof = BASE64_EOF;
        // Treat character '=' as invalid for byte 1:
        break;
      }
      *o++ = st.carry | (q >> 4);
      st.carry = (uint8_t)(q << 4);
      st.bytes++;
      olen++;

  		// fallthrough

    case 2:
      if (slen-- == 0) {
        ret = 1;
        break;
      }
      if ((q = base64_table_dec_8bit[*s++]) >= 254) {
        st.bytes++;
        // When q == 254, the input char is '='.
        // Check if next byte is also '=':
        if (q == 254) {
          if (slen-- != 0) {
            st.bytes = 0;
            // EOF:
            st.eof = BASE64_EOF;
            q = base64_table_dec_8bit[*s++];
            ret = ((q == 254) && (slen == 0)) ? 1 : 0;
            break;
          }
          else {
            // Almost EOF
            st.eof = BASE64_AEOF;
            ret = 1;
            break;
          }
        }
        // If we get here, there was an error:
        break;
      }
      *o++ = st.carry | (q >> 2);
      st.carry = (uint8_t)(q << 6);
      st.bytes++;
      olen++;

      // fallthrough

    case 3:
      if (slen-- == 0) {
        ret = 1;
        break;
      }
      if ((q = base64_table_dec_8bit[*s++]) >= 254) {
        st.bytes = 0;
        st.eof = BASE64_EOF;
        // When q == 254, the input char is '='. Return 1 and EOF.
        // When q == 255, the input char is invalid. Return 0 and EOF.
        ret = ((q == 254) && (slen == 0)) ? 1 : 0;
        break;
      }
      *o++ = st.carry | q;
      st.carry = 0;
      st.bytes = 0;
      olen++;
    }
  }

  state->eof = st.eof;
  state->bytes = st.bytes;
  state->carry = st.carry;
  *outlen = olen;
  return ret;
}

nonnull((1,3,4))
static really_inline int base64_decode(
  const char *src,
  size_t srclen,
  uint8_t *out,
  size_t *outlen)
{
  struct base64_state state = { .eof = 0, .bytes = 0, .carry = 0 };
  return base64_stream_decode(&state, src, srclen, out, outlen) & !state.bytes;
}

nonnull_all
static really_inline int32_t parse_base64_sequence(
  parser_t *parser,
  const type_info_t *type,
  const rdata_info_t *item,
  rdata_t *rdata,
  token_t *token)
{
  if (is_contiguous(token)) {
    struct base64_state state = { .eof = 0, .bytes = 0, .carry = 0 };

    do {
      size_t length = token->length / 4;
      if (((uintptr_t)rdata->limit - (uintptr_t)rdata->octets) / 3 < length)
        SYNTAX_ERROR(parser, "Invalid %s in %s", NAME(item), NAME(type));
      if (!base64_stream_decode(&state, token->data, token->length, rdata->octets, &length))
        SYNTAX_ERROR(parser, "Invalid %s in %s", NAME(item), NAME(type));
      rdata->octets += length;
      take(parser, token);
    } while (is_contiguous(token));

    // incomplete base64 sequence
    if (state.bytes)
      SYNTAX_ERROR(parser, "Invalid %s in %s", NAME(item), NAME(type));
  }

  return have_delimiter(parser, type, token);
}

nonnull_all
static really_inline int32_t parse_base64(
  parser_t *parser,
  const type_info_t *type,
  const rdata_info_t *item,
  rdata_t *rdata,
  const token_t *token)
{
  size_t length = token->length / 4;
  if (((uintptr_t)rdata->limit - (uintptr_t)rdata->octets) / 3 < length)
    SYNTAX_ERROR(parser, "Invalid %s in %s", NAME(item), NAME(type));
  if (!base64_decode(token->data, token->length, rdata->octets, &length))
    SYNTAX_ERROR(parser, "Invalid %s in %s", NAME(item), NAME(type));
  rdata->octets += length;
  return 0;
}

#endif // BASE64_H
