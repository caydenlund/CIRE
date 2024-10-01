; ModuleID = 'benchmarks_c/matrixDeterminant.c'
source_filename = "benchmarks_c/matrixDeterminant.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef double @ex0(double noundef %a, double noundef %b, double noundef %c, double noundef %d, double noundef %e, double noundef %f, double noundef %g, double noundef %h, double noundef %i) local_unnamed_addr #0 {
entry:
  %0 = insertelement <2 x double> poison, double %c, i64 0
  %1 = insertelement <2 x double> %0, double %a, i64 1
  %2 = insertelement <2 x double> poison, double %e, i64 0
  %3 = shufflevector <2 x double> %2, <2 x double> poison, <2 x i32> zeroinitializer
  %4 = fmul <2 x double> %1, %3
  %5 = insertelement <2 x double> poison, double %b, i64 0
  %6 = shufflevector <2 x double> %5, <2 x double> poison, <2 x i32> zeroinitializer
  %7 = insertelement <2 x double> poison, double %d, i64 0
  %8 = insertelement <2 x double> %7, double %f, i64 1
  %9 = fmul <2 x double> %6, %8
  %10 = fmul <2 x double> %1, %8
  %11 = shufflevector <2 x double> %10, <2 x double> poison, <2 x i32> <i32 1, i32 0>
  %12 = insertelement <2 x double> poison, double %i, i64 0
  %13 = insertelement <2 x double> %12, double %g, i64 1
  %14 = fmul <2 x double> %9, %13
  %15 = shufflevector <2 x double> %13, <2 x double> poison, <2 x i32> <i32 1, i32 0>
  %16 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %4, <2 x double> %15, <2 x double> %14)
  %17 = insertelement <2 x double> poison, double %h, i64 0
  %18 = shufflevector <2 x double> %17, <2 x double> poison, <2 x i32> zeroinitializer
  %19 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %11, <2 x double> %18, <2 x double> %16)
  %shift = shufflevector <2 x double> %19, <2 x double> poison, <2 x i32> <i32 1, i32 poison>
  %20 = fsub <2 x double> %shift, %19
  %sub = extractelement <2 x double> %20, i64 0
  ret double %sub
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x double> @llvm.fmuladd.v2f64(<2 x double>, <2 x double>, <2 x double>) #1

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 20.0.0git (https://github.com/llvm/llvm-project.git 8ac924744e93258d0c490e2fa2d4518e24cb458d)"}
