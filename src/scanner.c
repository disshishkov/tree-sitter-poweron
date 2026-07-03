#include "tree_sitter/parser.h"

// C port of the original scanner.cc. Zed's grammar builder only compiles
// src/scanner.c (it ignores C++ scanner.cc), so this file provides the
// external scanner Zed links against. The scanner is stateless.

enum TokenType { COMMENT, STRING_LITERAL, COL, DATASIZE };

static char to_lower(char c) {
  int asciival = (int)c;
  if (asciival < 65 || asciival > 90) {
    return c;
  }
  return (char)(asciival + 32);
}

static bool is_wspace(char c) {
  int ascii = (int)c;
  return ascii == 32 || ascii == 9 || ascii == 10 || ascii == 11 ||
         ascii == 12 || ascii == 13;
}

static bool scan_datasize(TSLexer *lexer) {
  const char datasize[] = "datasize=";
  bool match_found = false;
  for (int i = 0; i < 9; i++) {
    if (i == 8) {
      while (is_wspace(lexer->lookahead) && lexer->lookahead != 0) {
        lexer->advance(lexer, false);
      }
    }
    if (to_lower(lexer->lookahead) == datasize[i]) {
      match_found = true;
      lexer->advance(lexer, false);
    } else {
      match_found = false;
    }
    if (!match_found) {
      break;
    }
  }
  return match_found;
}

static bool scan_col(TSLexer *lexer) {
  const char col[] = "col=";
  bool match_found = false;
  for (int i = 0; i < 4; i++) {
    if (i == 3) {
      while (is_wspace(lexer->lookahead) && lexer->lookahead != 0) {
        lexer->advance(lexer, false);
      }
    }
    if (to_lower(lexer->lookahead) == col[i]) {
      match_found = true;
      lexer->advance(lexer, false);
    } else {
      match_found = false;
    }
    if (!match_found) {
      break;
    }
  }
  return match_found;
}

void *tree_sitter_poweron_external_scanner_create() { return NULL; }

void tree_sitter_poweron_external_scanner_destroy(void *payload) {}

unsigned tree_sitter_poweron_external_scanner_serialize(void *payload,
                                                        char *buffer) {
  return 0;
}

void tree_sitter_poweron_external_scanner_deserialize(void *payload,
                                                      const char *buffer,
                                                      unsigned length) {}

bool tree_sitter_poweron_external_scanner_scan(void *payload, TSLexer *lexer,
                                               const bool *valid_symbols) {
  while (is_wspace(lexer->lookahead)) {
    lexer->advance(lexer, true);
  }

  char next_char = lexer->lookahead;

  if (next_char == '"') {
    lexer->advance(lexer, false);
    while (lexer->lookahead != '"' && lexer->lookahead != 0) {
      lexer->advance(lexer, false);
    }
    lexer->advance(lexer, false);
    lexer->result_symbol = STRING_LITERAL;
    return true;
  }

  if (to_lower(next_char) == 'd') {
    if (scan_datasize(lexer)) {
      lexer->result_symbol = DATASIZE;
      return true;
    }
    return false;
  }

  if (to_lower(next_char) == 'c') {
    if (scan_col(lexer)) {
      lexer->result_symbol = COL;
      return true;
    }
    return false;
  }

  char close_bracket = ']';
  int open_bracket_count = 0;
  int close_bracket_count = 0;

  if (next_char != '[') {
    return false;
  }
  char open_bracket = next_char;

  open_bracket_count += 1;
  lexer->advance(lexer, false);

  for (; lexer->lookahead != 0; lexer->advance(lexer, false)) {
    if (lexer->lookahead == open_bracket) {
      open_bracket_count += 1;
    }
    if (lexer->lookahead == close_bracket) {
      close_bracket_count += 1;
    }
    if (close_bracket_count == open_bracket_count) {
      lexer->result_symbol = COMMENT;
      lexer->advance(lexer, false);
      return true;
    }
  }
  return false;
}
