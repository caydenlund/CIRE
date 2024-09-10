; ModuleID = 'benchmarks_c/NBodySimulation.c'
source_filename = "benchmarks_c/NBodySimulation.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local double @ex0(double noundef %x0, double noundef %y0, double noundef %z0, double noundef %vx0, double noundef %vy0, double noundef %vz0) local_unnamed_addr #0 {
entry:
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %x.0144 = phi double [ %x0, %entry ], [ %3, %while.body ]
  %y.0143 = phi double [ %y0, %entry ], [ %5, %while.body ]
  %z.0142 = phi double [ %z0, %entry ], [ %7, %while.body ]
  %vx.0141 = phi double [ %vx0, %entry ], [ %2, %while.body ]
  %vy.0140 = phi double [ %vy0, %entry ], [ %4, %while.body ]
  %vz.0139 = phi double [ %vz0, %entry ], [ %6, %while.body ]
  %i.0138.int = phi i32 [ 0, %entry ], [ %add.int, %while.body ]
  %mul1 = fmul double %y.0143, %y.0143
  %0 = tail call double @llvm.fmuladd.f64(double %x.0144, double %x.0144, double %mul1)
  %1 = tail call double @llvm.fmuladd.f64(double %z.0142, double %z.0142, double %0)
  %sqrt137 = tail call double @llvm.sqrt.f64(double %1)
  %mul = fmul double %sqrt137, %sqrt137
  %mul2 = fmul double %sqrt137, %mul
  %div = fdiv double 1.000000e-01, %mul2
  %neg = fmul double %x.0144, 0xC043BD3CC9BE45DE
  %2 = tail call double @llvm.fmuladd.f64(double %neg, double %div, double %vx.0141)
  %3 = tail call double @llvm.fmuladd.f64(double %2, double 1.000000e-01, double %x.0144)
  %neg15 = fmul double %y.0143, 0xC043BD3CC9BE45DE
  %4 = tail call double @llvm.fmuladd.f64(double %neg15, double %div, double %vy.0140)
  %5 = tail call double @llvm.fmuladd.f64(double %4, double 1.000000e-01, double %y.0143)
  %neg26 = fmul double %z.0142, 0xC043BD3CC9BE45DE
  %6 = tail call double @llvm.fmuladd.f64(double %neg26, double %div, double %vz.0139)
  %7 = tail call double @llvm.fmuladd.f64(double %6, double 1.000000e-01, double %z.0142)
  %add.int = add nuw nsw i32 %i.0138.int, 1
  %exitcond = icmp eq i32 %add.int, 100
  br i1 %exitcond, label %while.end, label %while.body, !llvm.loop !5

while.end:                                        ; preds = %while.body
  ret double %3
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.sqrt.f64(double) #2

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
