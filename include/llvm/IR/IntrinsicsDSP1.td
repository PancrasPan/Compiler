//===- IntrinsicsDSP.td - Defines DSP intrinsics -----------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
//
//===----------------------------------------------------------------------===//
//
// This file defines all of the DSP-specific intrinsics.
//
//===----------------------------------------------------------------------===//



let TargetPrefix = "dsp" in {
  def int_dsp_vmax_10  :GCCBuiltin<"__builtin_dsp_vmax_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;
  def int_dsp_vmax_20  :GCCBuiltin<"__builtin_dsp_vmax_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;
  def int_dsp_vmax_40  :GCCBuiltin<"__builtin_dsp_vmax_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;
  def int_dsp_vmin_10  :GCCBuiltin<"__builtin_dsp_vmin_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;
  def int_dsp_vmin_20  :GCCBuiltin<"__builtin_dsp_vmin_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;
  def int_dsp_vmin_40  :GCCBuiltin<"__builtin_dsp_vmin_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;
  def int_dsp_vsum_10  :GCCBuiltin<"__builtin_dsp_vsum_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty],
                  [IntrNoMem]>;
  def int_dsp_vsum_20  :GCCBuiltin<"__builtin_dsp_vsum_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty],
                  [IntrNoMem]>;
  def int_dsp_vsum_40  :GCCBuiltin<"__builtin_dsp_vsum_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty],
                  [IntrNoMem]>;
  def int_dsp_max :GCCBuiltin<"__builtin_dsp_max">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty,llvm_i32_ty],
                  [IntrNoMem]>;
  def int_dsp_min :GCCBuiltin<"__builtin_dsp_min">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty,llvm_i32_ty],
                  [IntrNoMem]>;
def int_dsp_abs_qb: GCCBuiltin<"__builtin_dsp_abs_qb">,
  Intrinsic<[llvm_i32_ty], [llvm_i32_ty], []>;

def int_dsp_test_qb: GCCBuiltin<"__builtin_dsp_test_qb">,
  Intrinsic<[llvm_i32_ty], [llvm_i32_ty,llvm_i32_ty], []>;

def int_dsp_testi_qb: GCCBuiltin<"__builtin_dsp_testi_qb">,
  Intrinsic<[llvm_i32_ty], [llvm_i32_ty,llvm_i32_ty], []>;

def int_dsp_cbw_qb: GCCBuiltin<"__builtin_dsp_cbw_qb">,
  Intrinsic<[llvm_i32_ty], [llvm_i32_ty], []>;

def int_dsp_chw_qb: GCCBuiltin<"__builtin_dsp_chw_qb">,
  Intrinsic<[llvm_i32_ty], [llvm_i32_ty], []>;

def int_dsp_bfext_qb: GCCBuiltin<"__builtin_dsp_bfext_qb">,
  Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], []>;

def int_dsp_bfextu_qb: GCCBuiltin<"__builtin_dsp_bfextu_qb">,
  Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], []>;

def int_dsp_bfst_qb: GCCBuiltin<"__builtin_dsp_bfst_qb">,
  Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], []>;

def int_dsp_bst_qb: GCCBuiltin<"__builtin_dsp_bst_qb">,
  Intrinsic<[llvm_i32_ty], [llvm_i32_ty], []>;    

def int_dsp_bclr_qb: GCCBuiltin<"__builtin_dsp_bclr_qb">,
  Intrinsic<[llvm_i32_ty], [llvm_i32_ty], []>;				

