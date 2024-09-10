; ModuleID = 'benchmarks_c/floudas1.c'
source_filename = "benchmarks_c/floudas1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local double @ex0(double noundef %x1, double noundef %x2, double noundef %x3, double noundef %x4, double noundef %x5, double noundef %x6) local_unnamed_addr #0 {
entry:
  %sub = fadd double %x1, -2.000000e+00
  %mul = fmul double %sub, %sub
  %sub3 = fadd double %x2, -2.000000e+00
  %0 = fneg double %sub3
  %neg = fmul double %sub3, %0
  %1 = tail call double @llvm.fmuladd.f64(double %mul, double -2.500000e+01, double %neg)
  %sub6 = fadd double %x3, -1.000000e+00
  %neg9 = fneg double %sub6
  %2 = tail call double @llvm.fmuladd.f64(double %neg9, double %sub6, double %1)
  %sub10 = fadd double %x4, -4.000000e+00
  %neg13 = fneg double %sub10
  %3 = tail call double @llvm.fmuladd.f64(double %neg13, double %sub10, double %2)
  %sub14 = fadd double %x5, -1.000000e+00
  %neg17 = fneg double %sub14
  %4 = tail call double @llvm.fmuladd.f64(double %neg17, double %sub14, double %3)
  %sub18 = fadd double %x6, -4.000000e+00
  %neg21 = fneg double %sub18
  %5 = tail call double @llvm.fmuladd.f64(double %neg21, double %sub18, double %4)
  ret double %5
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
