; ModuleID = 'benchmarks_c/Lead-LagSystem.c'
source_filename = "benchmarks_c/Lead-LagSystem.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local float @ex0(float noundef %y, float noundef %yd) local_unnamed_addr #0 {
entry:
  %sub = fsub float %y, %yd
  %cmp1 = fcmp olt float %sub, -1.000000e+00
  %cmp3 = fcmp ogt float %sub, 1.000000e+00
  %.sub = select i1 %cmp3, float 1.000000e+00, float %sub
  %tmp_1.0 = select i1 %cmp1, float -1.000000e+00, float %.sub
  %mul13 = fmul float %tmp_1.0, 0.000000e+00
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %xc1.031 = phi float [ 0.000000e+00, %entry ], [ %3, %while.body ]
  %xc0.030 = phi float [ 0.000000e+00, %entry ], [ %1, %while.body ]
  %0 = tail call float @llvm.fmuladd.f32(float %xc1.031, float 0xBFA99999A0000000, float %tmp_1.0)
  %1 = tail call float @llvm.fmuladd.f32(float %xc0.030, float 0x3FDFEF9DC0000000, float %0)
  %2 = fadd float %mul13, %xc1.031
  %3 = tail call float @llvm.fmuladd.f32(float %1, float 0x3F847AE140000000, float %2)
  %sub14 = fsub float %tmp_1.0, %3
  %4 = tail call float @llvm.fabs.f32(float %sub14)
  %cmp15 = fcmp ule float %4, 0x3F847AE140000000
  br i1 %cmp15, label %while.end, label %while.body, !llvm.loop !5

while.end:                                        ; preds = %while.body
  ret float %3
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #1

attributes #0 = { nofree norecurse nosync nounwind memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 20.0.0git (https://github.com/llvm/llvm-project.git 8ac924744e93258d0c490e2fa2d4518e24cb458d)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
