/*
 * Copyright (c) 2011-2012, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

#include "jsonparse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*--------------------------------------------------------------------*/
static int
push(struct jsonparse_state *state, char c)
{ 	if (state->depth >= JSONPARSE_MAX_DEPTH) { printf("JB patched - state->depth < JSONPARSE_MAX_DEPTH | %d < %d \n", state->depth, JSONPARSE_MAX_DEPTH); /* return JSON_TYPE_ERROR;*/ }
  state->stack[state->depth] = c;
  state->depth++;
  state->vtype = 0;
  return state->depth < JSONPARSE_MAX_DEPTH;
}
/*--------------------------------------------------------------------*/
static char
pop(struct jsonparse_state *state)
{
  if(state->depth == 0) {
    return JSON_TYPE_ERROR;
  }
  state->depth--;
  return state->stack[state->depth];
}
/*--------------------------------------------------------------------*/
/* will pass by the value and store the start and length of the value for
   atomic types */
/*--------------------------------------------------------------------*/
static void
atomic(struct jsonparse_state *state, char type)
{
  char c;

  state->vstart = state->pos;
  state->vtype = type;
  if(type == JSON_TYPE_STRING || type == JSON_TYPE_PAIR_NAME) {
    while((c = state->json[state->pos++]) && c != '"') { // JB patched while((state->pos < state->len) && (c = state->json[state->pos++]) && c != '"') {
      if(c == '\\') {
        state->pos++;           /* skip current char */
      }
      if (state->pos >= state->len) { printf("JB patched %s %s %d - state->pos >= state->len : %d >= %d \n", __FILE__, __FUNCTION__, __LINE__, state->pos, state->len); /* break; */}
    }
    state->vlen = state->pos - state->vstart - 1;
  } else if(type == JSON_TYPE_NUMBER) {
    do {
      if (state->pos >= state->len) { printf("JB patched %s %s %d - state->pos >= state->len : %d >= %d \n", __FILE__, __FUNCTION__, __LINE__, state->pos, state->len); /* break; */ }
      c = state->json[state->pos];
      if((c < '0' || c > '9') && c != '.') {
        c = 0;
      } else {
        state->pos++;
      }
    } while(c);
    /* need to back one step since first char is already gone */
    state->vstart--;
    state->vlen = state->pos - state->vstart;
  }
  /* no other types for now... */
}
/*--------------------------------------------------------------------*/
static void
skip_ws(struct jsonparse_state *state)
{
  char c;
  printf("state->pos = %d state->len = %d\n", state->pos, state->len);
  while((state->pos < state->len) &&
        ((c = state->json[state->pos]) == ' ' || c == '\n')) {
    state->pos++;
  }
}
/*--------------------------------------------------------------------*/
void
jsonparse_setup(struct jsonparse_state *state, const char *json, int len)
{
  state->json = json;
  state->len = len;
  state->pos = 0;
  state->depth = 0;
  state->error = 0;
  state->stack[0] = 0;
}
/*--------------------------------------------------------------------*/
int
jsonparse_next(struct jsonparse_state *state)
{
  char c;
  char s;
	fprintf(stderr, "%s:%d\n", __FUNCTION__, __LINE__);
  skip_ws(state);
  if (state->pos >= state->len) { printf("JB patched %s %s : %d - state->pos >= state->len %d >= %d \n", __FILE__, __FUNCTION__, __LINE__, state->pos, state->len); /* return JSON_TYPE_ERROR; */ }
  c = state->json[state->pos];
  s = jsonparse_get_type(state);
  state->pos++;
fprintf(stderr, "%s:%d\n", __FUNCTION__, __LINE__);
fprintf(stderr, "c = %c\n", c);
  switch(c) { 
  case '{':
   push(state, c);
   // JB patched if (push(state, c) == JSON_TYPE_ERROR) return JSON_TYPE_ERROR;
    return c;
  case '}':
    if(s == ':' && state->vtype != 0) {
/*       printf("Popping vtype: '%c'\n", state->vtype); */
      pop(state);
      s = jsonparse_get_type(state);
    }
    if(s == '{') {
      pop(state);
    } else {
      state->error = JSON_ERROR_SYNTAX;
      return JSON_TYPE_ERROR;
    }
    return c;
  case ']':
    if(s == '[') {
      pop(state);
    } else {
      state->error = JSON_ERROR_UNEXPECTED_END_OF_ARRAY;
      return JSON_TYPE_ERROR;
    }
    return c;
  case ':':
    push(state, c);
     // JB patched if (push(state, c) == JSON_TYPE_ERROR) return JSON_TYPE_ERROR;
    return c;
  case ',':
    /* if x:y ... , */
    if(s == ':' && state->vtype != 0) {
      pop(state);
    } else if(s == '[') {
      /* ok! */
    } else {
      state->error = JSON_ERROR_SYNTAX;
      return JSON_TYPE_ERROR;
    }
    return c;
  case '"':
    if(s == '{' || s == '[' || s == ':') {
      atomic(state, c = (s == '{' ? JSON_TYPE_PAIR_NAME : c));
    } else {
      state->error = JSON_ERROR_UNEXPECTED_STRING;
      return JSON_TYPE_ERROR;
    }
    return c;
  case '[':
    if(s == '{' || s == '[' || s == ':') {
      push(state, c);
	// JB */    if (push(state, c) == JSON_TYPE_ERROR) return JSON_TYPE_ERROR;
    } else {
      state->error = JSON_ERROR_UNEXPECTED_ARRAY;
      return JSON_TYPE_ERROR;
    }
    return c;
  default:
    if(s == ':' || s == '[') {
      if(c <= '9' && c >= '0') {
        atomic(state, JSON_TYPE_NUMBER);
        return JSON_TYPE_NUMBER;
      }
    }
  }
  return 0;
}
/*--------------------------------------------------------------------*/
/* get the json value of the current position
 * works only on "atomic" values such as string, number, null, false, true
 */
