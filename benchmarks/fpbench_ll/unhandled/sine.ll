; ModuleID = 'benchmarks_c/sine.c'
source_filename = "benchmarks_c/sine.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local double @ex0(double noundef %x) local_unnamed_addr #0 {
entry:
  %mul = fmul double %x, %x
  %mul1 = fmul double %mul, %x
  %div = fdiv double %mul1, 6.000000e+00
  %sub = fsub double %x, %div
  %mul4 = fmul double %mul1, %x
  %mul5 = fmul double %mul4, %x
  %mul11 = fmul double %mul5, %x
  %mul12 = fmul double %mul11, %x
  %0 = insertelement <2 x double> poison, double %mul5, i64 0
  %1 = insertelement <2 x double> %0, double %mul12, i64 1
  %2 = fdiv <2 x double> %1, <double 1.200000e+02, double 5.040000e+03>
  %3 = extractelement <2 x double> %2, i64 0
  %add = fadd double %sub, %3
  %4 = extractelement <2 x double> %2, i64 1
  %sub14 = fsub double %add, %4
  ret double %sub14
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 20.0.0git (https://github.com/llvm/llvm-project.git 8ac924744e93258d0c490e2fa2d4518e24cb458d)"}
