#ifndef dosato_parser_h
#define dosato_parser_h

#include "common.h"
#include "node.h"
#include "token.h"
#include "ast.h"

Node parse (const char *source, size_t length, const int start, const int end, TokenList tokens, NodeType type);

#endif