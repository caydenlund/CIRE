; ModuleID = 'benchmarks_c/RocketTrajectory.c'
source_filename = "benchmarks_c/RocketTrajectory.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind memory(write) uwtable
define dso_local float @ex0(float noundef %Mf, float noundef %A) local_unnamed_addr #0 {
entry:
  br label %while.body

while.body:                                       ; preds = %entry, %cdce.end
  %i.0126.int = phi i32 [ 1, %entry ], [ %add61.int, %cdce.end ]
  %0 = phi <4 x float> [ <float 0.000000e+00, float 0.000000e+00, float 0x3F43822240000000, float 1.040000e+07>, %entry ], [ %8, %cdce.end ]
  %1 = extractelement <4 x float> %0, i64 3
  %mul14 = fmul float %1, %1
  %div15 = fdiv float 0x4513C3D600000000, %mul14
  %mul16 = fmul float %div15, 0xBDD2589B60000000
  %2 = extractelement <4 x float> %0, i64 2
  %mul18 = fmul float %1, %2
  %mul19 = fmul float %2, 0x3FB99999A0000000
  %mul20 = fmul float %mul18, %mul19
  %div21 = fdiv float %2, %1
  %3 = extractelement <4 x float> %0, i64 1
  %mul22 = fmul float %3, %div21
  %mul23 = fmul float %mul22, -2.000000e+00
  %4 = shufflevector <4 x float> %0, <4 x float> poison, <4 x i32> <i32 2, i32 poison, i32 poison, i32 1>
  %5 = insertelement <4 x float> %4, float %mul16, i64 1
  %6 = insertelement <4 x float> %5, float %mul23, i64 2
  %7 = insertelement <4 x float> %0, float %mul20, i64 1
  %8 = tail call <4 x float> @llvm.fmuladd.v4f32(<4 x float> %6, <4 x float> <float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000>, <4 x float> %7)
  %9 = extractelement <4 x float> %8, i64 0
  %call57 = tail call float @cosf(float noundef %9) #3, !tbaa !5
  %10 = tail call float @llvm.fabs.f32(float %9)
  %11 = fcmp oeq float %10, 0x7FF0000000000000
  br i1 %11, label %cdce.call, label %cdce.end, !prof !9

cdce.call:                                        ; preds = %while.body
  %call59 = tail call float @sinf(float noundef %9) #3, !tbaa !5
  br label %cdce.end

cdce.end:                                         ; preds = %while.body, %cdce.call
  %add61.int = add nuw nsw i32 %i.0126.int, 1
  %exitcond = icmp eq i32 %add61.int, 2000000
  br i1 %exitcond, label %while.end, label %while.body, !llvm.loop !10

while.end:                                        ; preds = %cdce.end
  %12 = extractelement <4 x float> %8, i64 3
  %mul58.le = fmul float %12, %call57
  ret float %mul58.le
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @cosf(float noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare float @sinf(float noundef) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <4 x float> @llvm.fmuladd.v4f32(<4 x float>, <4 x float>, <4 x float>) #2

attributes #0 = { nofree nounwind memory(write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
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
!9 = !{!"branch_weights", i32 1, i32 1048575}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.mustprogress"}
