#pragma once

#include <cuda.h>
#include <openmedia/macro.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct OMCudaContext {
  CUcontext cu_context;
} OMCudaContext;

#if defined(__cplusplus)
}
#endif
