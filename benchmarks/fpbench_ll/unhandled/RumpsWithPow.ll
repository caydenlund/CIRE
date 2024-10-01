; ModuleID = 'benchmarks_c/RumpsWithPow.c'
source_filename = "benchmarks_c/RumpsWithPow.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nounwind willreturn memory(write) uwtable
define dso_local double @ex0(double noundef %a, double noundef %b) local_unnamed_addr #0 {
entry:
  %call = tail call double @pow(double noundef %b, double noundef 6.000000e+00) #3, !tbaa !5
  %square = fmul double %a, %a
  %mul3 = fmul double %square, 1.100000e+01
  %square19 = fmul double %b, %b
  %call6 = tail call double @pow(double noundef %b, double noundef 6.000000e+00) #3, !tbaa !5
  %neg = fneg double %call6
  %0 = tail call double @llvm.fmuladd.f64(double %mul3, double %square19, double %neg)
  %call7 = tail call double @pow(double noundef %b, double noundef 4.000000e+00) #3, !tbaa !5
  %1 = tail call double @llvm.fmuladd.f64(double %call7, double -1.210000e+02, double %0)
  %sub = fadd double %1, -2.000000e+00
  %mul9 = fmul double %square, %sub
  %2 = tail call double @llvm.fmuladd.f64(double %call, double 3.337500e+02, double %mul9)
  %call10 = tail call double @pow(double noundef %b, double noundef 8.000000e+00) #3, !tbaa !5
  %3 = tail call double @llvm.fmuladd.f64(double %call10, double 5.500000e+00, double %2)
  %mul = fmul double %b, 2.000000e+00
  %div = fdiv double %a, %mul
  %add = fadd double %div, %3
  ret double %add
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @pow(double noundef, double noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #2

attributes #0 = { mustprogress nofree nounwind willreturn memory(write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nounwind }

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
