; ModuleID = 'benchmarks_c/NMSE3.4.2.c'
source_filename = "benchmarks_c/NMSE3.4.2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nounwind willreturn memory(write) uwtable
define dso_local double @ex0(double noundef %a, double noundef %b, double noundef %eps) local_unnamed_addr #0 {
entry:
  %add = fadd double %a, %b
  %mul = fmul double %add, %eps
  %call = tail call double @exp(double noundef %mul) #2, !tbaa !5
  %mul2 = fmul double %a, %eps
  %call3 = tail call double @exp(double noundef %mul2) #2, !tbaa !5
  %mul5 = fmul double %b, %eps
  %call6 = tail call double @exp(double noundef %mul5) #2, !tbaa !5
  %sub7 = fadd double %call6, -1.000000e+00
  %0 = insertelement <2 x double> poison, double %call, i64 0
  %1 = insertelement <2 x double> %0, double %call3, i64 1
  %2 = fadd <2 x double> %1, <double -1.000000e+00, double -1.000000e+00>
  %3 = insertelement <2 x double> poison, double %eps, i64 0
  %4 = insertelement <2 x double> %3, double %sub7, i64 1
  %5 = fmul <2 x double> %2, %4
  %6 = extractelement <2 x double> %5, i64 0
  %7 = extractelement <2 x double> %5, i64 1
  %div = fdiv double %6, %7
  ret double %div
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @exp(double noundef) local_unnamed_addr #1

attributes #0 = { mustprogress nofree nounwind willreturn memory(write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind }

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
