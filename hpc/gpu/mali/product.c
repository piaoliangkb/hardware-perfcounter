#include "hpc/gpu/mali/product.h"

//===-------------- BEGIN AUTOGENERATED REGION; DO NOT EDIT! --------------===//

hpc_gpu_mali_counter_layout_t hpc_gpu_mali_get_layout(uint16_t gpu_id) {
  if ((gpu_id & 0xf00fu) == 0x9005u) return HPC_GPU_MALI_COUNTER_LAYOUT_TBOX;
  if ((gpu_id & 0xf00fu) == 0x9002u) return HPC_GPU_MALI_COUNTER_LAYOUT_TBOX;
  if ((gpu_id & 0xf00fu) == 0x9004u) return HPC_GPU_MALI_COUNTER_LAYOUT_TOTX;
  if ((gpu_id & 0xf00fu) == 0x9000u) return HPC_GPU_MALI_COUNTER_LAYOUT_TTRX;
  if ((gpu_id & 0xf00fu) == 0x9003u) return HPC_GPU_MALI_COUNTER_LAYOUT_TNAX;
  if ((gpu_id & 0xf00fu) == 0x9001u) return HPC_GPU_MALI_COUNTER_LAYOUT_TNAX;
  if ((gpu_id & 0xf00fu) == 0x7001u) return HPC_GPU_MALI_COUNTER_LAYOUT_TNOX;
  if ((gpu_id & 0xf00fu) == 0x7002u) return HPC_GPU_MALI_COUNTER_LAYOUT_TGOX;
  if ((gpu_id & 0xf00fu) == 0x7000u) return HPC_GPU_MALI_COUNTER_LAYOUT_TSIX;
  if ((gpu_id & 0xf00fu) == 0x7003u) return HPC_GPU_MALI_COUNTER_LAYOUT_TDVX;
  if ((gpu_id & 0xf00fu) == 0x6001u) return HPC_GPU_MALI_COUNTER_LAYOUT_THEX;
  if ((gpu_id & 0xf00fu) == 0x6000u) return HPC_GPU_MALI_COUNTER_LAYOUT_TMIX;
  if ((gpu_id & 0xffffu) == 0x0880u) return HPC_GPU_MALI_COUNTER_LAYOUT_T88X;
  if ((gpu_id & 0xffffu) == 0x0860u) return HPC_GPU_MALI_COUNTER_LAYOUT_T86X;
  if ((gpu_id & 0xffffu) == 0x0830u) return HPC_GPU_MALI_COUNTER_LAYOUT_T83X;
  if ((gpu_id & 0xffffu) == 0x0820u) return HPC_GPU_MALI_COUNTER_LAYOUT_T82X;
  return HPC_GPU_MALI_COUNTER_LAYOUT_UNKNOWN;
}

//===--------------- END AUTOGENERATED REGION; DO NOT EDIT! ---------------===//
