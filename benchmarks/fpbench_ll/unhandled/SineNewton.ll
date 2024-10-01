; ModuleID = 'benchmarks_c/SineNewton.c'
source_filename = "benchmarks_c/SineNewton.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind memory(write) uwtable
define dso_local double @ex0(double noundef %x0) local_unnamed_addr #0 {
entry:
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %x.031 = phi double [ %x0, %entry ], [ %sub15, %while.body ]
  %i.030.int = phi i32 [ 0, %entry ], [ %add16.int, %while.body ]
  %call = tail call double @pow(double noundef %x.031, double noundef 3.000000e+00) #2, !tbaa !5
  %div = fdiv double %call, 6.000000e+00
  %call1 = tail call double @pow(double noundef %x.031, double noundef 5.000000e+00) #2, !tbaa !5
  %call3 = tail call double @pow(double noundef %x.031, double noundef 7.000000e+00) #2, !tbaa !5
  %mul = fmul double %x.031, %x.031
  %div6 = fmul double %mul, 5.000000e-01
  %call8 = tail call double @pow(double noundef %x.031, double noundef 4.000000e+00) #2, !tbaa !5
  %call11 = tail call double @pow(double noundef %x.031, double noundef 6.000000e+00) #2, !tbaa !5
  %0 = insertelement <2 x double> <double poison, double 1.000000e+00>, double %x.031, i64 0
  %1 = insertelement <2 x double> poison, double %div, i64 0
  %2 = insertelement <2 x double> %1, double %div6, i64 1
  %3 = fsub <2 x double> %0, %2
  %4 = insertelement <2 x double> poison, double %call1, i64 0
  %5 = insertelement <2 x double> %4, double %call8, i64 1
  %6 = fdiv <2 x double> %5, <double 1.200000e+02, double 2.400000e+01>
  %7 = fadd <2 x double> %3, %6
  %8 = insertelement <2 x double> poison, double %call3, i64 0
  %9 = insertelement <2 x double> %8, double %call11, i64 1
  %10 = fdiv <2 x double> %9, <double 5.040000e+03, double 7.200000e+02>
  %11 = fadd <2 x double> %7, %10
  %12 = extractelement <2 x double> %11, i64 0
  %13 = extractelement <2 x double> %11, i64 1
  %div14 = fdiv double %12, %13
  %sub15 = fsub double %x.031, %div14
  %add16.int = add nuw nsw i32 %i.030.int, 1
  %exitcond = icmp eq i32 %add16.int, 10
  br i1 %exitcond, label %while.end, label %while.body, !llvm.loop !9

while.end:                                        ; preds = %while.body
  ret double %sub15
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @pow(double noundef, double noundef) local_unnamed_addr #1

attributes #0 = { nofree nounwind memory(write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind }

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
