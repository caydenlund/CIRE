; ModuleID = 'benchmarks_c/rigidBody2.c'
source_filename = "benchmarks_c/rigidBody2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local double @ex0(double noundef %x1, double noundef %x2, double noundef %x3) local_unnamed_addr #0 {
entry:
  %mul = fmul double %x1, 2.000000e+00
  %mul1 = fmul double %mul, %x2
  %mul3 = fmul double %x3, 3.000000e+00
  %mul4 = fmul double %mul3, %x3
  %0 = tail call double @llvm.fmuladd.f64(double %mul1, double %x3, double %mul4)
  %1 = fneg double %x2
  %2 = fmul double %1, %x1
  %neg = fmul double %2, %x2
  %3 = tail call double @llvm.fmuladd.f64(double %neg, double %x3, double %0)
  %4 = tail call double @llvm.fmuladd.f64(double %mul3, double %x3, double %3)
  %sub = fsub double %4, %x2
  ret double %sub
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 20.0.0git (https://github.com/llvm/llvm-project.git 8ac924744e93258d0c490e2fa2d4518e24cb458d)"}