def int_dsp_veq_10  :GCCBuiltin<"__builtin_dsp_veq_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_veq_20  :GCCBuiltin<"__builtin_dsp_veq_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_veq_40  :GCCBuiltin<"__builtin_dsp_veq_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vgt_10  :GCCBuiltin<"__builtin_dsp_vgt_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_vgt_20  :GCCBuiltin<"__builtin_dsp_vgt_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vgt_40  :GCCBuiltin<"__builtin_dsp_vgt_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vlt_10  :GCCBuiltin<"__builtin_dsp_vlt_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_vlt_20  :GCCBuiltin<"__builtin_dsp_vlt_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vlt_40  :GCCBuiltin<"__builtin_dsp_vlt_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vge_10  :GCCBuiltin<"__builtin_dsp_vge_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_vge_20  :GCCBuiltin<"__builtin_dsp_vge_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vge_40  :GCCBuiltin<"__builtin_dsp_vge_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vle_10  :GCCBuiltin<"__builtin_dsp_vle_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_vle_20  :GCCBuiltin<"__builtin_dsp_vle_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vle_40  :GCCBuiltin<"__builtin_dsp_vle_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vadd_10  :GCCBuiltin<"__builtin_dsp_vadd_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_vadd_20  :GCCBuiltin<"__builtin_dsp_vadd_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vadd_40  :GCCBuiltin<"__builtin_dsp_vadd_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vsub_10  :GCCBuiltin<"__builtin_dsp_vsub_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_vsub_20  :GCCBuiltin<"__builtin_dsp_vsub_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vsub_40  :GCCBuiltin<"__builtin_dsp_vsub_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vsl_10  :GCCBuiltin<"__builtin_dsp_vsl_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_vsl_20  :GCCBuiltin<"__builtin_dsp_vsl_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vsl_40  :GCCBuiltin<"__builtin_dsp_vsl_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vsls_10  :GCCBuiltin<"__builtin_dsp_vsls_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_vsls_20  :GCCBuiltin<"__builtin_dsp_vsls_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vsls_40  :GCCBuiltin<"__builtin_dsp_vsls_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vsra_10  :GCCBuiltin<"__builtin_dsp_vsra_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_vsra_20  :GCCBuiltin<"__builtin_dsp_vsra_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vsra_40  :GCCBuiltin<"__builtin_dsp_vsra_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vsrl_10  :GCCBuiltin<"__builtin_dsp_vsrl_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_vsrl_20  :GCCBuiltin<"__builtin_dsp_vsrl_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vsrl_40  :GCCBuiltin<"__builtin_dsp_vsrl_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vabs_10  :GCCBuiltin<"__builtin_dsp_vabs_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_vabs_20  :GCCBuiltin<"__builtin_dsp_vabs_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vabs_40  :GCCBuiltin<"__builtin_dsp_vabs_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vfmul_10  :GCCBuiltin<"__builtin_dsp_vfmul_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_vfmul_20  :GCCBuiltin<"__builtin_dsp_vfmul_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vfmul_40  :GCCBuiltin<"__builtin_dsp_vfmul_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vfmac_10  :GCCBuiltin<"__builtin_dsp_vfmac_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_vfmac_20  :GCCBuiltin<"__builtin_dsp_vfmac_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vfmac_40  :GCCBuiltin<"__builtin_dsp_vfmac_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty,llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vcmac_10  :GCCBuiltin<"__builtin_dsp_vcmac_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                  [IntrNoMem]>;

def int_dsp_vcmac_20  :GCCBuiltin<"__builtin_dsp_vcmac_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vcmul_20  :GCCBuiltin<"__builtin_dsp_vcmul_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

def int_dsp_vcmulr_40  :GCCBuiltin<"__builtin_dsp_vcmulr_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vcmuli_40  :GCCBuiltin<"__builtin_dsp_vcmuli_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

