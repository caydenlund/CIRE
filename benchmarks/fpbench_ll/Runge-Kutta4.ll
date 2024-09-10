; ModuleID = 'benchmarks_c/Runge-Kutta4.c'
source_filename = "benchmarks_c/Runge-Kutta4.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local float @ex0(float noundef %h, float noundef %y_n_42_, float noundef %c) local_unnamed_addr #0 {
entry:
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %e.049 = phi float [ 1.000000e+00, %entry ], [ %sub21, %while.body ]
  %sub21 = fadd float %e.049, 0xBF747AE140000000
  %cmp22 = fcmp ule float %sub21, 0x3F747AE140000000
  br i1 %cmp22, label %while.end, label %while.body, !llvm.loop !5

while.end:                                        ; preds = %while.body
  %0 = tail call float @llvm.fabs.f32(float %sub21)
  ret float %0
}

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
