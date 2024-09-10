; ModuleID = 'benchmarks_c/Gustafsons.c'
source_filename = "benchmarks_c/Gustafsons.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nounwind willreturn memory(write) uwtable
define dso_local double @ex0(double noundef %x) local_unnamed_addr #0 {
entry:
  %0 = tail call double @llvm.fmuladd.f64(double %x, double %x, double 1.000000e+00)
  %sqrt14 = tail call double @llvm.sqrt.f64(double %0)
  %sub = fsub double %x, %sqrt14
  %1 = tail call double @llvm.fabs.f64(double %sub)
  %add = fadd double %sqrt14, %x
  %div = fdiv double 1.000000e+00, %add
  %sub2 = fsub double %1, %div
  %mul = fmul double %sub2, %sub2
  %cmp = fcmp oeq double %mul, 0.000000e+00
  br i1 %cmp, label %if.end, label %if.else

if.else:                                          ; preds = %entry
  %call3 = tail call double @exp(double noundef %mul) #4, !tbaa !5
  %sub4 = fadd double %call3, -1.000000e+00
  %div5 = fdiv double %sub4, %x
  br label %if.end

if.end:                                           ; preds = %entry, %if.else
  %tmp.0 = phi double [ %div5, %if.else ], [ 1.000000e+00, %entry ]
  ret double %tmp.0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fabs.f64(double) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @exp(double noundef) local_unnamed_addr #2

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
