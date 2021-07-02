#include "hpc/gpu/adreno/common.h"

#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "hpc/gpu/adreno/driver_ioctl.h"
#include "hpc/gpu/allocation_callback.h"
#include "hpc/gpu/error_code.h"

uint32_t adreno_common_perfcounter_get_group(
    hpc_gpu_adreno_common_perfcounter_t perfcounter);
uint32_t adreno_common_perfcounter_convert_to_a6xx(
    hpc_gpu_adreno_common_perfcounter_t perfcounter);
uint32_t adreno_common_perfcounter_convert_to_a5xx(
    hpc_gpu_adreno_common_perfcounter_t perfcounter);

typedef enum adreno_series_e {
  HPC_GPU_ADRENO_SERIES_UNKNOWN,
  HPC_GPU_ADRENO_SERIES_A6XX,
  HPC_GPU_ADRENO_SERIES_A5XX,
} adreno_series_t;

static adreno_series_t adreno_get_series(int gpu_id) {
  if ((gpu_id >= 600 && gpu_id < 700) || gpu_id == 702)
    return HPC_GPU_ADRENO_SERIES_A6XX;
  if (gpu_id >= 500 && gpu_id < 600) return HPC_GPU_ADRENO_SERIES_A5XX;
  return HPC_GPU_ADRENO_SERIES_UNKNOWN;
}

struct hpc_gpu_adreno_common_context_t {
  adreno_perfcounter_read_counter_t *counters;
  uint64_t *prev_values;
  uint32_t num_counters;
  uint32_t gpu_id;
  int gpu_device;
};

int hpc_gpu_adreno_common_create_context(
    uint32_t num_counters, hpc_gpu_adreno_common_perfcounter_t *counters,
    const hpc_gpu_allocation_callbacks_t *allocator,
    hpc_gpu_adreno_common_context_t **out_context) {
  hpc_gpu_adreno_common_context_t *context = allocator->alloc(
      allocator->user_data, sizeof(hpc_gpu_adreno_common_context_t));

  size_t counter_size =
      num_counters * sizeof(adreno_perfcounter_read_counter_t);
  context->counters = allocator->alloc(allocator->user_data, counter_size);
  memset(context->counters, 0, counter_size);

  size_t init_value_size = num_counters * sizeof(uint64_t);
  context->prev_values =
      allocator->alloc(allocator->user_data, init_value_size);
  memset(context->prev_values, 0, init_value_size);

  context->num_counters = num_counters;

  int gpu_device = adreno_open_gpu_device();
  if (gpu_device < 0) return gpu_device;
  context->gpu_device = gpu_device;

  uint32_t gpu_id = adreno_get_gpu_device_id(context->gpu_device);
  if (gpu_id < 0) return gpu_id;
  context->gpu_id = gpu_id;

  adreno_series_t series = adreno_get_series(gpu_id);

  switch (series) {
    case HPC_GPU_ADRENO_SERIES_UNKNOWN:
      return -HPC_GPU_ERROR_UNKNOWN_DEVICE;
    case HPC_GPU_ADRENO_SERIES_A6XX:
      for (int i = 0; i < num_counters; ++i) {
        uint32_t group_id = adreno_common_perfcounter_get_group(counters[i]);
        uint32_t countable_selector =
            adreno_common_perfcounter_convert_to_a6xx(counters[i]);
        context->counters[i].group_id = group_id;
        context->counters[i].countable_selector = countable_selector;
        andreno_activate_perfcounter(gpu_device, group_id, countable_selector);
      }
      break;
    case HPC_GPU_ADRENO_SERIES_A5XX:
      for (int i = 0; i < num_counters; ++i) {
        uint32_t group_id = adreno_common_perfcounter_get_group(counters[i]);
        uint32_t countable_selector =
            adreno_common_perfcounter_convert_to_a5xx(counters[i]);
        context->counters[i].group_id = group_id;
        context->counters[i].countable_selector = countable_selector;
        andreno_activate_perfcounter(gpu_device, group_id, countable_selector);
      }
      break;
  }

  *out_context = context;
  return 0;
}

int hpc_gpu_adreno_common_destroy_context(
    hpc_gpu_adreno_common_context_t *context,
    const hpc_gpu_allocation_callbacks_t *allocator) {
  int status = close(context->gpu_device);
  if (status < 0) return status;

  allocator->free(allocator->user_data, context->prev_values);
  allocator->free(allocator->user_data, context->counters);
  allocator->free(allocator->user_data, context);
  return 0;
}

int hpc_gpu_adreno_common_start_perfcounters(
    hpc_gpu_adreno_common_context_t *context) {
  // Activate all selected perfcounters
  adreno_series_t series = adreno_get_series(context->gpu_id);
  switch (series) {
    case HPC_GPU_ADRENO_SERIES_UNKNOWN:
      // Context creation should guard against this so it should not happen
      // here.
      return 0;
    case HPC_GPU_ADRENO_SERIES_A6XX:
      for (int i = 0; i < context->num_counters; ++i) {
        int status = andreno_activate_perfcounter(
            context->gpu_device, context->counters[i].group_id,
            context->counters[i].countable_selector);
        if (status < 0) return status;
      }
      break;
    case HPC_GPU_ADRENO_SERIES_A5XX:
      for (int i = 0; i < context->num_counters; ++i) {
        int status = andreno_activate_perfcounter(
            context->gpu_device, context->counters[i].group_id,
            context->counters[i].countable_selector);
        if (status < 0) return status;
      }
      break;
  }

  // Query their initial values
  int status =
      adreno_query_perfcounters(context->gpu_device, context->num_counters,
                                context->counters, context->prev_values);
  if (status < 0) return status;

  return 0;
}

int hpc_gpu_adreno_common_stop_perfcounters(
    hpc_gpu_adreno_common_context_t *context) {
  adreno_series_t series = adreno_get_series(context->gpu_id);
  switch (series) {
    case HPC_GPU_ADRENO_SERIES_UNKNOWN:
      // Context creation should guard against this so it should not happen
      // here.
      return 0;
    case HPC_GPU_ADRENO_SERIES_A6XX:
      for (int i = 0; i < context->num_counters; ++i) {
        int status = andreno_deactivate_perfcounter(
            context->gpu_device, context->counters[i].group_id,
            context->counters[i].countable_selector);
        if (status < 0) return status;
      }
      break;
    case HPC_GPU_ADRENO_SERIES_A5XX:
      for (int i = 0; i < context->num_counters; ++i) {
        int status = andreno_deactivate_perfcounter(
            context->gpu_device, context->counters[i].group_id,
            context->counters[i].countable_selector);
        if (status < 0) return status;
      }
      break;
  }
  return 0;
}

int hpc_gpu_adreno_common_query_perfcounters(
    hpc_gpu_adreno_common_context_t *context, uint64_t *values) {
  int status = adreno_query_perfcounters(
      context->gpu_device, context->num_counters, context->counters, values);
  for (int i = 0; i < context->num_counters; ++i) {
    uint64_t value = values[i];
    // Counters increase linearly. We need to subtract the previous value.
    values[i] -= context->prev_values[i];
    context->prev_values[i] = value;
  }

  if (status < 0) return status;
  return 0;
}

//===-------------- BEGIN AUTOGENERATED REGION; DO NOT EDIT! --------------===//

inline uint32_t adreno_common_perfcounter_get_group(
    hpc_gpu_adreno_common_perfcounter_t perfcounter) {
  return perfcounter >> 8u;
}