/*--------------------------------------------------------------------*/
int
jsonparse_copy_value(struct jsonparse_state *state, char *str, int size)
{
  int i;

  if(state->vtype == 0) {
    return 0;
  }
  size = size <= state->vlen ? (size - 1) : state->vlen;
  for(i = 0; i < size; i++) {
    str[i] = state->json[state->vstart + i];
  }
  str[i] = 0;
  return state->vtype;
}
/*--------------------------------------------------------------------*/
int
jsonparse_get_value_as_int(struct jsonparse_state *state)
{
  if(state->vtype != JSON_TYPE_NUMBER) {
    return 0;
  }
  return atoi(&state->json[state->vstart]);
}
/*--------------------------------------------------------------------*/
long
jsonparse_get_value_as_long(struct jsonparse_state *state)
{
  if(state->vtype != JSON_TYPE_NUMBER) {
    return 0;
  }
  return atol(&state->json[state->vstart]);
}
/*--------------------------------------------------------------------*/
/* strcmp - assume no strange chars that needs to be stuffed in string... */
/*--------------------------------------------------------------------*/
int
jsonparse_strcmp_value(struct jsonparse_state *state, const char *str)
{
  if(state->vtype == 0) {
    return -1;
  }
  return strncmp(str, &state->json[state->vstart], state->vlen);
}
/*--------------------------------------------------------------------*/
int
jsonparse_get_len(struct jsonparse_state *state)
{
  return state->vlen;
}
/*--------------------------------------------------------------------*/
int
jsonparse_get_type(struct jsonparse_state *state)
{
  if(state->depth == 0) {
    return 0;
  }
  return state->stack[state->depth - 1];
}
/*--------------------------------------------------------------------*/
int
jsonparse_has_next(struct jsonparse_state *state)
{
  return state->pos < state->len;
}
/*--------------------------------------------------------------------*/
