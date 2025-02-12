; ModuleID = '/home/tanmay/Documents/Tools/CIRE/benchmarks/fpbench_c/RumpFromCProgram.c'
source_filename = "/home/tanmay/Documents/Tools/CIRE/benchmarks/fpbench_c/RumpFromCProgram.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local double @ex0(double noundef %a, double noundef %b) local_unnamed_addr #0 {
entry:
  %mul = fmul double %b, %b
  %mul1 = fmul double %mul, %mul
  %mul2 = fmul double %mul, %mul1
  %mul3 = fmul double %mul1, %mul1
  %mul4 = fmul double %a, %a
  %mul5 = fmul double %mul4, 1.100000e+01
  %neg = fneg double %mul2
  %0 = tail call double @llvm.fmuladd.f64(double %mul5, double %mul, double %neg)
  %1 = tail call double @llvm.fmuladd.f64(double %mul1, double -1.210000e+02, double %0)
  %sub = fadd double %1, -2.000000e+00
  %mul9 = fmul double %mul4, %sub
  %2 = tail call double @llvm.fmuladd.f64(double %mul2, double 3.337500e+02, double %mul9)
  %3 = tail call double @llvm.fmuladd.f64(double %mul3, double 5.500000e+00, double %2)
  %mul11 = fmul double %b, 2.000000e+00
  %div = fdiv double %a, %mul11
  %add = fadd double %div, %3
  ret double %add
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 20.0.0git (https://github.com/llvm/llvm-project.git 8ac924744e93258d0c490e2fa2d4518e24cb458d)"}
