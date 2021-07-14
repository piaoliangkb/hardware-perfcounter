#ifndef HPC_GPU_MALI_COMMON_H_
#define HPC_GPU_MALI_COMMON_H_

#include <stdint.h>

#include "hpc/gpu/base_utilities.h"

// Forward declaration of counter enums.
typedef enum hpc_gpu_mali_common_counter_e hpc_gpu_mali_common_counter_t;
typedef enum hpc_gpu_mali_valhall_counter_e hpc_gpu_mali_valhall_counter_e;
typedef enum hpc_gpu_mali_bifrost_counter_t_counter_e
    hpc_gpu_mali_bifrost_counter_t_counter_e;

/// The context for sampling Mali GPU counters.
typedef struct hpc_gpu_mali_context_t hpc_gpu_mali_context_t;

/// Creates a context for sampling counters available to all known Mali GPUs.
///
/// Creating the context means talking with the Mali GPU kernel driver and
/// allocating resources for sampling the given counters.
///
/// @param[in]  num_counters The number of counters to sample later.
/// @param[in]  counters     The pointer to the list of counters to sample
///                          later.
/// @param[in]  allocator    The allocator used to allocate host memory for
///                          sampling counters later.
/// @param[out] out_context  The pointer to the object receiving the resultant
///                          context.
int hpc_gpu_mali_common_create_context(
    uint32_t num_counters, hpc_gpu_mali_common_counter_t *counters,
    const hpc_gpu_host_allocation_callbacks_t *allocator,
    hpc_gpu_mali_context_t **out_context);

/// Destroys the context for common Mali GPU counters.
///
/// @param[in] context   The counter sampling context.
/// @param[in] allocator The allocator used to free allocated host memory.
int hpc_gpu_mali_common_destroy_context(
    hpc_gpu_mali_context_t *context,
    const hpc_gpu_host_allocation_callbacks_t *allocator);

/// Starts sampling the common Mali GPU counters specified when creating
/// the context.
///
/// This zeros the registered counters in preparation for continously sampling.
///
/// @param[in] context The counter sampling context.
int hpc_gpu_mali_common_start_counters(hpc_gpu_mali_context_t *context);

/// Stops sampling the common Mali GPU counters specified when creating
/// the context.
///
/// @param[in] context The counter sampling context.
int hpc_gpu_mali_common_stop_counters(hpc_gpu_mali_context_t *context);

/// Samples the common Mali GPU counters specified when creating
/// the context.
///
/// @param[in]  context The counter sampling context.
/// @param[out] values  The pointer to the memory for receiving newly sampled
///                     values. Its element count should be greater than or
///                     equal to the number of counters specified when
///                     creating the `context`.
int hpc_gpu_mali_common_query_counters(hpc_gpu_mali_context_t *context,
                                       uint64_t *values);

//===-------------- BEGIN AUTOGENERATED REGION; DO NOT EDIT! --------------===//

