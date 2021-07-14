#include "hpc/gpu/adreno/a6xx.h"

#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "context.h"
#include "hpc/gpu/base_utilities.h"

//===-------------- BEGIN AUTOGENERATED REGION; DO NOT EDIT! --------------===//

static inline uint32_t adreno_a6xx_counter_get_group(
    hpc_gpu_adreno_a6xx_counter_t counter) {
  return counter >> 8u;
}

static inline uint32_t adreno_a6xx_counter_get_selector(
    hpc_gpu_adreno_a6xx_counter_t counter) {
  return counter & (256u - 1u);
}

//===--------------- END AUTOGENERATED REGION; DO NOT EDIT! ---------------===//

int hpc_gpu_adreno_a6xx_create_context(
    uint32_t num_counters, hpc_gpu_adreno_a6xx_counter_t *counters,
    const hpc_gpu_host_allocation_callbacks_t *allocator,
    hpc_gpu_adreno_context_t **out_context) {
  int status = hpc_gpu_adreno_create_context(num_counters, counters, allocator,
                                             out_context);
  if (status < 0) return status;

  hpc_gpu_adreno_context_t *context = *out_context;
  int gpu_device = context->gpu_device;
  hpc_gpu_adreno_series_t series = hpc_gpu_adreno_get_series(context->gpu_id);

  switch (series) {
    case HPC_GPU_ADRENO_SERIES_UNKNOWN:
      return -HPC_GPU_ERROR_UNKNOWN_DEVICE;
    case HPC_GPU_ADRENO_SERIES_A6XX:
      for (int i = 0; i < num_counters; ++i) {
        uint32_t group_id = adreno_a6xx_counter_get_group(counters[i]);
        uint32_t countable_selector =
            adreno_a6xx_counter_get_selector(counters[i]);
        context->counters[i].group_id = group_id;
        context->counters[i].countable_selector = countable_selector;
        hpc_gpu_adreno_ioctl_activate_counter(gpu_device, group_id,
                                              countable_selector);
      }
      break;
    case HPC_GPU_ADRENO_SERIES_A5XX:
      return -HPC_GPU_ERROR_INCOMPATIBLE_DEVICE;
  }

  return 0;
}

int hpc_gpu_adreno_a6xx_destroy_context(
    hpc_gpu_adreno_context_t *context,
    const hpc_gpu_host_allocation_callbacks_t *allocator) {
  return hpc_gpu_adreno_destroy_context(context, allocator);
}

int hpc_gpu_adreno_a6xx_start_counters(
    const hpc_gpu_adreno_context_t *context) {
  return hpc_gpu_adreno_context_start_counters(context);
}

int hpc_gpu_adreno_a6xx_stop_counters(const hpc_gpu_adreno_context_t *context) {
  return hpc_gpu_adreno_context_stop_counters(context);
}

int hpc_gpu_adreno_a6xx_query_counters(hpc_gpu_adreno_context_t *context,
                                       uint64_t *values) {
  return hpc_gpu_adreno_context_query_counters(context, values);
}