; ModuleID = 'bspline3.b024775ae823b2b3-cgu.0'
source_filename = "bspline3.b024775ae823b2b3-cgu.0"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind nonlazybind willreturn memory(none)
define dso_local noundef double @ex0(double noundef %u) unnamed_addr #0 {
start:
  %_4 = fmul double %u, %u
  %0 = fneg double %u
  %_2 = fmul double %_4, %0
  %_0 = fdiv double %_2, 6.000000e+00
  ret double %_0
}

; __rustc::rust_begin_unwind
; Function Attrs: nofree norecurse noreturn nosync nounwind nonlazybind memory(none)
define hidden void @_RNvCscSpY9Juk0HT_7___rustc17rust_begin_unwind(ptr noalias nocapture noundef readonly align 8 dereferenceable(24) %_info) unnamed_addr #1 {
start:
  br label %bb1

bb1:                                              ; preds = %bb1, %start
  br label %bb1
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind nonlazybind willreturn memory(none) "probe-stack"="inline-asm" "target-cpu"="x86-64" }
attributes #1 = { nofree norecurse noreturn nosync nounwind nonlazybind memory(none) "probe-stack"="inline-asm" "target-cpu"="x86-64" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 8, !"PIC Level", i32 2}
!1 = !{i32 7, !"PIE Level", i32 2}
!2 = !{i32 2, !"RtLibUseGOT", i32 1}
!3 = !{!"rustc version 1.87.0 (17067e9ac 2025-05-09)"}
