; ModuleID = '/home/tanmay/Documents/Tools/CIRE/benchmarks/fpbench_c/jetEngine.c'
source_filename = "/home/tanmay/Documents/Tools/CIRE/benchmarks/fpbench_c/jetEngine.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local double @ex0(double noundef %x1, double noundef %x2) local_unnamed_addr #0 {
entry:
  %mul = fmul double %x1, 3.000000e+00
  %mul2 = fmul double %x2, 2.000000e+00
  %0 = tail call double @llvm.fmuladd.f64(double %mul, double %x1, double %mul2)
  %sub = fsub double %0, %x1
  %neg = fneg double %mul2
  %1 = tail call double @llvm.fmuladd.f64(double %mul, double %x1, double %neg)
  %sub6 = fsub double %1, %x1
  %2 = tail call double @llvm.fmuladd.f64(double %x1, double %x1, double 1.000000e+00)
  %div = fdiv double %sub, %2
  %div8 = fdiv double %sub6, %2
  %mul9 = fmul double %x1, 2.000000e+00
  %mul10 = fmul double %mul9, %div
  %sub11 = fadd double %div, -3.000000e+00
  %mul13 = fmul double %x1, %x1
  %3 = tail call double @llvm.fmuladd.f64(double %div, double 4.000000e+00, double -6.000000e+00)
  %mul15 = fmul double %mul13, %3
  %4 = tail call double @llvm.fmuladd.f64(double %mul10, double %sub11, double %mul15)
  %mul18 = fmul double %mul, %x1
  %mul19 = fmul double %mul18, %div
  %5 = tail call double @llvm.fmuladd.f64(double %4, double %2, double %mul19)
  %6 = tail call double @llvm.fmuladd.f64(double %mul13, double %x1, double %5)
  %add = fadd double %6, %x1
  %7 = tail call double @llvm.fmuladd.f64(double %div8, double 3.000000e+00, double %add)
  %add23 = fadd double %7, %x1
  ret double %add23
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
