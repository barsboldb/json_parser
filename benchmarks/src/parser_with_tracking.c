// This file includes all parser sources with memory tracking enabled
// It's a single compilation unit where malloc/free are redirected

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>
#include <sys/stat.h>

// Define BENCHMARK_MEMORY_TRACKING before including mem_track.h
#define BENCHMARK_MEMORY_TRACKING
#include "../include/mem_track.h"

// Now include all parser source files
// The malloc/free macros will apply to all of them
#include "../../src/lexer.c"
#include "../../src/parser.c"
#include "../../src/json.c"
