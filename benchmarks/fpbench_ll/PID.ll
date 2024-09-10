; ModuleID = 'benchmarks_c/PID.c'
source_filename = "benchmarks_c/PID.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local double @ex0(double noundef %m, double noundef %kp, double noundef %ki, double noundef %kd, double noundef %c) local_unnamed_addr #0 {
entry:
  %mul1 = fmul double %ki, 5.000000e-01
  %mul3 = fmul double %kd, 2.000000e+00
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %t.024 = phi double [ 0.000000e+00, %entry ], [ %add8, %while.body ]
  %eold.023 = phi double [ 0.000000e+00, %entry ], [ %sub, %while.body ]
  %m_1.022 = phi double [ %m, %entry ], [ %1, %while.body ]
  %i.021 = phi double [ 0.000000e+00, %entry ], [ %0, %while.body ]
  %sub = fsub double %c, %m_1.022
  %mul = fmul double %sub, %kp
  %0 = tail call double @llvm.fmuladd.f64(double %mul1, double %sub, double %i.021)
  %sub4 = fsub double %sub, %eold.023
  %mul5 = fmul double %mul3, %sub4
  %add = fadd double %mul, %0
  %add6 = fadd double %add, %mul5
  %1 = tail call double @llvm.fmuladd.f64(double %add6, double 1.000000e-02, double %m_1.022)
  %add8 = fadd double %t.024, 5.000000e-01
  %cmp9 = fcmp uge double %add8, 1.000000e+02
  br i1 %cmp9, label %while.end, label %while.body, !llvm.loop !5

while.end:                                        ; preds = %while.body
  ret double %1
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

attributes #0 = { nofree norecurse nosync nounwind memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 20.0.0git (https://github.com/llvm/llvm-project.git 8ac924744e93258d0c490e2fa2d4518e24cb458d)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
