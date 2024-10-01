; ModuleID = 'benchmarks_c/instantaneousCurrent.c'
source_filename = "benchmarks_c/instantaneousCurrent.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nounwind willreturn memory(write) uwtable
define dso_local double @ex0(double noundef %t, double noundef %resistance, double noundef %frequency, double noundef %inductance, double noundef %maxVoltage) local_unnamed_addr #0 {
entry:
  %mul1 = fmul double %frequency, 0x401921FB54442EEA
  %mul2 = fmul double %mul1, %inductance
  %mul4 = fmul double %mul2, %mul2
  %0 = tail call double @llvm.fmuladd.f64(double %resistance, double %resistance, double %mul4)
  %1 = fneg double %mul2
  %2 = insertelement <2 x double> poison, double %1, i64 0
  %3 = insertelement <2 x double> %2, double %resistance, i64 1
  %4 = insertelement <2 x double> poison, double %maxVoltage, i64 0
  %5 = shufflevector <2 x double> %4, <2 x double> poison, <2 x i32> zeroinitializer
  %6 = fmul <2 x double> %3, %5
  %7 = insertelement <2 x double> poison, double %0, i64 0
  %8 = shufflevector <2 x double> %7, <2 x double> poison, <2 x i32> zeroinitializer
  %9 = fdiv <2 x double> %6, %8
  %10 = extractelement <2 x double> %9, i64 0
  %11 = fmul <2 x double> %9, %9
  %mul9 = extractelement <2 x double> %11, i64 0
  %12 = extractelement <2 x double> %9, i64 1
  %13 = tail call double @llvm.fmuladd.f64(double %12, double %12, double %mul9)
  %sqrt = tail call double @llvm.sqrt.f64(double %13)
  %div10 = fdiv double %10, %12
  %call11 = tail call double @atan(double noundef %div10) #4, !tbaa !5
  %14 = tail call double @llvm.fmuladd.f64(double %mul1, double %t, double %call11)
  %call15 = tail call double @cos(double noundef %14) #4, !tbaa !5
  %mul16 = fmul double %sqrt, %call15
  ret double %mul16
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @atan(double noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @cos(double noundef) local_unnamed_addr #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.sqrt.f64(double) #3

attributes #0 = { mustprogress nofree nounwind willreturn memory(write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 20.0.0git (https://github.com/llvm/llvm-project.git 8ac924744e93258d0c490e2fa2d4518e24cb458d)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
