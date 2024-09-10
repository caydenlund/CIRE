; ModuleID = 'benchmarks_c/Pendulum.c'
source_filename = "benchmarks_c/Pendulum.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind memory(write) uwtable
define dso_local double @ex0(double noundef %t0, double noundef %w0, double noundef %N) local_unnamed_addr #0 {
entry:
  %cmp = fcmp ogt double %N, 0.000000e+00
  br i1 %cmp, label %while.body.preheader, label %while.end

while.body.preheader:                             ; preds = %entry
  %0 = insertelement <2 x double> poison, double %t0, i64 0
  %1 = insertelement <2 x double> %0, double %w0, i64 1
  br label %while.body

while.body:                                       ; preds = %while.body.preheader, %while.body
  %n.026 = phi double [ %add, %while.body ], [ 0.000000e+00, %while.body.preheader ]
  %2 = phi <2 x double> [ %9, %while.body ], [ %1, %while.body.preheader ]
  %3 = extractelement <2 x double> %2, i64 0
  %call = tail call double @sin(double noundef %3) #4, !tbaa !5
  %mul = fmul double %call, 0xC0139D013A92A305
  %4 = extractelement <2 x double> %2, i64 1
  %5 = tail call double @llvm.fmuladd.f64(double %mul, double 5.000000e-03, double %4)
  %6 = tail call double @llvm.fmuladd.f64(double %4, double 5.000000e-03, double %3)
  %call8 = tail call double @sin(double noundef %6) #4, !tbaa !5
  %mul9 = fmul double %call8, 0xC0139D013A92A305
  %7 = insertelement <2 x double> poison, double %5, i64 0
  %8 = insertelement <2 x double> %7, double %mul9, i64 1
  %9 = tail call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %8, <2 x double> <double 1.000000e-02, double 1.000000e-02>, <2 x double> %2)
  %add = fadd double %n.026, 1.000000e+00
  %cmp11 = fcmp olt double %add, %N
  br i1 %cmp11, label %while.body, label %while.end.loopexit, !llvm.loop !9

while.end.loopexit:                               ; preds = %while.body
  %10 = extractelement <2 x double> %9, i64 0
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit, %entry
  %t.0.lcssa = phi double [ %t0, %entry ], [ %10, %while.end.loopexit ]
  ret double %t.0.lcssa
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @sin(double noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x double> @llvm.fmuladd.v2f64(<2 x double>, <2 x double>, <2 x double>) #3

attributes #0 = { nofree nounwind memory(write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
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
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
