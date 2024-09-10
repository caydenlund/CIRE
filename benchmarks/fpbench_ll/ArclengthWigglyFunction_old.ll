; ModuleID = 'benchmarks_c/ArclengthWigglyFunction_old.c'
source_filename = "benchmarks_c/ArclengthWigglyFunction_old.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind memory(write) uwtable
define dso_local double @ex0(double noundef %n) local_unnamed_addr #0 {
entry:
  %div = fdiv double 0x400921FB54442D18, %n
  %cmp = fcmp ult double %n, 1.000000e+00
  br i1 %cmp, label %while.end43, label %while.body

while.body:                                       ; preds = %entry, %while.body
  %s1.076 = phi double [ %conv40, %while.body ], [ 0.000000e+00, %entry ]
  %t1.075 = phi double [ %add34.4, %while.body ], [ 0.000000e+00, %entry ]
  %i.074 = phi double [ %add39, %while.body ], [ 1.000000e+00, %entry ]
  %mul = fmul double %div, %i.074
  %mul8 = fmul double %mul, 2.000000e+00
  %call9 = tail call double @sin(double noundef %mul8) #4, !tbaa !5
  %div11 = fmul double %call9, 5.000000e-01
  %add = fadd double %mul, %div11
  %mul8.1 = fmul double %mul, 4.000000e+00
  %call9.1 = tail call double @sin(double noundef %mul8.1) #4, !tbaa !5
  %div11.1 = fmul double %call9.1, 2.500000e-01
  %add.1 = fadd double %add, %div11.1
  %mul8.2 = fmul double %mul, 8.000000e+00
  %call9.2 = tail call double @sin(double noundef %mul8.2) #4, !tbaa !5
  %div11.2 = fmul double %call9.2, 1.250000e-01
  %add.2 = fadd double %add.1, %div11.2
  %mul8.3 = fmul double %mul, 1.600000e+01
  %call9.3 = tail call double @sin(double noundef %mul8.3) #4, !tbaa !5
  %div11.3 = fmul double %call9.3, 6.250000e-02
  %add.3 = fadd double %add.2, %div11.3
  %mul8.4 = fmul double %mul, 3.200000e+01
  %call9.4 = tail call double @sin(double noundef %mul8.4) #4, !tbaa !5
  %div11.4 = fmul double %call9.4, 3.125000e-02
  %add.4 = fadd double %add.3, %div11.4
  %sub = fsub double %add.4, %t1.075
  %mul17 = fmul double %sub, %sub
  %0 = tail call double @llvm.fmuladd.f64(double %div, double %div, double %mul17)
  %sqrt = tail call double @llvm.sqrt.f64(double %0)
  %conv20 = fpext double %sqrt to x86_fp80
  %call31 = tail call double @sin(double noundef %mul8) #4, !tbaa !5
  %div33 = fmul double %call31, 5.000000e-01
  %add34 = fadd double %mul, %div33
  %call31.1 = tail call double @sin(double noundef %mul8.1) #4, !tbaa !5
  %div33.1 = fmul double %call31.1, 2.500000e-01
  %add34.1 = fadd double %add34, %div33.1
  %call31.2 = tail call double @sin(double noundef %mul8.2) #4, !tbaa !5
  %div33.2 = fmul double %call31.2, 1.250000e-01
  %add34.2 = fadd double %add34.1, %div33.2
  %call31.3 = tail call double @sin(double noundef %mul8.3) #4, !tbaa !5
  %div33.3 = fmul double %call31.3, 6.250000e-02
  %add34.3 = fadd double %add34.2, %div33.3
  %call31.4 = tail call double @sin(double noundef %mul8.4) #4, !tbaa !5
  %div33.4 = fmul double %call31.4, 3.125000e-02
  %add34.4 = fadd double %add34.3, %div33.4
  %conv19 = fpext double %s1.076 to x86_fp80
  %add21 = fadd x86_fp80 %conv19, %conv20
  %add39 = fadd double %i.074, 1.000000e+00
  %conv40 = fptrunc x86_fp80 %add21 to double
  %cmp41 = fcmp ugt double %add39, %n
  br i1 %cmp41, label %while.end43, label %while.body, !llvm.loop !9

while.end43:                                      ; preds = %while.body, %entry
  %s1.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %conv40, %while.body ]
  ret double %s1.0.lcssa
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @sin(double noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.sqrt.f64(double) #3

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
