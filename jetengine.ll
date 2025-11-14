; ModuleID = 'jetengine-all-double.c'
source_filename = "jetengine-all-double.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local double @jetengine(double noundef %x1, double noundef %x2) local_unnamed_addr #0 {
entry:
  %mul = fmul double %x1, %x1
  %mul1 = fmul double %x1, %mul
  %mul3 = fmul double %x2, 2.000000e+00
  %0 = tail call double @llvm.fmuladd.f64(double %mul, double 3.000000e+00, double %mul3)
  %sub = fsub double %0, %x1
  %add = fadd double %mul, 1.000000e+00
  %div = fdiv double %sub, %add
  %mul4 = fmul double %x1, 2.000000e+00
  %mul5 = fmul double %mul4, %div
  %sub6 = fadd double %div, -3.000000e+00
  %mul7 = fmul double %mul5, %sub6
  %1 = tail call double @llvm.fmuladd.f64(double %div, double 4.000000e+00, double -6.000000e+00)
  %mul9 = fmul double %mul, %1
  %add10 = fadd double %mul7, %mul9
  %mul12 = fmul double %add, %add10
  %mul13 = fmul double %mul, 3.000000e+00
  %2 = tail call double @llvm.fmuladd.f64(double %mul13, double %div, double %mul12)
  %add15 = fadd double %mul1, %2
  %add16 = fadd double %x1, %add15
  %mul17 = fmul double %div, 3.000000e+00
  %add18 = fadd double %mul17, %add16
  %add19 = fadd double %x1, %add18
  ret double %add19
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
!4 = !{!"clang version 21.0.0git (https://github.com/Groverkss/llvm-project.git 757554ee26aff867290162100b2cc9941a206e17)"}
