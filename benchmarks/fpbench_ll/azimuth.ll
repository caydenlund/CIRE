; ModuleID = 'benchmarks_c/azimuth.c'
source_filename = "benchmarks_c/azimuth.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nounwind willreturn memory(write) uwtable
define dso_local double @ex0(double noundef %lat1, double noundef %lat2, double noundef %lon1, double noundef %lon2) local_unnamed_addr #0 {
entry:
  %sub = fsub double %lon2, %lon1
  %call = tail call double @sin(double noundef %lat1) #3, !tbaa !5
  %call1 = tail call double @cos(double noundef %lat1) #3, !tbaa !5
  %call2 = tail call double @sin(double noundef %lat2) #3, !tbaa !5
  %call3 = tail call double @cos(double noundef %lat2) #3, !tbaa !5
  %call4 = tail call double @sin(double noundef %sub) #3, !tbaa !5
  %call5 = tail call double @cos(double noundef %sub) #3, !tbaa !5
  %mul = fmul double %call3, %call4
  %0 = fneg double %call
  %1 = fmul double %call3, %0
  %neg = fmul double %1, %call5
  %2 = tail call double @llvm.fmuladd.f64(double %call1, double %call2, double %neg)
  %div = fdiv double %mul, %2
  %call9 = tail call double @atan(double noundef %div) #3, !tbaa !5
  ret double %call9
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @sin(double noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @cos(double noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @atan(double noundef) local_unnamed_addr #1

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
