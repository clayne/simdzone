/*
 * syntax.c -- presentation format syntax test cases
 *
 * Copyright (c) 2023, NLnet Labs. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>
#include <cmocka.h>
#if !_WIN32
#include <unistd.h>
#endif

#include "zone.h"
#include "diagnostic.h"
#include "tools.h"

static int32_t digest_test_accept_rr(
  zone_parser_t *parser,
  const zone_name_t *owner,
  uint16_t type,
  uint16_t class,
  uint32_t ttl,
  uint16_t rdlength,
  const uint8_t *rdata,
  void *user_data)
{
  (void)parser;
  (void)owner;
  (void)type;
  (void)class;
  (void)ttl;
  (void)rdlength;
  (void)rdata;
  (void)user_data;
  return 0;
}

static int32_t parse_digest(const char *input)
{
  const uint8_t origin[] = { 0 };
  zone_parser_t parser;
  zone_name_buffer_t name;
  zone_rdata_buffer_t rdata;
  zone_buffers_t buffers = { 1, &name, &rdata };
  zone_options_t options;

  memset(&options, 0, sizeof(options));
  options.accept.callback = digest_test_accept_rr;
  options.origin.octets = origin;
  options.origin.length = sizeof(origin);
  options.default_ttl = 3600;
  options.default_class = 1;

  fprintf(stderr, "INPUT: \"%s\"\n", input);
  return zone_parse_string(
    &parser, &options, &buffers, input, strlen(input), NULL);
}

/*!cmocka */
void ds_digest_lengths(void **state)
{
  static const char fmt[] =
    "dskey.example.com. 86400 IN DS 60485 5 %c ( %.*s )";
  static const char hex_fmt[] =
    "dskey.example.com. 86400 CLASS1 TYPE43 \\# %d EC45 05 0%c ( %.*s )";
  static const char hex[] =
    "0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef";

  static const struct {
    int algorithm;
    int digest_length;
    int32_t code;
  } tests[] = {
    // 0: Reserved
    { 0, 10, ZONE_SUCCESS },
    // 1: SHA-1
    { 1, 20, ZONE_SUCCESS },
    { 1, 19, ZONE_SEMANTIC_ERROR },
    { 1, 21, ZONE_SEMANTIC_ERROR },
    // 2: SHA-256
    { 2, 32, ZONE_SUCCESS },
    { 2, 31, ZONE_SEMANTIC_ERROR },
    { 2, 33, ZONE_SEMANTIC_ERROR },
    // 3: GOST R 34.11-94
    { 3, 32, ZONE_SUCCESS },
    { 3, 31, ZONE_SEMANTIC_ERROR },
    { 3, 33, ZONE_SEMANTIC_ERROR },
    // 4: SHA-384
    { 4, 48, ZONE_SUCCESS },
    { 4, 47, ZONE_SEMANTIC_ERROR },
    { 4, 49, ZONE_SEMANTIC_ERROR },
    // 5: GOST R 34.10-2012
    { 5, 48, ZONE_SUCCESS },
    { 5, 47, ZONE_SEMANTIC_ERROR },
    { 5, 49, ZONE_SEMANTIC_ERROR },
    // 6: SM3
    { 6, 48, ZONE_SUCCESS },
    { 6, 47, ZONE_SEMANTIC_ERROR },
    { 6, 49, ZONE_SEMANTIC_ERROR }
  };

  (void)state;

  int32_t code;
  for (size_t i=0, n = sizeof(tests)/sizeof(tests[0]); i < n; i++) {
    char buf[512];
    const int algo = tests[i].algorithm;
    const int len = tests[i].digest_length;

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), fmt, algo + 0x30, len * 2, hex);
    code = parse_digest(buf);
    assert_int_equal(code, tests[i].code);

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), hex_fmt, 4 + len, algo + 0x30, len * 2, hex);
    code = parse_digest(buf);
    assert_int_equal(code, tests[i].code);
  }
}

