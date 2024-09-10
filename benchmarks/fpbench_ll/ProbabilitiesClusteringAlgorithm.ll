; ModuleID = 'benchmarks_c/ProbabilitiesClusteringAlgorithm.c'
source_filename = "benchmarks_c/ProbabilitiesClusteringAlgorithm.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nounwind willreturn memory(write) uwtable
define dso_local double @ex0(double noundef %cp, double noundef %cn, double noundef %t, double noundef %s) local_unnamed_addr #0 {
entry:
  %fneg = fneg double %s
  %call = tail call double @exp(double noundef %fneg) #2, !tbaa !5
  %add = fadd double %call, 1.000000e+00
  %div = fdiv double 1.000000e+00, %add
  %call1 = tail call double @pow(double noundef %div, double noundef %cp) #2, !tbaa !5
  %call3 = tail call double @exp(double noundef %fneg) #2, !tbaa !5
  %add4 = fadd double %call3, 1.000000e+00
  %div5 = fdiv double 1.000000e+00, %add4
  %sub = fsub double 1.000000e+00, %div5
  %call6 = tail call double @pow(double noundef %sub, double noundef %cn) #2, !tbaa !5
  %mul = fmul double %call1, %call6
  %fneg7 = fneg double %t
  %call8 = tail call double @exp(double noundef %fneg7) #2, !tbaa !5
  %add9 = fadd double %call8, 1.000000e+00
  %div10 = fdiv double 1.000000e+00, %add9
  %call11 = tail call double @pow(double noundef %div10, double noundef %cp) #2, !tbaa !5
  %call13 = tail call double @exp(double noundef %fneg7) #2, !tbaa !5
  %add14 = fadd double %call13, 1.000000e+00
  %div15 = fdiv double 1.000000e+00, %add14
  %sub16 = fsub double 1.000000e+00, %div15
  %call17 = tail call double @pow(double noundef %sub16, double noundef %cn) #2, !tbaa !5
  %mul18 = fmul double %call11, %call17
  %div19 = fdiv double %mul, %mul18
  ret double %div19
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @pow(double noundef, double noundef) local_unnamed_addr #1

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