/// Common Mali GPU counters.
typedef enum hpc_gpu_mali_common_counter_e {
  HPC_GPU_MALI_COMMON_JOB_MANAGER_MESSAGES_SENT = 4u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_MESSAGES_RECEIVED = 5u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_GPU_ACTIVE = 6u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_IRQ_ACTIVE = 7u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS0_JOBS = 8u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS0_TASKS = 9u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS0_ACTIVE = 10u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS0_WAIT_READ = 12u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS0_WAIT_ISSUE = 13u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS0_WAIT_DEPEND = 14u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS0_WAIT_FINISH = 15u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS1_JOBS = 16u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS1_TASKS = 17u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS1_ACTIVE = 18u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS1_WAIT_READ = 20u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS1_WAIT_ISSUE = 21u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS1_WAIT_DEPEND = 22u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS1_WAIT_FINISH = 23u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS2_JOBS = 24u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS2_TASKS = 25u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS2_ACTIVE = 26u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS2_WAIT_READ = 28u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS2_WAIT_ISSUE = 29u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS2_WAIT_DEPEND = 30u,
  HPC_GPU_MALI_COMMON_JOB_MANAGER_JS2_WAIT_FINISH = 31u,
  HPC_GPU_MALI_COMMON_TILER_JOBS_PROCESSED = 259u,
  HPC_GPU_MALI_COMMON_TILER_TRIANGLES = 260u,
  HPC_GPU_MALI_COMMON_TILER_POINTS = 263u,
  HPC_GPU_MALI_COMMON_TILER_LINES = 264u,
  HPC_GPU_MALI_COMMON_TILER_FRONT_FACING = 265u,
  HPC_GPU_MALI_COMMON_TILER_BACK_FACING = 266u,
  HPC_GPU_MALI_COMMON_TILER_PRIM_VISIBLE = 267u,
  HPC_GPU_MALI_COMMON_TILER_PRIM_CULLED = 268u,
  HPC_GPU_MALI_COMMON_TILER_PRIM_CLIPPED = 269u,
  HPC_GPU_MALI_COMMON_SHADER_CORE_FRAG_ACTIVE = 516u,
  HPC_GPU_MALI_COMMON_SHADER_CORE_FRAG_QUADS_RAST = 526u,
  HPC_GPU_MALI_COMMON_SHADER_CORE_FRAG_QUADS_EZS_TEST = 527u,
  HPC_GPU_MALI_COMMON_SHADER_CORE_FRAG_TRANS_ELIM = 533u,
  HPC_GPU_MALI_COMMON_SHADER_CORE_COMPUTE_ACTIVE = 534u,
  HPC_GPU_MALI_COMMON_SHADER_CORE_COMPUTE_TASKS = 535u,
  HPC_GPU_MALI_COMMON_MEMORY_MMU_REQUESTS = 777u,
  HPC_GPU_MALI_COMMON_MEMORY_L2_EXT_WRITE_BEATS = 798u,
  HPC_GPU_MALI_COMMON_MEMORY_L2_EXT_READ_BEATS = 799u,
  HPC_GPU_MALI_COMMON_MEMORY_L2_ANY_LOOKUP = 800u,
  HPC_GPU_MALI_COMMON_MEMORY_L2_READ_LOOKUP = 801u,
  HPC_GPU_MALI_COMMON_MEMORY_L2_WRITE_LOOKUP = 807u,
  HPC_GPU_MALI_COMMON_MEMORY_L2_EXT_READ = 816u,
  HPC_GPU_MALI_COMMON_MEMORY_L2_EXT_WRITE = 818u,
  HPC_GPU_MALI_COMMON_MEMORY_L2_EXT_AR_STALL = 822u,
  HPC_GPU_MALI_COMMON_MEMORY_L2_EXT_W_STALL = 826u
} hpc_gpu_mali_common_counter_t;