/*!cmocka */
void zonemd_digest_lengths(void **state)
{
  static const char fmt[] =
    "example.com. 86400 IN ZONEMD 2018031500 1 %c ( %.*s )";
  static const char hex_fmt[] =
    "example.com. 86400 CLASS1 TYPE63 \\# %d 7848B78C 01 0%c ( %.*s )";
  static const char hex[] =
    "0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef";

  static const struct {
    int algorithm;
    int digest_length;
    int32_t code;
  } tests[] = {
    // 0: Reserved
    { 0, 10, ZONE_SUCCESS },
    // 1: SHA-384
    { 1, 48, ZONE_SUCCESS },
    { 1, 47, ZONE_SEMANTIC_ERROR },
    { 1, 49, ZONE_SEMANTIC_ERROR },
    // 2: SHA-512
    { 2, 64, ZONE_SUCCESS },
    { 2, 63, ZONE_SEMANTIC_ERROR },
    { 2, 65, ZONE_SEMANTIC_ERROR }
  };

  (void)state;

  int32_t code;

  for (size_t i=0, n = sizeof(tests)/sizeof(tests[0]); i < n; i++) {
    char buf[512];
    const int algo = tests[i].algorithm;
    const int len = tests[i].digest_length;

    snprintf(buf, sizeof(buf), fmt, algo + 0x30, len * 2, hex);
    code = parse_digest(buf);
    assert_int_equal(code, tests[i].code);

    snprintf(buf, sizeof(buf), hex_fmt, 6 + len, algo + 0x30, len * 2, hex);
    code = parse_digest(buf);
    assert_int_equal(code, tests[i].code);
  }
}

/*!cmocka */
void dsync_scheme_types(void **state)
{
  static const char fmt[] =
    "example.com. 86400 IN DSYNC %s %d 5359 ( type-scanner.example.net. )";
  static const char hex_fmt[] =
    "example.com. 86400 CLASS1 TYPE66 \\# 31 %.4x %.2x 14ef "
    "( 0c747970652d7363616e6e6572076578616d706c65036e657400 )";

  static const struct {
    const char* dsync_type_str;
    uint16_t    dsync_type;
    uint8_t     dsync_scheme;
    int32_t     code;
  } tests[] = {
    // Schema 0: Reserved
    { "CDS"    , ZONE_TYPE_CDS    , 0, ZONE_SUCCESS },
    { "TYPE59" , 59 /* CDS */     , 0, ZONE_SUCCESS },
    { "CSYNC"  , ZONE_TYPE_CSYNC  , 0, ZONE_SUCCESS },
    { "TYPE62" , 62 /* CSYNC */   , 0, ZONE_SUCCESS },
    { "TXT"    , ZONE_TYPE_TXT    , 0, ZONE_SUCCESS },
    { "TYPE16" , 16 /* TXT */     , 0, ZONE_SUCCESS },
    // Scheme 1: only CDS and CSYNC
    { "CDS"    , ZONE_TYPE_CDS    , 1, ZONE_SUCCESS },
    { "TYPE59" , 59 /* CDS */     , 1, ZONE_SUCCESS },
    { "CSYNC"  , ZONE_TYPE_CSYNC  , 1, ZONE_SUCCESS },
    { "TYPE62" , 62 /* CSYNC */   , 1, ZONE_SUCCESS },
    { "TXT"    , ZONE_TYPE_TXT    , 1, ZONE_SEMANTIC_ERROR },
    { "TYPE16" , 16 /* TXT */     , 1, ZONE_SEMANTIC_ERROR },
    // Other schemes, anything goes
    { "CDS"    , ZONE_TYPE_CDS    , 2, ZONE_SUCCESS },
    { "TYPE59" , 59 /* CDS */     , 2, ZONE_SUCCESS },
    { "CSYNC"  , ZONE_TYPE_CSYNC  , 2, ZONE_SUCCESS },
    { "TYPE62" , 62 /* CSYNC */   , 2, ZONE_SUCCESS },
    { "TXT"    , ZONE_TYPE_TXT    , 2, ZONE_SUCCESS },
    { "TYPE16" , 16 /* TXT */     , 2, ZONE_SUCCESS }
  };

  (void)state;

  int32_t code;

  for (size_t i=0, n = sizeof(tests)/sizeof(tests[0]); i < n; i++) {
    char buf[512];
    const char    *dsync_type_str = tests[i].dsync_type_str;
    const uint16_t dsync_type   = tests[i].dsync_type;
    const uint8_t  dsync_scheme = tests[i].dsync_scheme;

    snprintf(buf, sizeof(buf), fmt, dsync_type_str, (int)dsync_scheme);
    code = parse_digest(buf);
    assert_int_equal(code, tests[i].code);

    /* No need to htobe16(dsync_type), since %.4x is "written" big endian */
    snprintf(buf, sizeof(buf), hex_fmt, (int)dsync_type, (int)dsync_scheme);
    code = parse_digest(buf);
    assert_int_equal(code, tests[i].code);
  }
}
