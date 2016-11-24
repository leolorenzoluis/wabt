/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "allocator.h"
#include "vector.h"
#include "option-parser.h"

#define PROGRAM_NAME "wasm-link"
#define NOPE WASM_OPTION_NO_ARGUMENT
#define YEP WASM_OPTION_HAS_ARGUMENT

enum {
  FLAG_VERBOSE,
  FLAG_HELP,
  NUM_FLAGS
};

static const char s_description[] =
    "  link one or more wasm binary modules into a single binary module."
    "\n"
    "  $ wasm-link m1.wasm m2.wasm -o out.wasm\n";

static WasmOption s_options[] = {
    {FLAG_VERBOSE, 'v', "verbose", NULL, NOPE,
     "use multiple times for more info"},
    {FLAG_HELP, 'h', "help", NULL, NOPE, "print this help message"},
};
WASM_STATIC_ASSERT(NUM_FLAGS == WASM_ARRAY_SIZE(s_options));

static WasmBool s_verbose;
typedef const char* String;
WASM_DEFINE_VECTOR(filename, String);
StringVector s_infiles;
WasmAllocator* s_allocator;

static void on_option(struct WasmOptionParser* parser,
                      struct WasmOption* option,
                      const char* argument) {
  switch (option->id) {
    case FLAG_VERBOSE:
      s_verbose++;
      break;

    case FLAG_HELP:
      wasm_print_help(parser, PROGRAM_NAME);
      exit(0);
      break;
  }
}

static void on_argument(struct WasmOptionParser* parser, const char* argument) {
  *wasm_append_filename(s_allocator, &s_infiles) = argument;
}

static void on_option_error(struct WasmOptionParser* parser,
                            const char* message) {
  WASM_FATAL("%s\n", message);
}

static void parse_options(int argc, char** argv) {
  WasmOptionParser parser;
  WASM_ZERO_MEMORY(parser);
  parser.description = s_description;
  parser.options = s_options;
  parser.num_options = WASM_ARRAY_SIZE(s_options);
  parser.on_option = on_option;
  parser.on_argument = on_argument;
  parser.on_error = on_option_error;
  wasm_parse_options(&parser, argc, argv);

  if (!s_infiles.size) {
    wasm_print_help(&parser, PROGRAM_NAME);
    WASM_FATAL("No inputs files specified.\n");
  }

  for (uint i = 0; i < s_infiles.size; i++) {
    printf(" %d : %s\n", i, s_infiles.data[i]);
  }
}

int main(int argc, char** argv) {
  s_allocator = &g_wasm_libc_allocator;
  wasm_init_stdio();
  parse_options(argc, argv);

  WasmResult result = WASM_OK;
  wasm_print_allocator_stats(s_allocator);
  wasm_destroy_allocator(s_allocator);
  return result;
}
