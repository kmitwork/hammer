#include "parser_internal.h"

typedef struct {
  const HParser *p1;
  const HParser *p2;
} HTwoParsers;


static HParseResult* parse_butnot(void *env, HParseState *state) {
  HTwoParsers *parsers = (HTwoParsers*)env;
  // cache the initial state of the input stream
  HInputStream start_state = state->input_stream;
  HParseResult *r1 = h_do_parse(parsers->p1, state);
  // if p1 failed, bail out early
  if (NULL == r1) {
    return NULL;
  } 
  // cache the state after parse #1, since we might have to back up to it
  HInputStream after_p1_state = state->input_stream;
  state->input_stream = start_state;
  HParseResult *r2 = h_do_parse(parsers->p2, state);
  // TODO(mlp): I'm pretty sure the input stream state should be the post-p1 state in all cases
  state->input_stream = after_p1_state;
  // if p2 failed, restore post-p1 state and bail out early
  if (NULL == r2) {
    return r1;
  }
  size_t r1len = token_length(r1);
  size_t r2len = token_length(r2);
  // if both match but p1's text is shorter than than p2's (or the same length), fail
  if (r1len <= r2len) {
    return NULL;
  } else {
    return r1;
  }
}

static bool bn_isValidCF(void *env) {
  HTwoParsers *tp = (HTwoParsers*)env;
  return (tp->p1->vtable->isValidCF(tp->p1->env) &&
	  tp->p2->vtable->isValidCF(tp->p2->env));
}

static const HParserVtable butnot_vt = {
  .parse = parse_butnot,
  .isValidRegular = h_false,
  .isValidCF = bn_isValidCF,
};

const HParser* h_butnot(const HParser* p1, const HParser* p2) {
  return h_butnot__m(&system_allocator, p1, p2);
}
const HParser* h_butnot__m(HAllocator* mm__, const HParser* p1, const HParser* p2) {
  HTwoParsers *env = h_new(HTwoParsers, 1);
  env->p1 = p1; env->p2 = p2;
  HParser *ret = h_new(HParser, 1);
  ret->vtable = &butnot_vt; ret->env = (void*)env;
  return ret;
}

