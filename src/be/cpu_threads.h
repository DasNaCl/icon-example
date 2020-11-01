#pragma once

#ifndef NUM_OF_THREADS
#define NUM_OF_THREADS 4
#endif

#include <stdint.h>

int32_t get_thread_count() { return NUM_OF_THREADS; }

