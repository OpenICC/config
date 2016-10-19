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

#include <stdarg.h>  /* va_list */
#include <stddef.h>  /* ptrdiff_t size_t */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

#include <yajl/yajl_parse.h>
#ifndef YAJL_VERSION
#include <yajl/yajl_version.h>
#endif
#include "oyjl_tree.h"

#if defined(_MSC_VER) 
#define snprintf sprintf_s
#endif

char *             oyjl_string_copy  ( char              * string,
                                       void*            (* alloc)(size_t size))
{
  char * text = 0;

  if(!alloc) alloc = malloc;

  text = alloc( strlen(string) + 1 );
  strcpy( text, string );
    
  return text;
}

int                   oyjl_string_add( char             ** string,
                                       const char        * format,
                                                           ... )
{
  char * text_copy = NULL;
  char * text = 0;
  va_list list;
  int len;
  size_t sz = 0;

  va_start( list, format);
  len = vsnprintf( text, sz, format, list );
  va_end  ( list );

  {
    text = malloc( len + 1 );
    va_start( list, format);
    len = vsnprintf( text, len+1, format, list );
    va_end  ( list );
  }

  if(string && *string)
  {
    int l = strlen(*string);
    text_copy = malloc( len + l + 1 );
    strcpy( text_copy, *string );
    strcpy( &text_copy[l], text );
    

    free(*string);
    *string = text_copy;

    free(text);

  } else
    *string = text;

  return 0;
}

char * oyjl_value_text (yajl_val v, void*(*alloc)(size_t size))
{
  char * t = 0, * text = 0;

  if(v)
  switch(v->type)
  {
    case yajl_t_null:
         break;
    case yajl_t_number:
         if(v->u.number.flags & YAJL_NUMBER_DOUBLE_VALID)
           oyjl_string_add (&t, "%g", v->u.number.d);
         else
           oyjl_string_add (&t, "%ld", v->u.number.i);
         break;
    case yajl_t_true:
         oyjl_string_add (&t, "1"); break;
    case yajl_t_false:
         oyjl_string_add (&t, "0"); break;
    case yajl_t_string:
         oyjl_string_add (&t, "%s", v->u.string); break;
    case yajl_t_array:
    case yajl_t_object:
         break;
    default:
         fprintf( stderr, "unknown type: %d\n", v->type );
         break;
  }

  if(t)
  {
    text = oyjl_string_copy (t, alloc);
    free (t); t = 0;
  }

  return text;
}

void oyjl_tree_to_json (yajl_val v, int * level, char ** json)
{
  int n = *level;

  if(v)
  switch(v->type)
  {
    case yajl_t_null:
         break;
    case yajl_t_number:
         if(v->u.number.flags & YAJL_NUMBER_DOUBLE_VALID)
           oyjl_string_add (json, "%g", v->u.number.d);
         else
           oyjl_string_add (json, "%ld", v->u.number.i);
         break;
    case yajl_t_true:
         oyjl_string_add (json, "1"); break;
    case yajl_t_false:
         oyjl_string_add (json, "0"); break;
    case yajl_t_string:
         oyjl_string_add (json, "\"%s\"", v->u.string); break;
    case yajl_t_array:
         {
           int i,
               count = v->u.array.len;

           oyjl_string_add( json, "[" );

           *level += 2;
           for(i = 0; i < count; ++i)
           {
             oyjl_tree_to_json( v->u.array.values[i], level, json );
             if(count > 1)
             {
               if(i < count - 1)
                 oyjl_string_add( json, "," );
             }
           }
           *level -= 2;

           oyjl_string_add( json, "]");
         } break;
    case yajl_t_object:
         {
           int i,
               count = v->u.object.len;

           oyjl_string_add( json, "{" );

           *level += 2;
           for(i = 0; i < count; ++i)
           {
             oyjl_string_add( json, "\n");
             n = *level; while(n--) oyjl_string_add(json, " ");
             oyjl_string_add( json, "\"%s\": ", v->u.object.keys[i] );
             oyjl_tree_to_json( v->u.object.values[i], level, json );
             if(count > 1)
             {
               if(i < count - 1)
                 oyjl_string_add( json, "," );
             }
           }
           *level -= 2;

           oyjl_string_add( json, "\n");
           n = *level; while(n--) oyjl_string_add(json, " ");
           oyjl_string_add( json, "}");
         }
         break;
    default:
         fprintf( stderr, "unknown type: %d\n", v->type );
         break;
  }
  return;
}

