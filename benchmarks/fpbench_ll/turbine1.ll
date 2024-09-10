; ModuleID = 'benchmarks_c/turbine1.c'
source_filename = "benchmarks_c/turbine1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local double @ex0(double noundef %v, double noundef %w, double noundef %r) local_unnamed_addr #0 {
entry:
  %0 = insertelement <2 x double> <double poison, double 1.000000e+00>, double %r, i64 0
  %1 = insertelement <2 x double> %0, double %v, i64 1
  %2 = fmul <2 x double> %0, %1
  %3 = fsub <2 x double> %0, %1
  %4 = shufflevector <2 x double> %2, <2 x double> %3, <2 x i32> <i32 0, i32 3>
  %5 = tail call double @llvm.fmuladd.f64(double %v, double -2.000000e+00, double 3.000000e+00)
  %mul2 = fmul double %5, 1.250000e-01
  %mul3 = fmul double %w, %w
  %mul4 = fmul double %mul3, %r
  %mul5 = fmul double %mul4, %r
  %mul6 = fmul double %mul2, %mul5
  %6 = insertelement <2 x double> <double 2.000000e+00, double poison>, double %mul6, i64 1
  %7 = fdiv <2 x double> %6, %4
  %8 = extractelement <2 x double> %7, i64 0
  %add = fadd double %8, 3.000000e+00
  %9 = extractelement <2 x double> %7, i64 1
  %sub8 = fsub double %add, %9
  %sub9 = fadd double %sub8, -4.500000e+00
  ret double %sub9
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
