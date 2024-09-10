; ModuleID = 'benchmarks_c/test01_sum3.c'
source_filename = "benchmarks_c/test01_sum3.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef float @ex0(float noundef %x0, float noundef %x1, float noundef %x2) local_unnamed_addr #0 {
entry:
  %add = fadd float %x0, %x1
  %sub = fsub float %add, %x2
  %add1 = fadd float %x1, %x2
  %sub2 = fsub float %add1, %x0
  %add3 = fadd float %x0, %x2
  %sub4 = fsub float %add3, %x1
  %add5 = fadd float %sub, %sub2
  %add6 = fadd float %sub4, %add5
  ret float %add6
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 20.0.0git (https://github.com/llvm/llvm-project.git 8ac924744e93258d0c490e2fa2d4518e24cb458d)"}