/// Common Mali Valhall GPU counters.
typedef enum hpc_gpu_mali_valhall_counter_e {
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_MESSAGES_SENT = 4u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_MESSAGES_RECEIVED = 5u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_GPU_ACTIVE = 6u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_IRQ_ACTIVE = 7u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS0_JOBS = 8u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS0_TASKS = 9u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS0_ACTIVE = 10u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS0_WAIT_FLUSH = 11u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS0_WAIT_READ = 12u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS0_WAIT_ISSUE = 13u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS0_WAIT_DEPEND = 14u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS0_WAIT_FINISH = 15u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS1_JOBS = 16u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS1_TASKS = 17u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS1_ACTIVE = 18u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS1_WAIT_FLUSH = 19u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS1_WAIT_READ = 20u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS1_WAIT_ISSUE = 21u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS1_WAIT_DEPEND = 22u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS1_WAIT_FINISH = 23u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS2_JOBS = 24u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS2_TASKS = 25u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS2_ACTIVE = 26u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS2_WAIT_FLUSH = 27u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS2_WAIT_READ = 28u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS2_WAIT_ISSUE = 29u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS2_WAIT_DEPEND = 30u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_JS2_WAIT_FINISH = 31u,
  HPC_GPU_MALI_VALHALL_JOB_MANAGER_CACHE_FLUSH = 63u,
  HPC_GPU_MALI_VALHALL_TILER_TILER_ACTIVE = 260u,
  HPC_GPU_MALI_VALHALL_TILER_JOBS_PROCESSED = 261u,
  HPC_GPU_MALI_VALHALL_TILER_TRIANGLES = 262u,
  HPC_GPU_MALI_VALHALL_TILER_LINES = 263u,
  HPC_GPU_MALI_VALHALL_TILER_POINTS = 264u,
  HPC_GPU_MALI_VALHALL_TILER_FRONT_FACING = 265u,
  HPC_GPU_MALI_VALHALL_TILER_BACK_FACING = 266u,
  HPC_GPU_MALI_VALHALL_TILER_PRIM_VISIBLE = 267u,
  HPC_GPU_MALI_VALHALL_TILER_PRIM_CULLED = 268u,
  HPC_GPU_MALI_VALHALL_TILER_PRIM_CLIPPED = 269u,
  HPC_GPU_MALI_VALHALL_TILER_PRIM_SAT_CULLED = 270u,
  HPC_GPU_MALI_VALHALL_TILER_BIN_ALLOC_INIT = 271u,
  HPC_GPU_MALI_VALHALL_TILER_BIN_ALLOC_OVERFLOW = 272u,
  HPC_GPU_MALI_VALHALL_TILER_BUS_READ = 273u,
  HPC_GPU_MALI_VALHALL_TILER_BUS_WRITE_UTLB0 = 274u,
  HPC_GPU_MALI_VALHALL_TILER_BUS_WRITE_UTLB1 = 275u,
  HPC_GPU_MALI_VALHALL_TILER_LOADING_DESC = 276u,
  HPC_GPU_MALI_VALHALL_TILER_IDVS_POS_SHAD_REQ = 277u,
  HPC_GPU_MALI_VALHALL_TILER_IDVS_POS_SHAD_WAIT = 278u,
  HPC_GPU_MALI_VALHALL_TILER_IDVS_POS_SHAD_STALL = 279u,
  HPC_GPU_MALI_VALHALL_TILER_IDVS_POS_FIFO_FULL = 280u,
  HPC_GPU_MALI_VALHALL_TILER_PREFETCH_STALL = 281u,
  HPC_GPU_MALI_VALHALL_TILER_VCACHE_HIT = 282u,
  HPC_GPU_MALI_VALHALL_TILER_VCACHE_MISS = 283u,
  HPC_GPU_MALI_VALHALL_TILER_VCACHE_LINE_WAIT = 284u,
  HPC_GPU_MALI_VALHALL_TILER_VFETCH_POS_READ_WAIT = 285u,
  HPC_GPU_MALI_VALHALL_TILER_VFETCH_VERTEX_WAIT = 286u,
  HPC_GPU_MALI_VALHALL_TILER_VFETCH_STALL = 287u,
  HPC_GPU_MALI_VALHALL_TILER_PRIMASSY_STALL = 288u,
  HPC_GPU_MALI_VALHALL_TILER_BBOX_GEN_STALL = 289u,
  HPC_GPU_MALI_VALHALL_TILER_IDVS_VBU_HIT = 290u,
  HPC_GPU_MALI_VALHALL_TILER_IDVS_VBU_MISS = 291u,
  HPC_GPU_MALI_VALHALL_TILER_IDVS_VBU_LINE_DEALLOCATE = 292u,
  HPC_GPU_MALI_VALHALL_TILER_IDVS_VAR_SHAD_REQ = 293u,
  HPC_GPU_MALI_VALHALL_TILER_IDVS_VAR_SHAD_STALL = 294u,
  HPC_GPU_MALI_VALHALL_TILER_BINNER_STALL = 295u,
  HPC_GPU_MALI_VALHALL_TILER_ITER_STALL = 296u,
  HPC_GPU_MALI_VALHALL_TILER_COMPRESS_MISS = 297u,
  HPC_GPU_MALI_VALHALL_TILER_COMPRESS_STALL = 298u,
  HPC_GPU_MALI_VALHALL_TILER_PCACHE_HIT = 299u,
  HPC_GPU_MALI_VALHALL_TILER_PCACHE_MISS = 300u,
  HPC_GPU_MALI_VALHALL_TILER_PCACHE_MISS_STALL = 301u,
  HPC_GPU_MALI_VALHALL_TILER_PCACHE_EVICT_STALL = 302u,
  HPC_GPU_MALI_VALHALL_TILER_PMGR_PTR_WR_STALL = 303u,
  HPC_GPU_MALI_VALHALL_TILER_PMGR_PTR_RD_STALL = 304u,
  HPC_GPU_MALI_VALHALL_TILER_PMGR_CMD_WR_STALL = 305u,
  HPC_GPU_MALI_VALHALL_TILER_WRBUF_ACTIVE = 306u,
  HPC_GPU_MALI_VALHALL_TILER_WRBUF_HIT = 307u,
  HPC_GPU_MALI_VALHALL_TILER_WRBUF_MISS = 308u,
  HPC_GPU_MALI_VALHALL_TILER_WRBUF_NO_FREE_LINE_STALL = 309u,
  HPC_GPU_MALI_VALHALL_TILER_WRBUF_NO_AXI_ID_STALL = 310u,
  HPC_GPU_MALI_VALHALL_TILER_WRBUF_AXI_STALL = 311u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_ACTIVE = 516u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_PRIMITIVES_OUT = 517u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_PRIM_RAST = 518u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_FPK_ACTIVE = 519u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_STARVING = 520u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_WARPS = 521u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_PARTIAL_QUADS_RAST = 522u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_QUADS_RAST = 523u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_QUADS_EZS_TEST = 524u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_QUADS_EZS_UPDATE = 525u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_QUADS_EZS_KILL = 526u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_LZS_TEST = 527u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_LZS_KILL = 528u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_WARP_REG_SIZE_64 = 529u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_PTILES = 530u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FRAG_TRANS_ELIM = 531u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_QUAD_FPK_KILLER = 532u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_FULL_QUAD_WARPS = 533u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_COMPUTE_ACTIVE = 534u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_COMPUTE_TASKS = 535u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_COMPUTE_WARPS = 536u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_COMPUTE_STARVING = 537u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_EXEC_CORE_ACTIVE = 538u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_EXEC_INSTR_FMA = 539u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_EXEC_INSTR_CVT = 540u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_EXEC_INSTR_SFU = 541u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_EXEC_INSTR_MSG = 542u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_EXEC_INSTR_DIVERGED = 543u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_EXEC_ICACHE_MISS = 544u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_EXEC_STARVE_ARITH = 545u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_CALL_BLEND_SHADER = 546u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_TEX_MSGI_NUM_FLITS = 547u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_TEX_DFCH_CLK_STALLED = 548u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_TEX_TFCH_CLK_STALLED = 549u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_TEX_TFCH_STARVED_PENDING_DATA_FETCH = 550u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_TEX_FILT_NUM_OPERATIONS = 551u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_TEX_FILT_NUM_FXR_OPERATIONS = 552u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_TEX_FILT_NUM_FST_OPERATIONS = 553u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_TEX_MSGO_NUM_MSG = 554u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_TEX_MSGO_NUM_FLITS = 555u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_LS_MEM_READ_FULL = 556u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_LS_MEM_READ_SHORT = 557u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_LS_MEM_WRITE_FULL = 558u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_LS_MEM_WRITE_SHORT = 559u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_LS_MEM_ATOMIC = 560u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_VARY_INSTR = 561u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_VARY_SLOT_32 = 562u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_VARY_SLOT_16 = 563u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_ATTR_INSTR = 564u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_BEATS_RD_FTC = 566u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_BEATS_RD_FTC_EXT = 567u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_BEATS_RD_LSC = 568u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_BEATS_RD_LSC_EXT = 569u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_BEATS_RD_TEX = 570u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_BEATS_RD_TEX_EXT = 571u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_BEATS_RD_OTHER = 572u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_BEATS_WR_LSC_OTHER = 573u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_BEATS_WR_TIB = 574u,
  HPC_GPU_MALI_VALHALL_SHADER_CORE_BEATS_WR_LSC_WB = 575u,
  HPC_GPU_MALI_VALHALL_MEMORY_MMU_REQUESTS = 772u,
  HPC_GPU_MALI_VALHALL_MEMORY_MMU_TABLE_READS_L3 = 773u,
  HPC_GPU_MALI_VALHALL_MEMORY_MMU_TABLE_READS_L2 = 774u,
  HPC_GPU_MALI_VALHALL_MEMORY_MMU_HIT_L3 = 775u,
  HPC_GPU_MALI_VALHALL_MEMORY_MMU_HIT_L2 = 776u,
  HPC_GPU_MALI_VALHALL_MEMORY_MMU_S2_REQUESTS = 777u,
  HPC_GPU_MALI_VALHALL_MEMORY_MMU_S2_TABLE_READS_L3 = 778u,
  HPC_GPU_MALI_VALHALL_MEMORY_MMU_S2_TABLE_READS_L2 = 779u,
  HPC_GPU_MALI_VALHALL_MEMORY_MMU_S2_HIT_L3 = 780u,
  HPC_GPU_MALI_VALHALL_MEMORY_MMU_S2_HIT_L2 = 781u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_RD_MSG_IN = 784u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_RD_MSG_IN_STALL = 785u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_WR_MSG_IN = 786u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_WR_MSG_IN_STALL = 787u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_SNP_MSG_IN = 788u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_SNP_MSG_IN_STALL = 789u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_RD_MSG_OUT = 790u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_RD_MSG_OUT_STALL = 791u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_WR_MSG_OUT = 792u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_ANY_LOOKUP = 793u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_READ_LOOKUP = 794u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_WRITE_LOOKUP = 795u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_SNOOP_LOOKUP = 796u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_READ = 797u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_READ_NOSNP = 798u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_READ_UNIQUE = 799u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_READ_BEATS = 800u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_AR_STALL = 801u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_AR_CNT_Q1 = 802u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_AR_CNT_Q2 = 803u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_AR_CNT_Q3 = 804u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_RRESP_0_127 = 805u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_RRESP_128_191 = 806u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_RRESP_192_255 = 807u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_RRESP_256_319 = 808u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_RRESP_320_383 = 809u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_WRITE = 810u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_WRITE_NOSNP_FULL = 811u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_WRITE_NOSNP_PTL = 812u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_WRITE_SNP_FULL = 813u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_WRITE_SNP_PTL = 814u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_WRITE_BEATS = 815u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_W_STALL = 816u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_AW_CNT_Q1 = 817u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_AW_CNT_Q2 = 818u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_AW_CNT_Q3 = 819u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_SNOOP = 820u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_SNOOP_STALL = 821u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_SNOOP_RESP_CLEAN = 822u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_SNOOP_RESP_DATA = 823u,
  HPC_GPU_MALI_VALHALL_MEMORY_L2_EXT_SNOOP_INTERNAL = 824u
} hpc_gpu_mali_valhall_counter_t;