uint32_t adreno_common_perfcounter_convert_to_a6xx(
    hpc_gpu_adreno_common_perfcounter_t perfcounter) {
  // clang-format off
  switch (perfcounter) {
    case HPC_GPU_ADRENO_COMMON_CP_ALWAYS_COUNT: return 0;
    case HPC_GPU_ADRENO_COMMON_CP_BUSY_GFX_CORE_IDLE: return 1;
    case HPC_GPU_ADRENO_COMMON_CP_BUSY_CYCLES: return 2;
    case HPC_GPU_ADRENO_COMMON_CP_NUM_PREEMPTIONS: return 3;
    case HPC_GPU_ADRENO_COMMON_CP_PREEMPTION_REACTION_DELAY: return 4;
    case HPC_GPU_ADRENO_COMMON_CP_PREEMPTION_SWITCH_OUT_TIME: return 5;
    case HPC_GPU_ADRENO_COMMON_CP_PREEMPTION_SWITCH_IN_TIME: return 6;
    case HPC_GPU_ADRENO_COMMON_CP_DEAD_DRAWS_IN_BIN_RENDER: return 7;
    case HPC_GPU_ADRENO_COMMON_CP_PREDICATED_DRAWS_KILLED: return 8;
    case HPC_GPU_ADRENO_COMMON_CP_MODE_SWITCH: return 9;
    case HPC_GPU_ADRENO_COMMON_CP_ZPASS_DONE: return 10;
    case HPC_GPU_ADRENO_COMMON_CP_CONTEXT_DONE: return 11;
    case HPC_GPU_ADRENO_COMMON_CP_CACHE_FLUSH: return 12;
    case HPC_GPU_ADRENO_COMMON_CP_LONG_PREEMPTIONS: return 13;
    case HPC_GPU_ADRENO_COMMON_RBBM_ALWAYS_COUNT: return 0;
    case HPC_GPU_ADRENO_COMMON_RBBM_ALWAYS_ON: return 1;
    case HPC_GPU_ADRENO_COMMON_RBBM_TSE_BUSY: return 2;
    case HPC_GPU_ADRENO_COMMON_RBBM_RAS_BUSY: return 3;
    case HPC_GPU_ADRENO_COMMON_RBBM_PC_DCALL_BUSY: return 4;
    case HPC_GPU_ADRENO_COMMON_RBBM_PC_VSD_BUSY: return 5;
    case HPC_GPU_ADRENO_COMMON_RBBM_STATUS_MASKED: return 6;
    case HPC_GPU_ADRENO_COMMON_RBBM_COM_BUSY: return 7;
    case HPC_GPU_ADRENO_COMMON_RBBM_DCOM_BUSY: return 8;
    case HPC_GPU_ADRENO_COMMON_RBBM_VBIF_BUSY: return 9;
    case HPC_GPU_ADRENO_COMMON_RBBM_VSC_BUSY: return 10;
    case HPC_GPU_ADRENO_COMMON_RBBM_TESS_BUSY: return 11;
    case HPC_GPU_ADRENO_COMMON_RBBM_UCHE_BUSY: return 12;
    case HPC_GPU_ADRENO_COMMON_RBBM_HLSQ_BUSY: return 13;
    case HPC_GPU_ADRENO_COMMON_PC_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_PC_WORKING_CYCLES: return 1;
    case HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_VFD: return 2;
    case HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_TSE: return 3;
    case HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_VPC: return 4;
    case HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_UCHE: return 5;
    case HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_TESS: return 6;
    case HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_TSE_ONLY: return 7;
    case HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_VPC_ONLY: return 8;
    case HPC_GPU_ADRENO_COMMON_PC_PASS1_TF_STALL_CYCLES: return 9;
    case HPC_GPU_ADRENO_COMMON_PC_STARVE_CYCLES_FOR_INDEX: return 10;
    case HPC_GPU_ADRENO_COMMON_PC_STARVE_CYCLES_FOR_TESS_FACTOR: return 11;
    case HPC_GPU_ADRENO_COMMON_PC_STARVE_CYCLES_FOR_VIZ_STREAM: return 12;
    case HPC_GPU_ADRENO_COMMON_PC_STARVE_CYCLES_FOR_POSITION: return 13;
    case HPC_GPU_ADRENO_COMMON_PC_STARVE_CYCLES_DI: return 14;
    case HPC_GPU_ADRENO_COMMON_PC_VIS_STREAMS_LOADED: return 15;
    case HPC_GPU_ADRENO_COMMON_PC_INSTANCES: return 16;
    case HPC_GPU_ADRENO_COMMON_PC_VPC_PRIMITIVES: return 17;
    case HPC_GPU_ADRENO_COMMON_PC_DEAD_PRIM: return 18;
    case HPC_GPU_ADRENO_COMMON_PC_LIVE_PRIM: return 19;
    case HPC_GPU_ADRENO_COMMON_PC_VERTEX_HITS: return 20;
    case HPC_GPU_ADRENO_COMMON_PC_IA_VERTICES: return 21;
    case HPC_GPU_ADRENO_COMMON_PC_IA_PRIMITIVES: return 22;
    case HPC_GPU_ADRENO_COMMON_PC_GS_PRIMITIVES: return 23;
    case HPC_GPU_ADRENO_COMMON_PC_HS_INVOCATIONS: return 24;
    case HPC_GPU_ADRENO_COMMON_PC_DS_INVOCATIONS: return 25;
    case HPC_GPU_ADRENO_COMMON_PC_VS_INVOCATIONS: return 26;
    case HPC_GPU_ADRENO_COMMON_PC_GS_INVOCATIONS: return 27;
    case HPC_GPU_ADRENO_COMMON_PC_DS_PRIMITIVES: return 28;
    case HPC_GPU_ADRENO_COMMON_PC_VPC_POS_DATA_TRANSACTION: return 29;
    case HPC_GPU_ADRENO_COMMON_PC_3D_DRAWCALLS: return 30;
    case HPC_GPU_ADRENO_COMMON_PC_2D_DRAWCALLS: return 31;
    case HPC_GPU_ADRENO_COMMON_PC_NON_DRAWCALL_GLOBAL_EVENTS: return 32;
    case HPC_GPU_ADRENO_COMMON_PC_TESS_BUSY_CYCLES: return 33;
    case HPC_GPU_ADRENO_COMMON_PC_TESS_WORKING_CYCLES: return 34;
    case HPC_GPU_ADRENO_COMMON_PC_TESS_STALL_CYCLES_PC: return 35;
    case HPC_GPU_ADRENO_COMMON_PC_TESS_STARVE_CYCLES_PC: return 36;
    case HPC_GPU_ADRENO_COMMON_VFD_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_VFD_STALL_CYCLES_UCHE: return 1;
    case HPC_GPU_ADRENO_COMMON_VFD_STALL_CYCLES_VPC_ALLOC: return 2;
    case HPC_GPU_ADRENO_COMMON_VFD_STALL_CYCLES_SP_INFO: return 3;
    case HPC_GPU_ADRENO_COMMON_VFD_STALL_CYCLES_SP_ATTR: return 4;
    case HPC_GPU_ADRENO_COMMON_VFD_STARVE_CYCLES_UCHE: return 5;
    case HPC_GPU_ADRENO_COMMON_VFD_RBUFFER_FULL: return 6;
    case HPC_GPU_ADRENO_COMMON_VFD_ATTR_INFO_FIFO_FULL: return 7;
    case HPC_GPU_ADRENO_COMMON_VFD_DECODED_ATTRIBUTE_BYTES: return 8;
    case HPC_GPU_ADRENO_COMMON_VFD_NUM_ATTRIBUTES: return 9;
    case HPC_GPU_ADRENO_COMMON_VFD_UPPER_SHADER_FIBERS: return 10;
    case HPC_GPU_ADRENO_COMMON_VFD_LOWER_SHADER_FIBERS: return 11;
    case HPC_GPU_ADRENO_COMMON_VFD_MODE_0_FIBERS: return 12;
    case HPC_GPU_ADRENO_COMMON_VFD_MODE_1_FIBERS: return 13;
    case HPC_GPU_ADRENO_COMMON_VFD_MODE_2_FIBERS: return 14;
    case HPC_GPU_ADRENO_COMMON_VFD_MODE_3_FIBERS: return 15;
    case HPC_GPU_ADRENO_COMMON_VFD_MODE_4_FIBERS: return 16;
    case HPC_GPU_ADRENO_COMMON_VFD_TOTAL_VERTICES: return 17;
    case HPC_GPU_ADRENO_COMMON_VFD_VFDP_STALL_CYCLES_VFD: return 18;
    case HPC_GPU_ADRENO_COMMON_VFD_VFDP_STALL_CYCLES_VFD_INDEX: return 19;
    case HPC_GPU_ADRENO_COMMON_VFD_VFDP_STALL_CYCLES_VFD_PROG: return 20;
    case HPC_GPU_ADRENO_COMMON_VFD_VFDP_STARVE_CYCLES_PC: return 21;
    case HPC_GPU_ADRENO_COMMON_HLSQ_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_HLSQ_STALL_CYCLES_UCHE: return 1;
    case HPC_GPU_ADRENO_COMMON_HLSQ_STALL_CYCLES_SP_STATE: return 2;
    case HPC_GPU_ADRENO_COMMON_HLSQ_STALL_CYCLES_SP_FS_STAGE: return 3;
    case HPC_GPU_ADRENO_COMMON_HLSQ_UCHE_LATENCY_CYCLES: return 4;
    case HPC_GPU_ADRENO_COMMON_HLSQ_UCHE_LATENCY_COUNT: return 5;
    case HPC_GPU_ADRENO_COMMON_HLSQ_QUADS: return 8;
    case HPC_GPU_ADRENO_COMMON_HLSQ_CS_INVOCATIONS: return 9;
    case HPC_GPU_ADRENO_COMMON_HLSQ_COMPUTE_DRAWCALLS: return 10;
    case HPC_GPU_ADRENO_COMMON_VPC_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_VPC_WORKING_CYCLES: return 1;
    case HPC_GPU_ADRENO_COMMON_VPC_STALL_CYCLES_UCHE: return 2;
    case HPC_GPU_ADRENO_COMMON_VPC_STALL_CYCLES_VFD_WACK: return 3;
    case HPC_GPU_ADRENO_COMMON_VPC_STALL_CYCLES_HLSQ_PRIM_ALLOC: return 4;
    case HPC_GPU_ADRENO_COMMON_VPC_STALL_CYCLES_PC: return 5;
    case HPC_GPU_ADRENO_COMMON_VPC_STALL_CYCLES_SP_LM: return 6;
    case HPC_GPU_ADRENO_COMMON_VPC_STARVE_CYCLES_SP: return 7;
    case HPC_GPU_ADRENO_COMMON_VPC_STARVE_CYCLES_LRZ: return 8;
    case HPC_GPU_ADRENO_COMMON_VPC_PC_PRIMITIVES: return 9;
    case HPC_GPU_ADRENO_COMMON_VPC_SP_COMPONENTS: return 10;
    case HPC_GPU_ADRENO_COMMON_TSE_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_TSE_CLIPPING_CYCLES: return 1;
    case HPC_GPU_ADRENO_COMMON_TSE_STALL_CYCLES_RAS: return 2;
    case HPC_GPU_ADRENO_COMMON_TSE_STALL_CYCLES_LRZ_BARYPLANE: return 3;
    case HPC_GPU_ADRENO_COMMON_TSE_STALL_CYCLES_LRZ_ZPLANE: return 4;
    case HPC_GPU_ADRENO_COMMON_TSE_STARVE_CYCLES_PC: return 5;
    case HPC_GPU_ADRENO_COMMON_TSE_INPUT_PRIM: return 6;
    case HPC_GPU_ADRENO_COMMON_TSE_INPUT_NULL_PRIM: return 7;
    case HPC_GPU_ADRENO_COMMON_TSE_TRIVAL_REJ_PRIM: return 8;
    case HPC_GPU_ADRENO_COMMON_TSE_CLIPPED_PRIM: return 9;
    case HPC_GPU_ADRENO_COMMON_TSE_ZERO_AREA_PRIM: return 10;
    case HPC_GPU_ADRENO_COMMON_TSE_FACENESS_CULLED_PRIM: return 11;
    case HPC_GPU_ADRENO_COMMON_TSE_ZERO_PIXEL_PRIM: return 12;
    case HPC_GPU_ADRENO_COMMON_TSE_OUTPUT_NULL_PRIM: return 13;
    case HPC_GPU_ADRENO_COMMON_TSE_OUTPUT_VISIBLE_PRIM: return 14;
    case HPC_GPU_ADRENO_COMMON_TSE_CINVOCATION: return 15;
    case HPC_GPU_ADRENO_COMMON_TSE_CPRIMITIVES: return 16;
    case HPC_GPU_ADRENO_COMMON_TSE_2D_INPUT_PRIM: return 17;
    case HPC_GPU_ADRENO_COMMON_RAS_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_RAS_SUPERTILE_ACTIVE_CYCLES: return 1;
    case HPC_GPU_ADRENO_COMMON_RAS_STALL_CYCLES_LRZ: return 2;
    case HPC_GPU_ADRENO_COMMON_RAS_STARVE_CYCLES_TSE: return 3;
    case HPC_GPU_ADRENO_COMMON_RAS_SUPER_TILES: return 4;
    case HPC_GPU_ADRENO_COMMON_RAS_8X4_TILES: return 5;
    case HPC_GPU_ADRENO_COMMON_RAS_MASKGEN_ACTIVE: return 6;
    case HPC_GPU_ADRENO_COMMON_RAS_FULLY_COVERED_SUPER_TILES: return 7;
    case HPC_GPU_ADRENO_COMMON_RAS_FULLY_COVERED_8X4_TILES: return 8;
    case HPC_GPU_ADRENO_COMMON_RAS_PRIM_KILLED_INVISILBE: return 9;
    case HPC_GPU_ADRENO_COMMON_UCHE_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_LATENCY_CYCLES: return 2;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_LATENCY_SAMPLES: return 3;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_TP: return 4;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_VFD: return 5;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_HLSQ: return 6;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_LRZ: return 7;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_SP: return 8;
    case HPC_GPU_ADRENO_COMMON_UCHE_READ_REQUESTS_TP: return 9;
    case HPC_GPU_ADRENO_COMMON_UCHE_READ_REQUESTS_VFD: return 10;
    case HPC_GPU_ADRENO_COMMON_UCHE_READ_REQUESTS_HLSQ: return 11;
    case HPC_GPU_ADRENO_COMMON_UCHE_READ_REQUESTS_LRZ: return 12;
    case HPC_GPU_ADRENO_COMMON_UCHE_READ_REQUESTS_SP: return 13;
    case HPC_GPU_ADRENO_COMMON_UCHE_WRITE_REQUESTS_LRZ: return 14;
    case HPC_GPU_ADRENO_COMMON_UCHE_WRITE_REQUESTS_SP: return 15;
    case HPC_GPU_ADRENO_COMMON_UCHE_WRITE_REQUESTS_VPC: return 16;
    case HPC_GPU_ADRENO_COMMON_UCHE_WRITE_REQUESTS_VSC: return 17;
    case HPC_GPU_ADRENO_COMMON_UCHE_EVICTS: return 18;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ0: return 19;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ1: return 20;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ2: return 21;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ3: return 22;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ4: return 23;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ5: return 24;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ6: return 25;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ7: return 26;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_CH0: return 27;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_CH1: return 28;
    case HPC_GPU_ADRENO_COMMON_UCHE_GMEM_READ_BEATS: return 29;
    case HPC_GPU_ADRENO_COMMON_TP_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_TP_STALL_CYCLES_UCHE: return 1;
    case HPC_GPU_ADRENO_COMMON_TP_LATENCY_CYCLES: return 2;
    case HPC_GPU_ADRENO_COMMON_TP_LATENCY_TRANS: return 3;
    case HPC_GPU_ADRENO_COMMON_TP_FLAG_CACHE_REQUEST_SAMPLES: return 4;
    case HPC_GPU_ADRENO_COMMON_TP_FLAG_CACHE_REQUEST_LATENCY: return 5;
    case HPC_GPU_ADRENO_COMMON_TP_L1_CACHELINE_REQUESTS: return 6;
    case HPC_GPU_ADRENO_COMMON_TP_L1_CACHELINE_MISSES: return 7;
    case HPC_GPU_ADRENO_COMMON_TP_SP_TP_TRANS: return 8;
    case HPC_GPU_ADRENO_COMMON_TP_TP_SP_TRANS: return 9;
    case HPC_GPU_ADRENO_COMMON_TP_OUTPUT_PIXELS: return 10;
    case HPC_GPU_ADRENO_COMMON_TP_FILTER_WORKLOAD_16BIT: return 11;
    case HPC_GPU_ADRENO_COMMON_TP_FILTER_WORKLOAD_32BIT: return 12;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_RECEIVED: return 13;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_OFFSET: return 14;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_SHADOW: return 15;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_ARRAY: return 16;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_GRADIENT: return 17;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_1D: return 18;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_2D: return 19;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_BUFFER: return 20;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_3D: return 21;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_CUBE: return 22;
    case HPC_GPU_ADRENO_COMMON_TP_DIVERGENT_QUADS_RECEIVED: return 23;
    case HPC_GPU_ADRENO_COMMON_TP_PRT_NON_RESIDENT_EVENTS: return 24;
    case HPC_GPU_ADRENO_COMMON_TP_OUTPUT_PIXELS_POINT: return 25;
    case HPC_GPU_ADRENO_COMMON_TP_OUTPUT_PIXELS_BILINEAR: return 26;
    case HPC_GPU_ADRENO_COMMON_TP_OUTPUT_PIXELS_MIP: return 27;
    case HPC_GPU_ADRENO_COMMON_TP_OUTPUT_PIXELS_ANISO: return 28;
    case HPC_GPU_ADRENO_COMMON_TP_OUTPUT_PIXELS_ZERO_LOD: return 29;
    case HPC_GPU_ADRENO_COMMON_TP_FLAG_CACHE_REQUESTS: return 30;
    case HPC_GPU_ADRENO_COMMON_TP_FLAG_CACHE_MISSES: return 31;
    case HPC_GPU_ADRENO_COMMON_TP_L1_5_L2_REQUESTS: return 32;
    case HPC_GPU_ADRENO_COMMON_TP_2D_OUTPUT_PIXELS: return 33;
    case HPC_GPU_ADRENO_COMMON_TP_2D_OUTPUT_PIXELS_POINT: return 34;
    case HPC_GPU_ADRENO_COMMON_TP_2D_OUTPUT_PIXELS_BILINEAR: return 35;
    case HPC_GPU_ADRENO_COMMON_TP_2D_FILTER_WORKLOAD_16BIT: return 36;
    case HPC_GPU_ADRENO_COMMON_TP_2D_FILTER_WORKLOAD_32BIT: return 37;
    case HPC_GPU_ADRENO_COMMON_SP_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_SP_ALU_WORKING_CYCLES: return 1;
    case HPC_GPU_ADRENO_COMMON_SP_EFU_WORKING_CYCLES: return 2;
    case HPC_GPU_ADRENO_COMMON_SP_STALL_CYCLES_VPC: return 3;
    case HPC_GPU_ADRENO_COMMON_SP_STALL_CYCLES_TP: return 4;
    case HPC_GPU_ADRENO_COMMON_SP_STALL_CYCLES_UCHE: return 5;
    case HPC_GPU_ADRENO_COMMON_SP_STALL_CYCLES_RB: return 6;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_CONTEXTS: return 8;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_CONTEXT_CYCLES: return 9;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_WAVE_CYCLES: return 10;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_WAVE_SAMPLES: return 11;
    case HPC_GPU_ADRENO_COMMON_SP_VS_STAGE_WAVE_CYCLES: return 12;
    case HPC_GPU_ADRENO_COMMON_SP_VS_STAGE_WAVE_SAMPLES: return 13;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_DURATION_CYCLES: return 14;
    case HPC_GPU_ADRENO_COMMON_SP_VS_STAGE_DURATION_CYCLES: return 15;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_CTRL_CYCLES: return 16;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_LOAD_CYCLES: return 17;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_EMIT_CYCLES: return 18;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_NOP_CYCLES: return 19;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_WAIT_CYCLES: return 20;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_FETCH_CYCLES: return 21;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_IDLE_CYCLES: return 22;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_END_CYCLES: return 23;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_LONG_SYNC_CYCLES: return 24;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_SHORT_SYNC_CYCLES: return 25;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_JOIN_CYCLES: return 26;
    case HPC_GPU_ADRENO_COMMON_SP_LM_LOAD_INSTRUCTIONS: return 27;
    case HPC_GPU_ADRENO_COMMON_SP_LM_STORE_INSTRUCTIONS: return 28;
    case HPC_GPU_ADRENO_COMMON_SP_LM_ATOMICS: return 29;
    case HPC_GPU_ADRENO_COMMON_SP_GM_LOAD_INSTRUCTIONS: return 30;
    case HPC_GPU_ADRENO_COMMON_SP_GM_STORE_INSTRUCTIONS: return 31;
    case HPC_GPU_ADRENO_COMMON_SP_GM_ATOMICS: return 32;
    case HPC_GPU_ADRENO_COMMON_SP_VS_STAGE_TEX_INSTRUCTIONS: return 33;
    case HPC_GPU_ADRENO_COMMON_SP_VS_STAGE_EFU_INSTRUCTIONS: return 34;
    case HPC_GPU_ADRENO_COMMON_SP_VS_STAGE_FULL_ALU_INSTRUCTIONS: return 35;
    case HPC_GPU_ADRENO_COMMON_SP_VS_STAGE_HALF_ALU_INSTRUCTIONS: return 36;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_TEX_INSTRUCTIONS: return 37;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_CFLOW_INSTRUCTIONS: return 38;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_EFU_INSTRUCTIONS: return 39;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_FULL_ALU_INSTRUCTIONS: return 40;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_HALF_ALU_INSTRUCTIONS: return 41;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_BARY_INSTRUCTIONS: return 42;
    case HPC_GPU_ADRENO_COMMON_SP_VS_INSTRUCTIONS: return 43;
    case HPC_GPU_ADRENO_COMMON_SP_FS_INSTRUCTIONS: return 44;
    case HPC_GPU_ADRENO_COMMON_SP_ADDR_LOCK_COUNT: return 45;
    case HPC_GPU_ADRENO_COMMON_SP_UCHE_READ_TRANS: return 46;
    case HPC_GPU_ADRENO_COMMON_SP_UCHE_WRITE_TRANS: return 47;
    case HPC_GPU_ADRENO_COMMON_SP_EXPORT_VPC_TRANS: return 48;
    case HPC_GPU_ADRENO_COMMON_SP_EXPORT_RB_TRANS: return 49;
    case HPC_GPU_ADRENO_COMMON_SP_PIXELS_KILLED: return 50;
    case HPC_GPU_ADRENO_COMMON_SP_ICL1_REQUESTS: return 51;
    case HPC_GPU_ADRENO_COMMON_SP_ICL1_MISSES: return 52;
    case HPC_GPU_ADRENO_COMMON_SP_HS_INSTRUCTIONS: return 53;
    case HPC_GPU_ADRENO_COMMON_SP_DS_INSTRUCTIONS: return 54;
    case HPC_GPU_ADRENO_COMMON_SP_GS_INSTRUCTIONS: return 55;
    case HPC_GPU_ADRENO_COMMON_SP_CS_INSTRUCTIONS: return 56;
    case HPC_GPU_ADRENO_COMMON_SP_GPR_READ: return 57;
    case HPC_GPU_ADRENO_COMMON_SP_GPR_WRITE: return 58;
    case HPC_GPU_ADRENO_COMMON_SP_LM_BANK_CONFLICTS: return 61;
    case HPC_GPU_ADRENO_COMMON_RB_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_RB_STALL_CYCLES_HLSQ: return 1;
    case HPC_GPU_ADRENO_COMMON_RB_STALL_CYCLES_FIFO0_FULL: return 2;
    case HPC_GPU_ADRENO_COMMON_RB_STALL_CYCLES_FIFO1_FULL: return 3;
    case HPC_GPU_ADRENO_COMMON_RB_STALL_CYCLES_FIFO2_FULL: return 4;
    case HPC_GPU_ADRENO_COMMON_RB_STARVE_CYCLES_SP: return 5;
    case HPC_GPU_ADRENO_COMMON_RB_STARVE_CYCLES_LRZ_TILE: return 6;
    case HPC_GPU_ADRENO_COMMON_RB_STARVE_CYCLES_CCU: return 7;
    case HPC_GPU_ADRENO_COMMON_RB_STARVE_CYCLES_Z_PLANE: return 8;
    case HPC_GPU_ADRENO_COMMON_RB_STARVE_CYCLES_BARY_PLANE: return 9;
    case HPC_GPU_ADRENO_COMMON_RB_Z_WORKLOAD: return 10;
    case HPC_GPU_ADRENO_COMMON_RB_HLSQ_ACTIVE: return 11;
    case HPC_GPU_ADRENO_COMMON_RB_Z_READ: return 12;
    case HPC_GPU_ADRENO_COMMON_RB_Z_WRITE: return 13;
    case HPC_GPU_ADRENO_COMMON_RB_C_READ: return 14;
    case HPC_GPU_ADRENO_COMMON_RB_C_WRITE: return 15;
    case HPC_GPU_ADRENO_COMMON_RB_TOTAL_PASS: return 16;
    case HPC_GPU_ADRENO_COMMON_RB_Z_PASS: return 17;
    case HPC_GPU_ADRENO_COMMON_RB_Z_FAIL: return 18;
    case HPC_GPU_ADRENO_COMMON_RB_S_FAIL: return 19;
    case HPC_GPU_ADRENO_COMMON_RB_BLENDED_FXP_COMPONENTS: return 20;
    case HPC_GPU_ADRENO_COMMON_RB_BLENDED_FP16_COMPONENTS: return 21;
    case HPC_GPU_ADRENO_COMMON_RB_2D_ALIVE_CYCLES: return 23;
    case HPC_GPU_ADRENO_COMMON_RB_2D_STALL_CYCLES_A2D: return 24;
    case HPC_GPU_ADRENO_COMMON_RB_2D_STARVE_CYCLES_SRC: return 25;
    case HPC_GPU_ADRENO_COMMON_RB_2D_STARVE_CYCLES_SP: return 26;
    case HPC_GPU_ADRENO_COMMON_RB_2D_STARVE_CYCLES_DST: return 27;
    case HPC_GPU_ADRENO_COMMON_RB_2D_VALID_PIXELS: return 28;
    case HPC_GPU_ADRENO_COMMON_RB_STALL_CYCLES_CCU: return 44;
    case HPC_GPU_ADRENO_COMMON_VSC_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_VSC_WORKING_CYCLES: return 1;
    case HPC_GPU_ADRENO_COMMON_VSC_STALL_CYCLES_UCHE: return 2;
    case HPC_GPU_ADRENO_COMMON_VSC_EOT_NUM: return 3;
    case HPC_GPU_ADRENO_COMMON_CCU_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_CCU_STALL_CYCLES_RB_DEPTH_RETURN: return 1;
    case HPC_GPU_ADRENO_COMMON_CCU_STALL_CYCLES_RB_COLOR_RETURN: return 2;
    case HPC_GPU_ADRENO_COMMON_CCU_STARVE_CYCLES_FLAG_RETURN: return 3;
    case HPC_GPU_ADRENO_COMMON_CCU_DEPTH_BLOCKS: return 4;
    case HPC_GPU_ADRENO_COMMON_CCU_COLOR_BLOCKS: return 5;
    case HPC_GPU_ADRENO_COMMON_CCU_DEPTH_BLOCK_HIT: return 6;
    case HPC_GPU_ADRENO_COMMON_CCU_COLOR_BLOCK_HIT: return 7;
    case HPC_GPU_ADRENO_COMMON_CCU_PARTIAL_BLOCK_READ: return 8;
    case HPC_GPU_ADRENO_COMMON_CCU_GMEM_READ: return 9;
    case HPC_GPU_ADRENO_COMMON_CCU_GMEM_WRITE: return 10;
    case HPC_GPU_ADRENO_COMMON_CCU_DEPTH_READ_FLAG0_COUNT: return 11;
    case HPC_GPU_ADRENO_COMMON_CCU_DEPTH_READ_FLAG1_COUNT: return 12;
    case HPC_GPU_ADRENO_COMMON_CCU_DEPTH_READ_FLAG2_COUNT: return 13;
    case HPC_GPU_ADRENO_COMMON_CCU_DEPTH_READ_FLAG3_COUNT: return 14;
    case HPC_GPU_ADRENO_COMMON_CCU_DEPTH_READ_FLAG4_COUNT: return 15;
    case HPC_GPU_ADRENO_COMMON_CCU_COLOR_READ_FLAG0_COUNT: return 19;
    case HPC_GPU_ADRENO_COMMON_CCU_COLOR_READ_FLAG1_COUNT: return 20;
    case HPC_GPU_ADRENO_COMMON_CCU_COLOR_READ_FLAG2_COUNT: return 21;
    case HPC_GPU_ADRENO_COMMON_CCU_COLOR_READ_FLAG3_COUNT: return 22;
    case HPC_GPU_ADRENO_COMMON_CCU_COLOR_READ_FLAG4_COUNT: return 23;
    case HPC_GPU_ADRENO_COMMON_CCU_2D_RD_REQ: return 27;
    case HPC_GPU_ADRENO_COMMON_CCU_2D_WR_REQ: return 28;
    case HPC_GPU_ADRENO_COMMON_LRZ_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_LRZ_STARVE_CYCLES_RAS: return 1;
    case HPC_GPU_ADRENO_COMMON_LRZ_STALL_CYCLES_RB: return 2;
    case HPC_GPU_ADRENO_COMMON_LRZ_STALL_CYCLES_VSC: return 3;
    case HPC_GPU_ADRENO_COMMON_LRZ_STALL_CYCLES_VPC: return 4;
    case HPC_GPU_ADRENO_COMMON_LRZ_STALL_CYCLES_FLAG_PREFETCH: return 5;
    case HPC_GPU_ADRENO_COMMON_LRZ_STALL_CYCLES_UCHE: return 6;
    case HPC_GPU_ADRENO_COMMON_LRZ_LRZ_READ: return 7;
    case HPC_GPU_ADRENO_COMMON_LRZ_LRZ_WRITE: return 8;
    case HPC_GPU_ADRENO_COMMON_LRZ_READ_LATENCY: return 9;
    case HPC_GPU_ADRENO_COMMON_LRZ_MERGE_CACHE_UPDATING: return 10;
    case HPC_GPU_ADRENO_COMMON_LRZ_PRIM_KILLED_BY_MASKGEN: return 11;
    case HPC_GPU_ADRENO_COMMON_LRZ_PRIM_KILLED_BY_LRZ: return 12;
    case HPC_GPU_ADRENO_COMMON_LRZ_VISIBLE_PRIM_AFTER_LRZ: return 13;
    case HPC_GPU_ADRENO_COMMON_LRZ_FULL_8X8_TILES: return 14;
    case HPC_GPU_ADRENO_COMMON_LRZ_PARTIAL_8X8_TILES: return 15;
    case HPC_GPU_ADRENO_COMMON_LRZ_TILE_KILLED: return 16;
    case HPC_GPU_ADRENO_COMMON_LRZ_TOTAL_PIXEL: return 17;
    case HPC_GPU_ADRENO_COMMON_LRZ_VISIBLE_PIXEL_AFTER_LRZ: return 18;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_LATENCY_CYCLES: return 1;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_LATENCY_SAMPLES: return 2;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_READ_DATA_CCU: return 3;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_WRITE_DATA_CCU: return 4;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_READ_REQUEST: return 5;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_WRITE_REQUEST: return 6;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_READ_DATA: return 7;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_WRITE_DATA: return 8;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_FLAG_FETCH_CYCLES: return 9;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_FLAG_FETCH_SAMPLES: return 10;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_DEPTH_WRITE_FLAG1_COUNT: return 11;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_DEPTH_WRITE_FLAG2_COUNT: return 12;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_DEPTH_WRITE_FLAG3_COUNT: return 13;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_DEPTH_WRITE_FLAG4_COUNT: return 14;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_COLOR_WRITE_FLAG1_COUNT: return 18;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_COLOR_WRITE_FLAG2_COUNT: return 19;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_COLOR_WRITE_FLAG3_COUNT: return 20;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_COLOR_WRITE_FLAG4_COUNT: return 21;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_2D_STALL_CYCLES_VBIF_REQ: return 25;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_2D_STALL_CYCLES_VBIF_WR: return 26;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_2D_STALL_CYCLES_VBIF_RETURN: return 27;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_2D_RD_DATA: return 28;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_2D_WR_DATA: return 29;
  }
  // clang-format on
}