int            oyjl_value_count      ( yajl_val            v )
{
  int count = 0;

  if(!v)
    return count;

  if(v->type == yajl_t_object)
    count = v->u.object.len;
  else if(v->type == yajl_t_array)
    count = v->u.array.len;

  return count;
}

yajl_val       oyjl_value_pos_get    ( yajl_val            v,
                                       int                 pos )
{
  if(!v)
    return NULL;

  if(v->type == yajl_t_object)
    return v->u.object.values[pos];
  else if(v->type == yajl_t_array)
    return v->u.array.values[pos];

  return NULL;
}

char **        oyjl_string_split     ( const char        * text,
                                       int               * count )
{
  char ** list = 0;
  int n = 0, i;
  char delimiter = '/';

  /* split the path search string by a delimiter */
  if(text && text[0] && delimiter)
  {
    const char * tmp = text;

    if(tmp[0] == delimiter) ++n;
    do { ++n;
    } while( (tmp = strchr(tmp + 1, delimiter)) );

    tmp = 0;

    if((list = malloc( (n+1) * sizeof(char*) )) == 0) return NULL;

    {
      const char * start = text;
      for(i = 0; i < n; ++i)
      {
        intptr_t len = 0;
        char * end = strchr(start, delimiter);

        if(end > start)
          len = end - start;
        else if (end == start)
          len = 0;
        else
          len = strlen(start);

        if((list[i] = malloc( len+1 )) == 0) return NULL;

        memcpy( list[i], start, len );
        list[i][len] = 0;
        start += len + 1;
      }
    }
  }

  *count = n;

  return list;
}

yajl_val   oyjl_tree_get_value       ( yajl_val            v,
                                       const char        * xpath )
{
  yajl_val level = 0;
  int n = 0, i, found = 0;
  char ** list = oyjl_string_split(xpath, &n),
        * ttmp = 0;

  /* follow the search path term */
  level = v;
  found = n;
  for(i = 0; i < n; ++i)
  {
    char * term = list[i],
         * tindex = strrchr(term,'[');
    int count = oyjl_value_count( level );
    int j;
    int pos = -1;

    
    if(tindex != NULL)
    {
      ptrdiff_t size;
      ++tindex;
      size = strrchr(term,']') - tindex;
      if(size > 0)
      {
        ttmp = malloc(size + 1);
        memcpy( ttmp, tindex, size );
        ttmp[size] = '\000';
        pos = atoi(ttmp);
        size = strrchr(term,'[') - term;
        memcpy( ttmp, term, size );
        ttmp[size] = '\000';
        term = ttmp;
      }
    }

    if(found == 0) break;
    found = 0;

    if(!(term && term[0]) && pos != -1)
    {
      level = oyjl_value_pos_get( level, pos );
      found = 1;
    } else
    for(j = 0; j < count; ++j)
    {
        if(term &&
           strcmp( level->u.object.keys[j], term ) == 0)
        {
          ++found;
          if(pos == -1 ||
             (found-1) == pos)
          {
            level = level->u.object.values[j];
            break;
          }
        }
    }
  }

  /* clean up temorary memory */
  for(i = 0; i < n; ++i)
    free(list[i]);
  if(list)
    free(list);

  if(ttmp)
    free( ttmp );

  if(found && level)
    return level;
  else
    return NULL;
}

/** Function oyjl_tree_get_valuef
 *  @brief   get a child node
 *
 *  @param[in]     v                   the oyjl node
 *  @param[in]     format              the xpath format
 *  @param[in]     ...                 the variable argument list
 *  @return                            the childs text value
 *
 *  @version Oyranos: 0.9.5
 *  @since   2011/09/24 (Oyranos: 0.3.3)
 *  @date    2013/02/24
 */
yajl_val   oyjl_tree_get_valuef      ( yajl_val            v,
                                       const char        * format,
                                                           ... )
{
  yajl_val value = 0;

  char * text = 0;
  va_list list;
  int len;
  size_t sz = strlen(format) * 2;

  text = malloc( sz );
  if(!text)
  {
    fprintf( stderr, "!!! ERROR: could not allocate memory\n" );
    return 0;
  }

  text[0] = 0;

  va_start( list, format);
  len = vsnprintf( text, sz, format, list );
  va_end  ( list );

  if (len >= sz)
  {
    text = realloc( text, (len+1)*sizeof(char) );
    va_start( list, format);
    len = vsnprintf( text, len+1, format, list );
    va_end  ( list );
  }

  value = oyjl_tree_get_value( v, text );

  if(text) free(text);

  return value;
}


