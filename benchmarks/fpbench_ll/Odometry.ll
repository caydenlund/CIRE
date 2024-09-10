; ModuleID = 'benchmarks_c/Odometry.c'
source_filename = "benchmarks_c/Odometry.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local float @ex0(float noundef %sr_42_, float noundef %sl_42_) local_unnamed_addr #0 {
entry:
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %j.076 = phi float [ 0.000000e+00, %entry ], [ %tmp_4.0, %while.body ]
  %sr.075 = phi float [ %sr_42_, %entry ], [ %tmp_3.0, %while.body ]
  %sl.074 = phi float [ %sl_42_, %entry ], [ %sr.0.sl.0, %while.body ]
  %t.073.int = phi i32 [ 0, %entry ], [ %add23.int, %while.body ]
  %theta.072 = phi float [ 0xBFEF851EC0000000, %entry ], [ %add22, %while.body ]
  %x.070 = phi float [ 0.000000e+00, %entry ], [ %3, %while.body ]
  %mul = fmul float %sl.074, 0x4028AE1480000000
  %mul1 = fmul float %sr.075, 0x4028AE1480000000
  %add = fadd float %mul, %mul1
  %mul2 = fmul float %add, 5.000000e-01
  %sub = fsub float %mul1, %mul
  %mul3 = fmul float %sub, 0x3FB99999A0000000
  %0 = tail call float @llvm.fmuladd.f32(float %mul3, float 5.000000e-01, float %theta.072)
  %mul5 = fmul float %0, %0
  %neg = fneg float %mul5
  %1 = tail call float @llvm.fmuladd.f32(float %neg, float 5.000000e-01, float 1.000000e+00)
  %mul8 = fmul float %0, %mul5
  %mul9 = fmul float %0, %mul8
  %2 = tail call float @llvm.fmuladd.f32(float %mul9, float 0x3FA5555560000000, float %1)
  %3 = tail call float @llvm.fmuladd.f32(float %mul2, float %2, float %x.070)
  %add22 = fadd float %theta.072, %mul3
  %add23.int = add nuw nsw i32 %t.073.int, 1
  %cmp24 = fcmp oeq float %j.076, 5.000000e+01
  %sr.0.sl.0 = select i1 %cmp24, float %sr.075, float %sl.074
  %tmp_3.0 = select i1 %cmp24, float %sl.074, float %sr.075
  %add35 = fadd float %j.076, 1.000000e+00
  %tmp_4.0 = select i1 %cmp24, float 0.000000e+00, float %add35
  %exitcond = icmp eq i32 %add23.int, 1000
  br i1 %exitcond, label %while.end, label %while.body, !llvm.loop !5

while.end:                                        ; preds = %while.body
  ret float %3
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #1

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
