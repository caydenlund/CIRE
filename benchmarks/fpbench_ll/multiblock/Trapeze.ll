; ModuleID = 'benchmarks_c/Trapeze.c'
source_filename = "benchmarks_c/Trapeze.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local double @ex0(double noundef %u) local_unnamed_addr #0 {
entry:
  %0 = insertelement <2 x double> poison, double %u, i64 0
  %1 = shufflevector <2 x double> %0, <2 x double> poison, <2 x i32> zeroinitializer
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %r.045 = phi double [ 0.000000e+00, %entry ], [ %13, %while.body ]
  %xa.044 = phi double [ 2.500000e-01, %entry ], [ %add, %while.body ]
  %add = fadd double %xa.044, 1.999900e+02
  %cmp1 = fcmp ogt double %add, 5.000000e+03
  %.add = select i1 %cmp1, double 5.000000e+03, double %add
  %2 = insertelement <2 x double> poison, double %xa.044, i64 0
  %3 = insertelement <2 x double> %2, double %.add, i64 1
  %4 = fmul <2 x double> %3, <double 0x3FE6666666666666, double 0x3FE6666666666666>
  %5 = fmul <2 x double> %3, %4
  %6 = fmul <2 x double> %3, <double -6.000000e-01, double -6.000000e-01>
  %7 = fmul <2 x double> %3, %6
  %8 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %5, <2 x double> %3, <2 x double> %7)
  %9 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %3, <2 x double> <double 9.000000e-01, double 9.000000e-01>, <2 x double> %8)
  %10 = fadd <2 x double> %9, <double -2.000000e-01, double -2.000000e-01>
  %11 = fdiv <2 x double> %1, %10
  %shift = shufflevector <2 x double> %11, <2 x double> poison, <2 x i32> <i32 1, i32 poison>
  %12 = fadd <2 x double> %11, %shift
  %add19 = extractelement <2 x double> %12, i64 0
  %mul20 = fmul double %add19, 5.000000e-01
  %13 = tail call double @llvm.fmuladd.f64(double %mul20, double 1.999900e+02, double %r.045)
  %cmp23 = fcmp uge double %add, 5.000000e+03
  br i1 %cmp23, label %while.end, label %while.body, !llvm.loop !5

while.end:                                        ; preds = %while.body
  ret double %13
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x double> @llvm.fmuladd.v2f64(<2 x double>, <2 x double>, <2 x double>) #2

attributes #0 = { nofree norecurse nosync nounwind memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 20.0.0git (https://github.com/llvm/llvm-project.git 8ac924744e93258d0c490e2fa2d4518e24cb458d)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
