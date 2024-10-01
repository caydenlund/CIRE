; ModuleID = 'benchmarks_c/ArclengthWigglyFunction.c'
source_filename = "benchmarks_c/ArclengthWigglyFunction.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind memory(write) uwtable
define dso_local double @ex0(i64 noundef %n) local_unnamed_addr #0 {
entry:
  %conv = sitofp i64 %n to double
  %div = fdiv double 0x400921FB54442D18, %conv
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %while.body, label %while.end29

while.body:                                       ; preds = %entry, %while.body
  %s1.052 = phi double [ %conv25, %while.body ], [ 0.000000e+00, %entry ]
  %t1_1.051 = phi double [ %add.4, %while.body ], [ 0.000000e+00, %entry ]
  %i.050 = phi i64 [ %add26, %while.body ], [ 1, %entry ]
  %conv2 = uitofp nneg i64 %i.050 to double
  %mul = fmul double %div, %conv2
  %mul11 = fmul double %mul, 2.000000e+00
  %call = tail call double @sin(double noundef %mul11) #4, !tbaa !5
  %div13 = fmul double %call, 5.000000e-01
  %add = fadd double %mul, %div13
  %mul11.1 = fmul double %mul, 4.000000e+00
  %call.1 = tail call double @sin(double noundef %mul11.1) #4, !tbaa !5
  %div13.1 = fmul double %call.1, 2.500000e-01
  %add.1 = fadd double %add, %div13.1
  %mul11.2 = fmul double %mul, 8.000000e+00
  %call.2 = tail call double @sin(double noundef %mul11.2) #4, !tbaa !5
  %div13.2 = fmul double %call.2, 1.250000e-01
  %add.2 = fadd double %add.1, %div13.2
  %mul11.3 = fmul double %mul, 1.600000e+01
  %call.3 = tail call double @sin(double noundef %mul11.3) #4, !tbaa !5
  %div13.3 = fmul double %call.3, 6.250000e-02
  %add.3 = fadd double %add.2, %div13.3
  %mul11.4 = fmul double %mul, 3.200000e+01
  %call.4 = tail call double @sin(double noundef %mul11.4) #4, !tbaa !5
  %div13.4 = fmul double %call.4, 3.125000e-02
  %add.4 = fadd double %add.3, %div13.4
  %sub = fsub double %add.4, %t1_1.051
  %mul20 = fmul double %sub, %sub
  %0 = tail call double @llvm.fmuladd.f64(double %div, double %div, double %mul20)
  %sqrt = tail call double @llvm.sqrt.f64(double %0)
  %conv22 = fpext double %s1.052 to x86_fp80
  %conv23 = fpext double %sqrt to x86_fp80
  %add24 = fadd x86_fp80 %conv22, %conv23
  %conv25 = fptrunc x86_fp80 %add24 to double
  %add26 = add nuw i64 %i.050, 1
  %exitcond.not = icmp eq i64 %i.050, %n
  br i1 %exitcond.not, label %while.end29, label %while.body, !llvm.loop !9

while.end29:                                      ; preds = %while.body, %entry
  %s1.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %conv25, %while.body ]
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