uint32_t adreno_common_perfcounter_convert_to_a5xx(
    hpc_gpu_adreno_common_perfcounter_t perfcounter) {
  // clang-format off
  switch (perfcounter) {
    case HPC_GPU_ADRENO_COMMON_CP_ALWAYS_COUNT: return 0;
    case HPC_GPU_ADRENO_COMMON_CP_BUSY_GFX_CORE_IDLE: return 1;
    case HPC_GPU_ADRENO_COMMON_CP_BUSY_CYCLES: return 2;
    case HPC_GPU_ADRENO_COMMON_CP_NUM_PREEMPTIONS: return 20;
    case HPC_GPU_ADRENO_COMMON_CP_PREEMPTION_REACTION_DELAY: return 21;
    case HPC_GPU_ADRENO_COMMON_CP_PREEMPTION_SWITCH_OUT_TIME: return 22;
    case HPC_GPU_ADRENO_COMMON_CP_PREEMPTION_SWITCH_IN_TIME: return 23;
    case HPC_GPU_ADRENO_COMMON_CP_DEAD_DRAWS_IN_BIN_RENDER: return 24;
    case HPC_GPU_ADRENO_COMMON_CP_PREDICATED_DRAWS_KILLED: return 25;
    case HPC_GPU_ADRENO_COMMON_CP_MODE_SWITCH: return 26;
    case HPC_GPU_ADRENO_COMMON_CP_ZPASS_DONE: return 27;
    case HPC_GPU_ADRENO_COMMON_CP_CONTEXT_DONE: return 28;
    case HPC_GPU_ADRENO_COMMON_CP_CACHE_FLUSH: return 29;
    case HPC_GPU_ADRENO_COMMON_CP_LONG_PREEMPTIONS: return 30;
    case HPC_GPU_ADRENO_COMMON_RBBM_ALWAYS_COUNT: return 0;
    case HPC_GPU_ADRENO_COMMON_RBBM_ALWAYS_ON: return 1;
    case HPC_GPU_ADRENO_COMMON_RBBM_TSE_BUSY: return 2;
    case HPC_GPU_ADRENO_COMMON_RBBM_RAS_BUSY: return 3;
    case HPC_GPU_ADRENO_COMMON_RBBM_PC_DCALL_BUSY: return 4;
    case HPC_GPU_ADRENO_COMMON_RBBM_PC_VSD_BUSY: return 5;
    case HPC_GPU_ADRENO_COMMON_RBBM_STATUS_MASKED: return 6;
    case HPC_GPU_ADRENO_COMMON_RBBM_COM_BUSY: return 7;
    case HPC_GPU_ADRENO_COMMON_RBBM_DCOM_BUSY: return 8;
    case HPC_GPU_ADRENO_COMMON_RBBM_VBIF_BUSY: return 9;
    case HPC_GPU_ADRENO_COMMON_RBBM_VSC_BUSY: return 10;
    case HPC_GPU_ADRENO_COMMON_RBBM_TESS_BUSY: return 11;
    case HPC_GPU_ADRENO_COMMON_RBBM_UCHE_BUSY: return 12;
    case HPC_GPU_ADRENO_COMMON_RBBM_HLSQ_BUSY: return 13;
    case HPC_GPU_ADRENO_COMMON_PC_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_PC_WORKING_CYCLES: return 1;
    case HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_VFD: return 2;
    case HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_TSE: return 3;
    case HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_VPC: return 4;
    case HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_UCHE: return 5;
    case HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_TESS: return 6;
    case HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_TSE_ONLY: return 7;
    case HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_VPC_ONLY: return 8;
    case HPC_GPU_ADRENO_COMMON_PC_PASS1_TF_STALL_CYCLES: return 9;
    case HPC_GPU_ADRENO_COMMON_PC_STARVE_CYCLES_FOR_INDEX: return 10;
    case HPC_GPU_ADRENO_COMMON_PC_STARVE_CYCLES_FOR_TESS_FACTOR: return 11;
    case HPC_GPU_ADRENO_COMMON_PC_STARVE_CYCLES_FOR_VIZ_STREAM: return 12;
    case HPC_GPU_ADRENO_COMMON_PC_STARVE_CYCLES_FOR_POSITION: return 13;
    case HPC_GPU_ADRENO_COMMON_PC_STARVE_CYCLES_DI: return 14;
    case HPC_GPU_ADRENO_COMMON_PC_VIS_STREAMS_LOADED: return 15;
    case HPC_GPU_ADRENO_COMMON_PC_INSTANCES: return 16;
    case HPC_GPU_ADRENO_COMMON_PC_VPC_PRIMITIVES: return 17;
    case HPC_GPU_ADRENO_COMMON_PC_DEAD_PRIM: return 18;
    case HPC_GPU_ADRENO_COMMON_PC_LIVE_PRIM: return 19;
    case HPC_GPU_ADRENO_COMMON_PC_VERTEX_HITS: return 20;
    case HPC_GPU_ADRENO_COMMON_PC_IA_VERTICES: return 21;
    case HPC_GPU_ADRENO_COMMON_PC_IA_PRIMITIVES: return 22;
    case HPC_GPU_ADRENO_COMMON_PC_GS_PRIMITIVES: return 23;
    case HPC_GPU_ADRENO_COMMON_PC_HS_INVOCATIONS: return 24;
    case HPC_GPU_ADRENO_COMMON_PC_DS_INVOCATIONS: return 25;
    case HPC_GPU_ADRENO_COMMON_PC_VS_INVOCATIONS: return 26;
    case HPC_GPU_ADRENO_COMMON_PC_GS_INVOCATIONS: return 27;
    case HPC_GPU_ADRENO_COMMON_PC_DS_PRIMITIVES: return 28;
    case HPC_GPU_ADRENO_COMMON_PC_VPC_POS_DATA_TRANSACTION: return 29;
    case HPC_GPU_ADRENO_COMMON_PC_3D_DRAWCALLS: return 30;
    case HPC_GPU_ADRENO_COMMON_PC_2D_DRAWCALLS: return 31;
    case HPC_GPU_ADRENO_COMMON_PC_NON_DRAWCALL_GLOBAL_EVENTS: return 32;
    case HPC_GPU_ADRENO_COMMON_PC_TESS_BUSY_CYCLES: return 33;
    case HPC_GPU_ADRENO_COMMON_PC_TESS_WORKING_CYCLES: return 34;
    case HPC_GPU_ADRENO_COMMON_PC_TESS_STALL_CYCLES_PC: return 35;
    case HPC_GPU_ADRENO_COMMON_PC_TESS_STARVE_CYCLES_PC: return 36;
    case HPC_GPU_ADRENO_COMMON_VFD_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_VFD_STALL_CYCLES_UCHE: return 1;
    case HPC_GPU_ADRENO_COMMON_VFD_STALL_CYCLES_VPC_ALLOC: return 2;
    case HPC_GPU_ADRENO_COMMON_VFD_STALL_CYCLES_SP_INFO: return 5;
    case HPC_GPU_ADRENO_COMMON_VFD_STALL_CYCLES_SP_ATTR: return 6;
    case HPC_GPU_ADRENO_COMMON_VFD_STARVE_CYCLES_UCHE: return 10;
    case HPC_GPU_ADRENO_COMMON_VFD_RBUFFER_FULL: return 11;
    case HPC_GPU_ADRENO_COMMON_VFD_ATTR_INFO_FIFO_FULL: return 12;
    case HPC_GPU_ADRENO_COMMON_VFD_DECODED_ATTRIBUTE_BYTES: return 13;
    case HPC_GPU_ADRENO_COMMON_VFD_NUM_ATTRIBUTES: return 14;
    case HPC_GPU_ADRENO_COMMON_VFD_UPPER_SHADER_FIBERS: return 16;
    case HPC_GPU_ADRENO_COMMON_VFD_LOWER_SHADER_FIBERS: return 17;
    case HPC_GPU_ADRENO_COMMON_VFD_MODE_0_FIBERS: return 18;
    case HPC_GPU_ADRENO_COMMON_VFD_MODE_1_FIBERS: return 19;
    case HPC_GPU_ADRENO_COMMON_VFD_MODE_2_FIBERS: return 20;
    case HPC_GPU_ADRENO_COMMON_VFD_MODE_3_FIBERS: return 21;
    case HPC_GPU_ADRENO_COMMON_VFD_MODE_4_FIBERS: return 22;
    case HPC_GPU_ADRENO_COMMON_VFD_TOTAL_VERTICES: return 23;
    case HPC_GPU_ADRENO_COMMON_VFD_VFDP_STALL_CYCLES_VFD: return 26;
    case HPC_GPU_ADRENO_COMMON_VFD_VFDP_STALL_CYCLES_VFD_INDEX: return 27;
    case HPC_GPU_ADRENO_COMMON_VFD_VFDP_STALL_CYCLES_VFD_PROG: return 28;
    case HPC_GPU_ADRENO_COMMON_VFD_VFDP_STARVE_CYCLES_PC: return 29;
    case HPC_GPU_ADRENO_COMMON_HLSQ_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_HLSQ_STALL_CYCLES_UCHE: return 1;
    case HPC_GPU_ADRENO_COMMON_HLSQ_STALL_CYCLES_SP_STATE: return 2;
    case HPC_GPU_ADRENO_COMMON_HLSQ_STALL_CYCLES_SP_FS_STAGE: return 3;
    case HPC_GPU_ADRENO_COMMON_HLSQ_UCHE_LATENCY_CYCLES: return 4;
    case HPC_GPU_ADRENO_COMMON_HLSQ_UCHE_LATENCY_COUNT: return 5;
    case HPC_GPU_ADRENO_COMMON_HLSQ_QUADS: return 8;
    case HPC_GPU_ADRENO_COMMON_HLSQ_CS_INVOCATIONS: return 13;
    case HPC_GPU_ADRENO_COMMON_HLSQ_COMPUTE_DRAWCALLS: return 14;
    case HPC_GPU_ADRENO_COMMON_VPC_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_VPC_WORKING_CYCLES: return 1;
    case HPC_GPU_ADRENO_COMMON_VPC_STALL_CYCLES_UCHE: return 2;
    case HPC_GPU_ADRENO_COMMON_VPC_STALL_CYCLES_VFD_WACK: return 3;
    case HPC_GPU_ADRENO_COMMON_VPC_STALL_CYCLES_HLSQ_PRIM_ALLOC: return 4;
    case HPC_GPU_ADRENO_COMMON_VPC_STALL_CYCLES_PC: return 5;
    case HPC_GPU_ADRENO_COMMON_VPC_STALL_CYCLES_SP_LM: return 6;
    case HPC_GPU_ADRENO_COMMON_VPC_STARVE_CYCLES_SP: return 8;
    case HPC_GPU_ADRENO_COMMON_VPC_STARVE_CYCLES_LRZ: return 9;
    case HPC_GPU_ADRENO_COMMON_VPC_PC_PRIMITIVES: return 10;
    case HPC_GPU_ADRENO_COMMON_VPC_SP_COMPONENTS: return 11;
    case HPC_GPU_ADRENO_COMMON_TSE_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_TSE_CLIPPING_CYCLES: return 1;
    case HPC_GPU_ADRENO_COMMON_TSE_STALL_CYCLES_RAS: return 2;
    case HPC_GPU_ADRENO_COMMON_TSE_STALL_CYCLES_LRZ_BARYPLANE: return 3;
    case HPC_GPU_ADRENO_COMMON_TSE_STALL_CYCLES_LRZ_ZPLANE: return 4;
    case HPC_GPU_ADRENO_COMMON_TSE_STARVE_CYCLES_PC: return 5;
    case HPC_GPU_ADRENO_COMMON_TSE_INPUT_PRIM: return 6;
    case HPC_GPU_ADRENO_COMMON_TSE_INPUT_NULL_PRIM: return 7;
    case HPC_GPU_ADRENO_COMMON_TSE_TRIVAL_REJ_PRIM: return 8;
    case HPC_GPU_ADRENO_COMMON_TSE_CLIPPED_PRIM: return 9;
    case HPC_GPU_ADRENO_COMMON_TSE_ZERO_AREA_PRIM: return 10;
    case HPC_GPU_ADRENO_COMMON_TSE_FACENESS_CULLED_PRIM: return 11;
    case HPC_GPU_ADRENO_COMMON_TSE_ZERO_PIXEL_PRIM: return 12;
    case HPC_GPU_ADRENO_COMMON_TSE_OUTPUT_NULL_PRIM: return 13;
    case HPC_GPU_ADRENO_COMMON_TSE_OUTPUT_VISIBLE_PRIM: return 14;
    case HPC_GPU_ADRENO_COMMON_TSE_CINVOCATION: return 15;
    case HPC_GPU_ADRENO_COMMON_TSE_CPRIMITIVES: return 16;
    case HPC_GPU_ADRENO_COMMON_TSE_2D_INPUT_PRIM: return 17;
    case HPC_GPU_ADRENO_COMMON_RAS_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_RAS_SUPERTILE_ACTIVE_CYCLES: return 1;
    case HPC_GPU_ADRENO_COMMON_RAS_STALL_CYCLES_LRZ: return 2;
    case HPC_GPU_ADRENO_COMMON_RAS_STARVE_CYCLES_TSE: return 3;
    case HPC_GPU_ADRENO_COMMON_RAS_SUPER_TILES: return 4;
    case HPC_GPU_ADRENO_COMMON_RAS_8X4_TILES: return 5;
    case HPC_GPU_ADRENO_COMMON_RAS_MASKGEN_ACTIVE: return 6;
    case HPC_GPU_ADRENO_COMMON_RAS_FULLY_COVERED_SUPER_TILES: return 7;
    case HPC_GPU_ADRENO_COMMON_RAS_FULLY_COVERED_8X4_TILES: return 8;
    case HPC_GPU_ADRENO_COMMON_RAS_PRIM_KILLED_INVISILBE: return 9;
    case HPC_GPU_ADRENO_COMMON_UCHE_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_LATENCY_CYCLES: return 2;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_LATENCY_SAMPLES: return 3;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_TP: return 4;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_VFD: return 5;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_HLSQ: return 6;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_LRZ: return 7;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_SP: return 8;
    case HPC_GPU_ADRENO_COMMON_UCHE_READ_REQUESTS_TP: return 9;
    case HPC_GPU_ADRENO_COMMON_UCHE_READ_REQUESTS_VFD: return 10;
    case HPC_GPU_ADRENO_COMMON_UCHE_READ_REQUESTS_HLSQ: return 11;
    case HPC_GPU_ADRENO_COMMON_UCHE_READ_REQUESTS_LRZ: return 12;
    case HPC_GPU_ADRENO_COMMON_UCHE_READ_REQUESTS_SP: return 13;
    case HPC_GPU_ADRENO_COMMON_UCHE_WRITE_REQUESTS_LRZ: return 14;
    case HPC_GPU_ADRENO_COMMON_UCHE_WRITE_REQUESTS_SP: return 15;
    case HPC_GPU_ADRENO_COMMON_UCHE_WRITE_REQUESTS_VPC: return 16;
    case HPC_GPU_ADRENO_COMMON_UCHE_WRITE_REQUESTS_VSC: return 17;
    case HPC_GPU_ADRENO_COMMON_UCHE_EVICTS: return 18;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ0: return 19;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ1: return 20;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ2: return 21;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ3: return 22;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ4: return 23;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ5: return 24;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ6: return 25;
    case HPC_GPU_ADRENO_COMMON_UCHE_BANK_REQ7: return 26;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_CH0: return 27;
    case HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_CH1: return 28;
    case HPC_GPU_ADRENO_COMMON_UCHE_GMEM_READ_BEATS: return 29;
    case HPC_GPU_ADRENO_COMMON_TP_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_TP_STALL_CYCLES_UCHE: return 1;
    case HPC_GPU_ADRENO_COMMON_TP_LATENCY_CYCLES: return 2;
    case HPC_GPU_ADRENO_COMMON_TP_LATENCY_TRANS: return 3;
    case HPC_GPU_ADRENO_COMMON_TP_FLAG_CACHE_REQUEST_SAMPLES: return 4;
    case HPC_GPU_ADRENO_COMMON_TP_FLAG_CACHE_REQUEST_LATENCY: return 5;
    case HPC_GPU_ADRENO_COMMON_TP_L1_CACHELINE_REQUESTS: return 6;
    case HPC_GPU_ADRENO_COMMON_TP_L1_CACHELINE_MISSES: return 7;
    case HPC_GPU_ADRENO_COMMON_TP_SP_TP_TRANS: return 8;
    case HPC_GPU_ADRENO_COMMON_TP_TP_SP_TRANS: return 9;
    case HPC_GPU_ADRENO_COMMON_TP_OUTPUT_PIXELS: return 10;
    case HPC_GPU_ADRENO_COMMON_TP_FILTER_WORKLOAD_16BIT: return 11;
    case HPC_GPU_ADRENO_COMMON_TP_FILTER_WORKLOAD_32BIT: return 12;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_RECEIVED: return 13;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_OFFSET: return 14;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_SHADOW: return 15;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_ARRAY: return 16;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_GRADIENT: return 17;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_1D: return 18;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_2D: return 19;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_BUFFER: return 20;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_3D: return 21;
    case HPC_GPU_ADRENO_COMMON_TP_QUADS_CUBE: return 22;
    case HPC_GPU_ADRENO_COMMON_TP_DIVERGENT_QUADS_RECEIVED: return 25;
    case HPC_GPU_ADRENO_COMMON_TP_PRT_NON_RESIDENT_EVENTS: return 28;
    case HPC_GPU_ADRENO_COMMON_TP_OUTPUT_PIXELS_POINT: return 29;
    case HPC_GPU_ADRENO_COMMON_TP_OUTPUT_PIXELS_BILINEAR: return 30;
    case HPC_GPU_ADRENO_COMMON_TP_OUTPUT_PIXELS_MIP: return 31;
    case HPC_GPU_ADRENO_COMMON_TP_OUTPUT_PIXELS_ANISO: return 32;
    case HPC_GPU_ADRENO_COMMON_TP_OUTPUT_PIXELS_ZERO_LOD: return 33;
    case HPC_GPU_ADRENO_COMMON_TP_FLAG_CACHE_REQUESTS: return 34;
    case HPC_GPU_ADRENO_COMMON_TP_FLAG_CACHE_MISSES: return 35;
    case HPC_GPU_ADRENO_COMMON_TP_L1_5_L2_REQUESTS: return 36;
    case HPC_GPU_ADRENO_COMMON_TP_2D_OUTPUT_PIXELS: return 37;
    case HPC_GPU_ADRENO_COMMON_TP_2D_OUTPUT_PIXELS_POINT: return 38;
    case HPC_GPU_ADRENO_COMMON_TP_2D_OUTPUT_PIXELS_BILINEAR: return 39;
    case HPC_GPU_ADRENO_COMMON_TP_2D_FILTER_WORKLOAD_16BIT: return 40;
    case HPC_GPU_ADRENO_COMMON_TP_2D_FILTER_WORKLOAD_32BIT: return 41;
    case HPC_GPU_ADRENO_COMMON_SP_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_SP_ALU_WORKING_CYCLES: return 1;
    case HPC_GPU_ADRENO_COMMON_SP_EFU_WORKING_CYCLES: return 2;
    case HPC_GPU_ADRENO_COMMON_SP_STALL_CYCLES_VPC: return 3;
    case HPC_GPU_ADRENO_COMMON_SP_STALL_CYCLES_TP: return 4;
    case HPC_GPU_ADRENO_COMMON_SP_STALL_CYCLES_UCHE: return 5;
    case HPC_GPU_ADRENO_COMMON_SP_STALL_CYCLES_RB: return 6;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_CONTEXTS: return 8;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_CONTEXT_CYCLES: return 9;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_WAVE_CYCLES: return 10;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_WAVE_SAMPLES: return 11;
    case HPC_GPU_ADRENO_COMMON_SP_VS_STAGE_WAVE_CYCLES: return 12;
    case HPC_GPU_ADRENO_COMMON_SP_VS_STAGE_WAVE_SAMPLES: return 13;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_DURATION_CYCLES: return 14;
    case HPC_GPU_ADRENO_COMMON_SP_VS_STAGE_DURATION_CYCLES: return 15;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_CTRL_CYCLES: return 16;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_LOAD_CYCLES: return 17;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_EMIT_CYCLES: return 18;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_NOP_CYCLES: return 19;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_WAIT_CYCLES: return 20;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_FETCH_CYCLES: return 21;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_IDLE_CYCLES: return 22;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_END_CYCLES: return 23;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_LONG_SYNC_CYCLES: return 24;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_SHORT_SYNC_CYCLES: return 25;
    case HPC_GPU_ADRENO_COMMON_SP_WAVE_JOIN_CYCLES: return 26;
    case HPC_GPU_ADRENO_COMMON_SP_LM_LOAD_INSTRUCTIONS: return 27;
    case HPC_GPU_ADRENO_COMMON_SP_LM_STORE_INSTRUCTIONS: return 28;
    case HPC_GPU_ADRENO_COMMON_SP_LM_ATOMICS: return 29;
    case HPC_GPU_ADRENO_COMMON_SP_GM_LOAD_INSTRUCTIONS: return 30;
    case HPC_GPU_ADRENO_COMMON_SP_GM_STORE_INSTRUCTIONS: return 31;
    case HPC_GPU_ADRENO_COMMON_SP_GM_ATOMICS: return 32;
    case HPC_GPU_ADRENO_COMMON_SP_VS_STAGE_TEX_INSTRUCTIONS: return 33;
    case HPC_GPU_ADRENO_COMMON_SP_VS_STAGE_EFU_INSTRUCTIONS: return 35;
    case HPC_GPU_ADRENO_COMMON_SP_VS_STAGE_FULL_ALU_INSTRUCTIONS: return 36;
    case HPC_GPU_ADRENO_COMMON_SP_VS_STAGE_HALF_ALU_INSTRUCTIONS: return 37;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_TEX_INSTRUCTIONS: return 38;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_CFLOW_INSTRUCTIONS: return 39;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_EFU_INSTRUCTIONS: return 40;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_FULL_ALU_INSTRUCTIONS: return 41;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_HALF_ALU_INSTRUCTIONS: return 42;
    case HPC_GPU_ADRENO_COMMON_SP_FS_STAGE_BARY_INSTRUCTIONS: return 43;
    case HPC_GPU_ADRENO_COMMON_SP_VS_INSTRUCTIONS: return 44;
    case HPC_GPU_ADRENO_COMMON_SP_FS_INSTRUCTIONS: return 45;
    case HPC_GPU_ADRENO_COMMON_SP_ADDR_LOCK_COUNT: return 46;
    case HPC_GPU_ADRENO_COMMON_SP_UCHE_READ_TRANS: return 47;
    case HPC_GPU_ADRENO_COMMON_SP_UCHE_WRITE_TRANS: return 48;
    case HPC_GPU_ADRENO_COMMON_SP_EXPORT_VPC_TRANS: return 49;
    case HPC_GPU_ADRENO_COMMON_SP_EXPORT_RB_TRANS: return 50;
    case HPC_GPU_ADRENO_COMMON_SP_PIXELS_KILLED: return 51;
    case HPC_GPU_ADRENO_COMMON_SP_ICL1_REQUESTS: return 52;
    case HPC_GPU_ADRENO_COMMON_SP_ICL1_MISSES: return 53;
    case HPC_GPU_ADRENO_COMMON_SP_HS_INSTRUCTIONS: return 56;
    case HPC_GPU_ADRENO_COMMON_SP_DS_INSTRUCTIONS: return 57;
    case HPC_GPU_ADRENO_COMMON_SP_GS_INSTRUCTIONS: return 58;
    case HPC_GPU_ADRENO_COMMON_SP_CS_INSTRUCTIONS: return 59;
    case HPC_GPU_ADRENO_COMMON_SP_GPR_READ: return 60;
    case HPC_GPU_ADRENO_COMMON_SP_GPR_WRITE: return 61;
    case HPC_GPU_ADRENO_COMMON_SP_LM_BANK_CONFLICTS: return 64;
    case HPC_GPU_ADRENO_COMMON_RB_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_RB_STALL_CYCLES_HLSQ: return 2;
    case HPC_GPU_ADRENO_COMMON_RB_STALL_CYCLES_FIFO0_FULL: return 3;
    case HPC_GPU_ADRENO_COMMON_RB_STALL_CYCLES_FIFO1_FULL: return 4;
    case HPC_GPU_ADRENO_COMMON_RB_STALL_CYCLES_FIFO2_FULL: return 5;
    case HPC_GPU_ADRENO_COMMON_RB_STARVE_CYCLES_SP: return 6;
    case HPC_GPU_ADRENO_COMMON_RB_STARVE_CYCLES_LRZ_TILE: return 7;
    case HPC_GPU_ADRENO_COMMON_RB_STARVE_CYCLES_CCU: return 8;
    case HPC_GPU_ADRENO_COMMON_RB_STARVE_CYCLES_Z_PLANE: return 9;
    case HPC_GPU_ADRENO_COMMON_RB_STARVE_CYCLES_BARY_PLANE: return 10;
    case HPC_GPU_ADRENO_COMMON_RB_Z_WORKLOAD: return 11;
    case HPC_GPU_ADRENO_COMMON_RB_HLSQ_ACTIVE: return 12;
    case HPC_GPU_ADRENO_COMMON_RB_Z_READ: return 13;
    case HPC_GPU_ADRENO_COMMON_RB_Z_WRITE: return 14;
    case HPC_GPU_ADRENO_COMMON_RB_C_READ: return 15;
    case HPC_GPU_ADRENO_COMMON_RB_C_WRITE: return 16;
    case HPC_GPU_ADRENO_COMMON_RB_TOTAL_PASS: return 17;
    case HPC_GPU_ADRENO_COMMON_RB_Z_PASS: return 18;
    case HPC_GPU_ADRENO_COMMON_RB_Z_FAIL: return 19;
    case HPC_GPU_ADRENO_COMMON_RB_S_FAIL: return 20;
    case HPC_GPU_ADRENO_COMMON_RB_BLENDED_FXP_COMPONENTS: return 21;
    case HPC_GPU_ADRENO_COMMON_RB_BLENDED_FP16_COMPONENTS: return 22;
    case HPC_GPU_ADRENO_COMMON_RB_2D_ALIVE_CYCLES: return 24;
    case HPC_GPU_ADRENO_COMMON_RB_2D_STALL_CYCLES_A2D: return 25;
    case HPC_GPU_ADRENO_COMMON_RB_2D_STARVE_CYCLES_SRC: return 26;
    case HPC_GPU_ADRENO_COMMON_RB_2D_STARVE_CYCLES_SP: return 27;
    case HPC_GPU_ADRENO_COMMON_RB_2D_STARVE_CYCLES_DST: return 28;
    case HPC_GPU_ADRENO_COMMON_RB_2D_VALID_PIXELS: return 29;
    case HPC_GPU_ADRENO_COMMON_RB_STALL_CYCLES_CCU: return 1;
    case HPC_GPU_ADRENO_COMMON_VSC_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_VSC_WORKING_CYCLES: return 1;
    case HPC_GPU_ADRENO_COMMON_VSC_STALL_CYCLES_UCHE: return 2;
    case HPC_GPU_ADRENO_COMMON_VSC_EOT_NUM: return 3;
    case HPC_GPU_ADRENO_COMMON_CCU_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_CCU_STALL_CYCLES_RB_DEPTH_RETURN: return 1;
    case HPC_GPU_ADRENO_COMMON_CCU_STALL_CYCLES_RB_COLOR_RETURN: return 2;
    case HPC_GPU_ADRENO_COMMON_CCU_STARVE_CYCLES_FLAG_RETURN: return 3;
    case HPC_GPU_ADRENO_COMMON_CCU_DEPTH_BLOCKS: return 4;
    case HPC_GPU_ADRENO_COMMON_CCU_COLOR_BLOCKS: return 5;
    case HPC_GPU_ADRENO_COMMON_CCU_DEPTH_BLOCK_HIT: return 6;
    case HPC_GPU_ADRENO_COMMON_CCU_COLOR_BLOCK_HIT: return 7;
    case HPC_GPU_ADRENO_COMMON_CCU_PARTIAL_BLOCK_READ: return 8;
    case HPC_GPU_ADRENO_COMMON_CCU_GMEM_READ: return 9;
    case HPC_GPU_ADRENO_COMMON_CCU_GMEM_WRITE: return 10;
    case HPC_GPU_ADRENO_COMMON_CCU_DEPTH_READ_FLAG0_COUNT: return 11;
    case HPC_GPU_ADRENO_COMMON_CCU_DEPTH_READ_FLAG1_COUNT: return 12;
    case HPC_GPU_ADRENO_COMMON_CCU_DEPTH_READ_FLAG2_COUNT: return 13;
    case HPC_GPU_ADRENO_COMMON_CCU_DEPTH_READ_FLAG3_COUNT: return 14;
    case HPC_GPU_ADRENO_COMMON_CCU_DEPTH_READ_FLAG4_COUNT: return 15;
    case HPC_GPU_ADRENO_COMMON_CCU_COLOR_READ_FLAG0_COUNT: return 16;
    case HPC_GPU_ADRENO_COMMON_CCU_COLOR_READ_FLAG1_COUNT: return 17;
    case HPC_GPU_ADRENO_COMMON_CCU_COLOR_READ_FLAG2_COUNT: return 18;
    case HPC_GPU_ADRENO_COMMON_CCU_COLOR_READ_FLAG3_COUNT: return 19;
    case HPC_GPU_ADRENO_COMMON_CCU_COLOR_READ_FLAG4_COUNT: return 20;
    case HPC_GPU_ADRENO_COMMON_CCU_2D_RD_REQ: return 22;
    case HPC_GPU_ADRENO_COMMON_CCU_2D_WR_REQ: return 23;
    case HPC_GPU_ADRENO_COMMON_LRZ_BUSY_CYCLES: return 0;
    case HPC_GPU_ADRENO_COMMON_LRZ_STARVE_CYCLES_RAS: return 1;
    case HPC_GPU_ADRENO_COMMON_LRZ_STALL_CYCLES_RB: return 2;
    case HPC_GPU_ADRENO_COMMON_LRZ_STALL_CYCLES_VSC: return 3;
    case HPC_GPU_ADRENO_COMMON_LRZ_STALL_CYCLES_VPC: return 4;
    case HPC_GPU_ADRENO_COMMON_LRZ_STALL_CYCLES_FLAG_PREFETCH: return 5;
    case HPC_GPU_ADRENO_COMMON_LRZ_STALL_CYCLES_UCHE: return 6;
    case HPC_GPU_ADRENO_COMMON_LRZ_LRZ_READ: return 7;
    case HPC_GPU_ADRENO_COMMON_LRZ_LRZ_WRITE: return 8;
    case HPC_GPU_ADRENO_COMMON_LRZ_READ_LATENCY: return 9;
    case HPC_GPU_ADRENO_COMMON_LRZ_MERGE_CACHE_UPDATING: return 10;
    case HPC_GPU_ADRENO_COMMON_LRZ_PRIM_KILLED_BY_MASKGEN: return 11;
    case HPC_GPU_ADRENO_COMMON_LRZ_PRIM_KILLED_BY_LRZ: return 12;
    case HPC_GPU_ADRENO_COMMON_LRZ_VISIBLE_PRIM_AFTER_LRZ: return 13;
    case HPC_GPU_ADRENO_COMMON_LRZ_FULL_8X8_TILES: return 14;
    case HPC_GPU_ADRENO_COMMON_LRZ_PARTIAL_8X8_TILES: return 15;
    case HPC_GPU_ADRENO_COMMON_LRZ_TILE_KILLED: return 16;
    case HPC_GPU_ADRENO_COMMON_LRZ_TOTAL_PIXEL: return 17;
    case HPC_GPU_ADRENO_COMMON_LRZ_VISIBLE_PIXEL_AFTER_LRZ: return 18;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_LATENCY_CYCLES: return 1;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_LATENCY_SAMPLES: return 2;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_READ_DATA_CCU: return 3;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_WRITE_DATA_CCU: return 4;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_READ_REQUEST: return 5;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_WRITE_REQUEST: return 6;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_READ_DATA: return 7;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_WRITE_DATA: return 8;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_FLAG_FETCH_CYCLES: return 9;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_FLAG_FETCH_SAMPLES: return 10;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_DEPTH_WRITE_FLAG1_COUNT: return 11;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_DEPTH_WRITE_FLAG2_COUNT: return 12;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_DEPTH_WRITE_FLAG3_COUNT: return 13;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_DEPTH_WRITE_FLAG4_COUNT: return 14;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_COLOR_WRITE_FLAG1_COUNT: return 15;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_COLOR_WRITE_FLAG2_COUNT: return 16;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_COLOR_WRITE_FLAG3_COUNT: return 17;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_COLOR_WRITE_FLAG4_COUNT: return 18;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_2D_STALL_CYCLES_VBIF_REQ: return 19;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_2D_STALL_CYCLES_VBIF_WR: return 20;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_2D_STALL_CYCLES_VBIF_RETURN: return 21;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_2D_RD_DATA: return 22;
    case HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_2D_WR_DATA: return 23;
  }
  // clang-format on
}

//===--------------- END AUTOGENERATED REGION; DO NOT EDIT! ---------------===//
