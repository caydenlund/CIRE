; ModuleID = '/home/tanmay/Documents/Tools/CIRE/benchmarks/fpbench_c/kepler1.c'
source_filename = "/home/tanmay/Documents/Tools/CIRE/benchmarks/fpbench_c/kepler1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local double @ex0(double noundef %x1, double noundef %x2, double noundef %x3, double noundef %x4) local_unnamed_addr #0 {
entry:
  %mul = fmul double %x1, %x4
  %fneg = fneg double %x1
  %add = fsub double %x2, %x1
  %add1 = fadd double %add, %x3
  %sub = fsub double %add1, %x4
  %sub3 = fsub double %x1, %x2
  %add4 = fadd double %sub3, %x3
  %add5 = fadd double %add4, %x4
  %mul6 = fmul double %add5, %x2
  %0 = tail call double @llvm.fmuladd.f64(double %mul, double %sub, double %mul6)
  %add7 = fadd double %x1, %x2
  %sub8 = fsub double %add7, %x3
  %add9 = fadd double %sub8, %x4
  %1 = tail call double @llvm.fmuladd.f64(double %x3, double %add9, double %0)
  %2 = fneg double %x2
  %neg = fmul double %2, %x3
  %3 = tail call double @llvm.fmuladd.f64(double %neg, double %x4, double %1)
  %4 = tail call double @llvm.fmuladd.f64(double %fneg, double %x3, double %3)
  %5 = tail call double @llvm.fmuladd.f64(double %fneg, double %x2, double %4)
  %sub17 = fsub double %5, %x4
  ret double %sub17
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
