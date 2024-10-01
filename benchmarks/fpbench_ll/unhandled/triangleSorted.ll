; ModuleID = 'benchmarks_c/triangleSorted.c'
source_filename = "benchmarks_c/triangleSorted.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nounwind willreturn memory(write) uwtable
define dso_local double @ex0(double noundef %a, double noundef %b, double noundef %c) local_unnamed_addr #0 {
entry:
  %cmp = fcmp olt double %a, %b
  %add = fadd double %a, %b
  %add1 = fadd double %add, %c
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %sub = fsub double %c, %b
  %sub2 = fsub double %a, %sub
  %mul = fmul double %add1, %sub2
  %add4 = fadd double %sub, %a
  %mul5 = fmul double %add4, %mul
  %sub6 = fsub double %b, %a
  br label %if.end

if.else:                                          ; preds = %entry
  %sub11 = fsub double %c, %a
  %sub12 = fsub double %b, %sub11
  %mul13 = fmul double %add1, %sub12
  %add15 = fadd double %sub11, %b
  %mul16 = fmul double %add15, %mul13
  %sub17 = fsub double %a, %b
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %sub17.sink = phi double [ %sub17, %if.else ], [ %sub6, %if.then ]
  %mul16.sink = phi double [ %mul16, %if.else ], [ %mul5, %if.then ]
  %add18 = fadd double %sub17.sink, %c
  %mul19 = fmul double %add18, %mul16.sink
  %call20 = tail call double @sqrt(double noundef %mul19) #2, !tbaa !5
  %tmp.0 = fmul double %call20, 2.500000e-01
  ret double %tmp.0
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
