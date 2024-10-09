; ModuleID = 'benchmarks_c/Newton-Raphsons.c'
source_filename = "benchmarks_c/Newton-Raphsons.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local float @ex0(float noundef %x0) local_unnamed_addr #0 {
entry:
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %i.068 = phi float [ 0.000000e+00, %entry ], [ %add29, %while.body ]
  %x.067 = phi float [ 0.000000e+00, %entry ], [ %sub27, %while.body ]
  %0 = fmul float %x.067, -1.000000e+01
  %1 = insertelement <2 x float> poison, float %x.067, i64 0
  %2 = shufflevector <2 x float> %1, <2 x float> poison, <2 x i32> zeroinitializer
  %3 = fmul <2 x float> %2, <float 4.000000e+01, float 5.000000e+00>
  %4 = fmul <2 x float> %2, <float -8.000000e+01, float 1.200000e+02>
  %5 = extractelement <2 x float> %3, i64 0
  %6 = fneg float %5
  %mul = fmul float %x.067, %x.067
  %mul3 = fmul float %x.067, %mul
  %neg = fmul float %0, %mul3
  %7 = tail call float @llvm.fmuladd.f32(float %mul, float %mul3, float %neg)
  %neg23 = fmul float %mul, %6
  %8 = insertelement <2 x float> poison, float %mul, i64 0
  %9 = insertelement <2 x float> %8, float %mul3, i64 1
  %10 = insertelement <2 x float> poison, float %7, i64 0
  %11 = insertelement <2 x float> %10, float %neg23, i64 1
  %12 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %3, <2 x float> %9, <2 x float> %11)
  %13 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %4, <2 x float> %2, <2 x float> %12)
  %14 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %2, <2 x float> <float 8.000000e+01, float -1.600000e+02>, <2 x float> %13)
  %15 = fadd <2 x float> %14, <float -3.200000e+01, float 8.000000e+01>
  %16 = extractelement <2 x float> %15, i64 0
  %17 = extractelement <2 x float> %15, i64 1
  %div = fdiv float %16, %17
  %sub27 = fsub float %x.067, %div
  %sub28 = fsub float %x.067, %sub27
  %18 = tail call float @llvm.fabs.f32(float %sub28)
  %add29 = fadd float %i.068, 1.000000e+00
  %cmp30 = fcmp ule float %18, 0x3F40624DE0000000
  %cmp32 = fcmp uge float %add29, 1.000000e+05
  %.not = select i1 %cmp30, i1 true, i1 %cmp32
  br i1 %.not, label %while.end, label %while.body, !llvm.loop !5

while.end:                                        ; preds = %while.body
  ret float %sub27
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x float> @llvm.fmuladd.v2f32(<2 x float>, <2 x float>, <2 x float>) #2

attributes #0 = { nofree norecurse nosync nounwind memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 20.0.0git (https://github.com/llvm/llvm-project.git 8ac924744e93258d0c490e2fa2d4518e24cb458d)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