def int_dsp_vloadr_8 : GCCBuiltin<"__builtin_dsp_vloadr_8">,
        Intrinsic<[llvm_v64i8_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vloadr_16 : GCCBuiltin<"__builtin_dsp_vloadr_16">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vloadr_32 : GCCBuiltin<"__builtin_dsp_vloadr_32">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_loadu_8 : GCCBuiltin<"__builtin_dsp_loadu_8">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_loadu_16 : GCCBuiltin<"__builtin_dsp_loadu_16">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_loadu_32 : GCCBuiltin<"__builtin_dsp_loadu_32">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vloadu_8 : GCCBuiltin<"__builtin_dsp_vloadu_8">,
        Intrinsic<[llvm_v64i8_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vloadu_16 : GCCBuiltin<"__builtin_dsp_vloadu_16">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vloadu_32 : GCCBuiltin<"__builtin_dsp_vloadu_32">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vloadur_8 : GCCBuiltin<"__builtin_dsp_vloadur_8">,
        Intrinsic<[llvm_v64i8_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vloadur_16 : GCCBuiltin<"__builtin_dsp_vloadur_16">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vloadur_32 : GCCBuiltin<"__builtin_dsp_vloadur_32">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_storeu_8 : GCCBuiltin<"__builtin_dsp_storeu_8">,
        Intrinsic<[], [llvm_i32_ty,llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_storeu_16 : GCCBuiltin<"__builtin_dsp_storeu_16">,
        Intrinsic<[], [llvm_i32_ty,llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_storeu_32 : GCCBuiltin<"__builtin_dsp_storeu_32">,
        Intrinsic<[], [llvm_i32_ty,llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_storeu_10 : GCCBuiltin<"__builtin_dsp_storeu_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_storeu_20 : GCCBuiltin<"__builtin_dsp_storeu_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_storeu_40 : GCCBuiltin<"__builtin_dsp_storeu_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vstoreu_10 : GCCBuiltin<"__builtin_dsp_vstoreu_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vstoreu_20 : GCCBuiltin<"__builtin_dsp_vstoreu_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vstoreu_40 : GCCBuiltin<"__builtin_dsp_vstoreu_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_loadvh : GCCBuiltin<"__builtin_dsp_loadvh">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_loadvl : GCCBuiltin<"__builtin_dsp_loadvl">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_storevh : GCCBuiltin<"__builtin_dsp_storevh">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_storevl : GCCBuiltin<"__builtin_dsp_storevl">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_loaduvh : GCCBuiltin<"__builtin_dsp_loaduvh">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_loaduvl : GCCBuiltin<"__builtin_dsp_loaduvl">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_storeuvh : GCCBuiltin<"__builtin_dsp_storeuvh">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_storeuvl : GCCBuiltin<"__builtin_dsp_storeuvl">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_loado_16 : GCCBuiltin<"__builtin_dsp_loado_16">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_loado_32 : GCCBuiltin<"__builtin_dsp_loado_32">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vloado_8 : GCCBuiltin<"__builtin_dsp_vloado_8">,
        Intrinsic<[llvm_v64i8_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vloado_16 : GCCBuiltin<"__builtin_dsp_vloado_16">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vloado_32 : GCCBuiltin<"__builtin_dsp_vloado_32">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_storeo_16 : GCCBuiltin<"__builtin_dsp_storeo_16">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_storeo_32 : GCCBuiltin<"__builtin_dsp_storeo_32">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vstoreo_10 : GCCBuiltin<"__builtin_dsp_vstoreo_10">,
        Intrinsic<[llvm_v64i8_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vstoreo_20 : GCCBuiltin<"__builtin_dsp_vstoreo_20">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_i32_ty],[]>;

def int_dsp_vstoreo_40 : GCCBuiltin<"__builtin_dsp_vstoreo_40">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i32_ty, llvm_i32_ty],[]>;
//ppp-start		
def int_dsp_vnot : GCCBuiltin<"__builtin_dsp_vnot">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty],[IntrNoMem]>;
def int_dsp_vand : GCCBuiltin<"__builtin_dsp_vand">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],[IntrNoMem]>;
def int_dsp_vxor : GCCBuiltin<"__builtin_dsp_vxor">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],[IntrNoMem]>;
def int_dsp_vor : GCCBuiltin<"__builtin_dsp_vor">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],[IntrNoMem]>;
def int_dsp_not : GCCBuiltin<"__builtin_dsp_not">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty],[IntrNoMem]>;
def int_dsp_vmovcv2v : GCCBuiltin<"__builtin_dsp_vmovcv2v">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],[IntrNoMem]>;
def int_dsp_vsel : GCCBuiltin<"__builtin_dsp_vsel">,
        Intrinsic<[llvm_v64i8_ty], [llvm_i32_ty, llvm_v32i16_ty,llvm_v32i16_ty],[IntrNoMem]>;
//ppp-end

}
