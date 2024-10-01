; ModuleID = 'benchmarks_c/NMSE3.6.c'
source_filename = "benchmarks_c/NMSE3.6.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nounwind willreturn memory(write) uwtable
define dso_local double @ex0(double noundef %x) local_unnamed_addr #0 {
entry:
  %call = tail call double @sqrt(double noundef %x) #2, !tbaa !5
  %add = fadd double %x, 1.000000e+00
  %call1 = tail call double @sqrt(double noundef %add) #2, !tbaa !5
  %0 = insertelement <2 x double> poison, double %call, i64 0
  %1 = insertelement <2 x double> %0, double %call1, i64 1
  %2 = fdiv <2 x double> <double 1.000000e+00, double 1.000000e+00>, %1
  %shift = shufflevector <2 x double> %2, <2 x double> poison, <2 x i32> <i32 1, i32 poison>
  %3 = fsub <2 x double> %2, %shift
  %sub = extractelement <2 x double> %3, i64 0
  ret double %sub
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @sqrt(double noundef) local_unnamed_addr #1

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
