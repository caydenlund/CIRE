; ModuleID = 'benchmarks_c/test04_dqmom9.c'
source_filename = "benchmarks_c/test04_dqmom9.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local double @ex0(double noundef %m0, double noundef %m1, double noundef %m2, double noundef %w0, double noundef %w1, double noundef %w2, double noundef %a0, double noundef %a1, double noundef %a2) local_unnamed_addr #0 {
entry:
  %sub = fsub double 0.000000e+00, %m2
  %mul = fmul double %sub, %w2
  %div = fdiv double %a2, %w2
  %mul3 = fmul double %div, %div
  %mul4 = fmul double %mul3, 3.000000e+00
  %mul5 = fmul double %mul, %mul4
  %0 = insertelement <2 x double> poison, double %m1, i64 0
  %1 = insertelement <2 x double> %0, double %m0, i64 1
  %2 = fsub <2 x double> zeroinitializer, %1
  %3 = insertelement <2 x double> poison, double %a1, i64 0
  %4 = insertelement <2 x double> %3, double %a0, i64 1
  %5 = insertelement <2 x double> poison, double %w1, i64 0
  %6 = insertelement <2 x double> %5, double %w0, i64 1
  %7 = fdiv <2 x double> %4, %6
  %8 = fmul <2 x double> %2, %6
  %9 = fmul <2 x double> %7, %7
  %10 = fmul <2 x double> %9, <double 3.000000e+00, double 3.000000e+00>
  %11 = fmul <2 x double> %8, %10
  %12 = fsub double 0.000000e+00, %mul5
  %13 = extractelement <2 x double> %11, i64 0
  %14 = fsub double %12, %13
  %15 = extractelement <2 x double> %11, i64 1
  %16 = fsub double %14, %15
  ret double %16
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 20.0.0git (https://github.com/llvm/llvm-project.git 8ac924744e93258d0c490e2fa2d4518e24cb458d)"}