/// Common Mali Bifrost GPU counters.
typedef enum hpc_gpu_mali_bifrost_counter_e {
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_MESSAGES_SENT = 4u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_MESSAGES_RECEIVED = 5u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_GPU_ACTIVE = 6u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_IRQ_ACTIVE = 7u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS0_JOBS = 8u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS0_TASKS = 9u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS0_ACTIVE = 10u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS0_WAIT_READ = 12u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS0_WAIT_ISSUE = 13u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS0_WAIT_DEPEND = 14u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS0_WAIT_FINISH = 15u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS1_JOBS = 16u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS1_TASKS = 17u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS1_ACTIVE = 18u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS1_WAIT_READ = 20u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS1_WAIT_ISSUE = 21u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS1_WAIT_DEPEND = 22u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS1_WAIT_FINISH = 23u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS2_JOBS = 24u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS2_TASKS = 25u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS2_ACTIVE = 26u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS2_WAIT_READ = 28u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS2_WAIT_ISSUE = 29u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS2_WAIT_DEPEND = 30u,
  HPC_GPU_MALI_BIFROST_JOB_MANAGER_JS2_WAIT_FINISH = 31u,
  HPC_GPU_MALI_BIFROST_TILER_TILER_ACTIVE = 260u,
  HPC_GPU_MALI_BIFROST_TILER_JOBS_PROCESSED = 261u,
  HPC_GPU_MALI_BIFROST_TILER_TRIANGLES = 262u,
  HPC_GPU_MALI_BIFROST_TILER_LINES = 263u,
  HPC_GPU_MALI_BIFROST_TILER_POINTS = 264u,
  HPC_GPU_MALI_BIFROST_TILER_FRONT_FACING = 265u,
  HPC_GPU_MALI_BIFROST_TILER_BACK_FACING = 266u,
  HPC_GPU_MALI_BIFROST_TILER_PRIM_VISIBLE = 267u,
  HPC_GPU_MALI_BIFROST_TILER_PRIM_CULLED = 268u,
  HPC_GPU_MALI_BIFROST_TILER_PRIM_CLIPPED = 269u,
  HPC_GPU_MALI_BIFROST_TILER_PRIM_SAT_CULLED = 270u,
  HPC_GPU_MALI_BIFROST_TILER_BUS_READ = 273u,
  HPC_GPU_MALI_BIFROST_TILER_BUS_WRITE = 275u,
  HPC_GPU_MALI_BIFROST_TILER_LOADING_DESC = 276u,
  HPC_GPU_MALI_BIFROST_TILER_IDVS_POS_SHAD_REQ = 277u,
  HPC_GPU_MALI_BIFROST_TILER_IDVS_POS_SHAD_WAIT = 278u,
  HPC_GPU_MALI_BIFROST_TILER_IDVS_POS_SHAD_STALL = 279u,
  HPC_GPU_MALI_BIFROST_TILER_IDVS_POS_FIFO_FULL = 280u,
  HPC_GPU_MALI_BIFROST_TILER_PREFETCH_STALL = 281u,
  HPC_GPU_MALI_BIFROST_TILER_VCACHE_HIT = 282u,
  HPC_GPU_MALI_BIFROST_TILER_VCACHE_MISS = 283u,
  HPC_GPU_MALI_BIFROST_TILER_VCACHE_LINE_WAIT = 284u,
  HPC_GPU_MALI_BIFROST_TILER_VFETCH_POS_READ_WAIT = 285u,
  HPC_GPU_MALI_BIFROST_TILER_VFETCH_VERTEX_WAIT = 286u,
  HPC_GPU_MALI_BIFROST_TILER_VFETCH_STALL = 287u,
  HPC_GPU_MALI_BIFROST_TILER_PRIMASSY_STALL = 288u,
  HPC_GPU_MALI_BIFROST_TILER_BBOX_GEN_STALL = 289u,
  HPC_GPU_MALI_BIFROST_TILER_IDVS_VBU_HIT = 290u,
  HPC_GPU_MALI_BIFROST_TILER_IDVS_VBU_MISS = 291u,
  HPC_GPU_MALI_BIFROST_TILER_IDVS_VBU_LINE_DEALLOCATE = 292u,
  HPC_GPU_MALI_BIFROST_TILER_IDVS_VAR_SHAD_REQ = 293u,
  HPC_GPU_MALI_BIFROST_TILER_IDVS_VAR_SHAD_STALL = 294u,
  HPC_GPU_MALI_BIFROST_TILER_BINNER_STALL = 295u,
  HPC_GPU_MALI_BIFROST_TILER_ITER_STALL = 296u,
  HPC_GPU_MALI_BIFROST_TILER_COMPRESS_MISS = 297u,
  HPC_GPU_MALI_BIFROST_TILER_COMPRESS_STALL = 298u,
  HPC_GPU_MALI_BIFROST_TILER_PCACHE_HIT = 299u,
  HPC_GPU_MALI_BIFROST_TILER_PCACHE_MISS = 300u,
  HPC_GPU_MALI_BIFROST_TILER_PCACHE_MISS_STALL = 301u,
  HPC_GPU_MALI_BIFROST_TILER_PCACHE_EVICT_STALL = 302u,
  HPC_GPU_MALI_BIFROST_TILER_PMGR_PTR_WR_STALL = 303u,
  HPC_GPU_MALI_BIFROST_TILER_PMGR_PTR_RD_STALL = 304u,
  HPC_GPU_MALI_BIFROST_TILER_PMGR_CMD_WR_STALL = 305u,
  HPC_GPU_MALI_BIFROST_TILER_WRBUF_ACTIVE = 306u,
  HPC_GPU_MALI_BIFROST_TILER_WRBUF_HIT = 307u,
  HPC_GPU_MALI_BIFROST_TILER_WRBUF_MISS = 308u,
  HPC_GPU_MALI_BIFROST_TILER_WRBUF_NO_FREE_LINE_STALL = 309u,
  HPC_GPU_MALI_BIFROST_TILER_WRBUF_NO_AXI_ID_STALL = 310u,
  HPC_GPU_MALI_BIFROST_TILER_WRBUF_AXI_STALL = 311u,
  HPC_GPU_MALI_BIFROST_TILER_UTLB_TRANS = 315u,
  HPC_GPU_MALI_BIFROST_TILER_UTLB_TRANS_HIT = 316u,
  HPC_GPU_MALI_BIFROST_TILER_UTLB_TRANS_STALL = 317u,
  HPC_GPU_MALI_BIFROST_TILER_UTLB_TRANS_MISS_DELAY = 318u,
  HPC_GPU_MALI_BIFROST_TILER_UTLB_MMU_REQ = 319u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_ACTIVE = 516u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_PRIMITIVES = 517u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_PRIM_RAST = 518u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_FPK_ACTIVE = 519u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_STARVING = 520u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_WARPS = 521u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_PARTIAL_WARPS = 522u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_QUADS_RAST = 523u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_QUADS_EZS_TEST = 524u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_QUADS_EZS_UPDATE = 525u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_QUADS_EZS_KILL = 526u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_LZS_TEST = 527u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_LZS_KILL = 528u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_PTILES = 530u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_FRAG_TRANS_ELIM = 531u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_QUAD_FPK_KILLER = 532u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_COMPUTE_ACTIVE = 534u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_COMPUTE_TASKS = 535u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_COMPUTE_WARPS = 536u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_COMPUTE_STARVING = 537u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_EXEC_CORE_ACTIVE = 538u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_EXEC_ACTIVE = 539u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_EXEC_INSTR_COUNT = 540u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_EXEC_INSTR_DIVERGED = 541u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_EXEC_INSTR_STARVING = 542u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_ARITH_INSTR_SINGLE_FMA = 543u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_ARITH_INSTR_DOUBLE = 544u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_ARITH_INSTR_MSG = 545u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_ARITH_INSTR_MSG_ONLY = 546u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_LS_MEM_READ_FULL = 556u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_LS_MEM_READ_SHORT = 557u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_LS_MEM_WRITE_FULL = 558u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_LS_MEM_WRITE_SHORT = 559u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_LS_MEM_ATOMIC = 560u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_VARY_INSTR = 561u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_VARY_SLOT_32 = 562u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_VARY_SLOT_16 = 563u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_ATTR_INSTR = 564u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_ARITH_INSTR_FP_MUL = 565u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_BEATS_RD_FTC = 566u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_BEATS_RD_FTC_EXT = 567u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_BEATS_RD_LSC = 568u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_BEATS_RD_LSC_EXT = 569u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_BEATS_RD_TEX = 570u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_BEATS_RD_TEX_EXT = 571u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_BEATS_RD_OTHER = 572u,
  HPC_GPU_MALI_BIFROST_SHADER_CORE_BEATS_WR_TIB = 574u,
  HPC_GPU_MALI_BIFROST_MEMORY_MMU_REQUESTS = 772u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_RD_MSG_IN = 784u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_RD_MSG_IN_STALL = 785u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_WR_MSG_IN = 786u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_WR_MSG_IN_STALL = 787u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_SNP_MSG_IN = 788u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_SNP_MSG_IN_STALL = 789u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_RD_MSG_OUT = 790u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_RD_MSG_OUT_STALL = 791u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_WR_MSG_OUT = 792u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_ANY_LOOKUP = 793u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_READ_LOOKUP = 794u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_WRITE_LOOKUP = 795u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_SNOOP_LOOKUP = 796u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_READ = 797u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_READ_NOSNP = 798u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_READ_UNIQUE = 799u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_READ_BEATS = 800u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_AR_STALL = 801u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_AR_CNT_Q1 = 802u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_AR_CNT_Q2 = 803u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_AR_CNT_Q3 = 804u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_RRESP_0_127 = 805u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_RRESP_128_191 = 806u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_RRESP_192_255 = 807u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_RRESP_256_319 = 808u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_RRESP_320_383 = 809u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_WRITE = 810u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_WRITE_NOSNP_FULL = 811u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_WRITE_NOSNP_PTL = 812u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_WRITE_SNP_FULL = 813u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_WRITE_SNP_PTL = 814u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_WRITE_BEATS = 815u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_W_STALL = 816u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_AW_CNT_Q1 = 817u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_AW_CNT_Q2 = 818u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_AW_CNT_Q3 = 819u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_SNOOP = 820u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_SNOOP_STALL = 821u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_SNOOP_RESP_CLEAN = 822u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_SNOOP_RESP_DATA = 823u,
  HPC_GPU_MALI_BIFROST_MEMORY_L2_EXT_SNOOP_INTERNAL = 824u
} hpc_gpu_mali_bifrost_counter_t;

//===--------------- END AUTOGENERATED REGION; DO NOT EDIT! ---------------===//

#endif  // HPC_GPU_MALI_COMMON_H_
