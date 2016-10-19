/*
 * Copyright (c) 2010-2011  Florian Forster  <ff at octo.it>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * \file oyjl_tree.h
 *
 * Parses JSON data and returns the data in tree form.
 *
 * \author Florian Forster
 * \date August 2010
 *
 * This interface makes quick parsing and extraction of
 * smallish JSON docs trivial:
 *
 * \include example/parse_config.c
 */

#ifndef OYJL_TREE_H
#define OYJL_TREE_H 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <yajl/yajl_parse.h>
#include <yajl/yajl_tree.h>
#ifndef OYJL_API
#define OYJL_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

void       oyjl_tree_to_json         ( yajl_val            v,
                                       int               * level,
                                       char             ** json );
char *     oyjl_value_text           ( yajl_val            v,
                                       void*             (*alloc)(size_t size));
yajl_val   oyjl_tree_get_value       ( yajl_val            v,
                                       const char        * xpath );
yajl_val   oyjl_tree_get_valuef      ( yajl_val            v,
                                       const char        * format,
                                                           ... );
int            oyjl_value_count      ( yajl_val            v );
yajl_val       oyjl_value_pos_get    ( yajl_val            v,
                                       int                 pos );

typedef enum {
  oyjl_message_info = 400 + yajl_status_ok,
  oyjl_message_client_canceled,
  oyjl_message_insufficient_data,
  oyjl_message_error
} oyjl_message_e;
typedef yajl_status(*oyjl_message_f) ( oyjl_message_e      error_code,
                                       const void        * context,
                                       const char        * format,
                                       ... );
yajl_status    oyjl_message_func_set ( oyjl_message_f      message_func );


#ifdef __cplusplus
}
#endif

#endif /* OYJL_TREE_H */
