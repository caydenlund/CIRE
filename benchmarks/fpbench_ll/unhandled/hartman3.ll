; ModuleID = 'benchmarks_c/hartman3.c'
source_filename = "benchmarks_c/hartman3.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nounwind willreturn memory(write) uwtable
define dso_local double @ex0(double noundef %x1, double noundef %x2, double noundef %x3) local_unnamed_addr #0 {
entry:
  %sub = fadd double %x1, -3.689000e-01
  %mul = fmul double %sub, %sub
  %sub3 = fadd double %x2, -1.170000e-01
  %mul5 = fmul double %sub3, %sub3
  %mul6 = fmul double %mul5, 1.000000e+01
  %0 = tail call double @llvm.fmuladd.f64(double %mul, double 3.000000e+00, double %mul6)
  %sub7 = fadd double %x3, -2.673000e-01
  %mul9 = fmul double %sub7, %sub7
  %1 = tail call double @llvm.fmuladd.f64(double %mul9, double 3.000000e+01, double %0)
  %sub11 = fadd double %x1, -4.699000e-01
  %mul13 = fmul double %sub11, %sub11
  %sub15 = fadd double %x2, -4.387000e-01
  %mul17 = fmul double %sub15, %sub15
  %mul18 = fmul double %mul17, 1.000000e+01
  %2 = tail call double @llvm.fmuladd.f64(double %mul13, double 1.000000e-01, double %mul18)
  %sub19 = fadd double %x3, -7.470000e-01
  %mul21 = fmul double %sub19, %sub19
  %3 = tail call double @llvm.fmuladd.f64(double %mul21, double 3.500000e+01, double %2)
  %sub23 = fadd double %x1, -1.091000e-01
  %mul25 = fmul double %sub23, %sub23
  %sub27 = fadd double %x2, 0xBFEBF141205BC01A
  %mul29 = fmul double %sub27, %sub27
  %mul30 = fmul double %mul29, 1.000000e+01
  %4 = tail call double @llvm.fmuladd.f64(double %mul25, double 3.000000e+00, double %mul30)
  %sub31 = fadd double %x3, -5.547000e-01
  %mul33 = fmul double %sub31, %sub31
  %5 = tail call double @llvm.fmuladd.f64(double %mul33, double 3.000000e+01, double %4)
  %sub35 = fadd double %x1, -3.815000e-02
  %mul37 = fmul double %sub35, %sub35
  %sub39 = fadd double %x2, -5.743000e-01
  %mul41 = fmul double %sub39, %sub39
  %mul42 = fmul double %mul41, 1.000000e+01
  %6 = tail call double @llvm.fmuladd.f64(double %mul37, double 1.000000e-01, double %mul42)
  %sub43 = fadd double %x3, -8.828000e-01
  %mul45 = fmul double %sub43, %sub43
  %7 = tail call double @llvm.fmuladd.f64(double %mul45, double 3.500000e+01, double %6)
  %fneg = fneg double %1
  %call = tail call double @exp(double noundef %fneg) #3, !tbaa !5
  %fneg47 = fneg double %3
  %call48 = tail call double @exp(double noundef %fneg47) #3, !tbaa !5
  %fneg49 = fneg double %5
  %call50 = tail call double @exp(double noundef %fneg49) #3, !tbaa !5
  %fneg51 = fneg double %7
  %call52 = tail call double @exp(double noundef %fneg51) #3, !tbaa !5
  %mul54 = fmul double %call48, 1.200000e+00
  %8 = fadd double %call, %mul54
  %9 = tail call double @llvm.fmuladd.f64(double %call50, double 3.000000e+00, double %8)
  %10 = tail call double @llvm.fmuladd.f64(double %call52, double 3.200000e+00, double %9)
  %fneg57 = fneg double %10
  ret double %fneg57
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @exp(double noundef) local_unnamed_addr #2

attributes #0 = { mustprogress nofree nounwind willreturn memory(write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
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
