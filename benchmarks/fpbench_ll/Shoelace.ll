; ModuleID = '/home/tanmay/Documents/Tools/CIRE/benchmarks/fpbench_c/Shoelace.c'
source_filename = "/home/tanmay/Documents/Tools/CIRE/benchmarks/fpbench_c/Shoelace.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local double @ex0(double noundef %x1, double noundef %y1, double noundef %x2, double noundef %y2, double noundef %x3, double noundef %y3) local_unnamed_addr #0 {
entry:
  %0 = fneg double %y1
  %neg = fmul double %0, %x2
  %1 = tail call double @llvm.fmuladd.f64(double %x1, double %y2, double %neg)
  %2 = fneg double %y2
  %neg3 = fmul double %2, %x3
  %3 = tail call double @llvm.fmuladd.f64(double %x2, double %y3, double %neg3)
  %4 = fneg double %y3
  %neg5 = fmul double %4, %x1
  %5 = tail call double @llvm.fmuladd.f64(double %x3, double %y1, double %neg5)
  %add = fadd double %1, %3
  %add6 = fadd double %add, %5
  %mul = fmul double %add6, 5.000000e-01
  ret double %mul
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
