; ModuleID = 'benchmarks_c/JacobisMethod.c'
source_filename = "benchmarks_c/JacobisMethod.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local float @ex0(float noundef %a11, float noundef %a22, float noundef %a33, float noundef %a44, float noundef %b1, float noundef %b2, float noundef %b3, float noundef %b4) local_unnamed_addr #0 {
entry:
  %0 = insertelement <4 x float> poison, float %b4, i64 0
  %1 = insertelement <4 x float> %0, float %b3, i64 1
  %2 = insertelement <4 x float> %1, float %b2, i64 2
  %3 = insertelement <4 x float> %2, float %b1, i64 3
  %4 = insertelement <4 x float> poison, float %a44, i64 0
  %5 = insertelement <4 x float> %4, float %a33, i64 1
  %6 = insertelement <4 x float> %5, float %a22, i64 2
  %7 = insertelement <4 x float> %6, float %a11, i64 3
  %8 = fdiv <4 x float> %3, %7
  %9 = fdiv <4 x float> <float 0x3FB99999A0000000, float 0xBFC99999A0000000, float 0xBFD3333340000000, float 0xBFB99999A0000000>, %7
  %10 = fdiv <4 x float> <float 0xBFC99999A0000000, float 0x3FD3333340000000, float 0x3FB99999A0000000, float 0xBFC99999A0000000>, %7
  %11 = fdiv <4 x float> <float 0xBFD3333340000000, float 0xBFB99999A0000000, float 0xBFC99999A0000000, float 0x3FD3333340000000>, %7
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %12 = phi <4 x float> [ zeroinitializer, %entry ], [ %18, %while.body ]
  %13 = shufflevector <4 x float> %12, <4 x float> poison, <4 x i32> <i32 3, i32 3, i32 3, i32 2>
  %14 = tail call <4 x float> @llvm.fmuladd.v4f32(<4 x float> %9, <4 x float> %13, <4 x float> %8)
  %15 = shufflevector <4 x float> %12, <4 x float> poison, <4 x i32> <i32 2, i32 2, i32 1, i32 1>
  %16 = tail call <4 x float> @llvm.fmuladd.v4f32(<4 x float> %10, <4 x float> %15, <4 x float> %14)
  %17 = shufflevector <4 x float> %12, <4 x float> poison, <4 x i32> <i32 1, i32 0, i32 0, i32 0>
  %18 = tail call <4 x float> @llvm.fmuladd.v4f32(<4 x float> %11, <4 x float> %17, <4 x float> %16)
  %19 = fsub <4 x float> %18, %12
  %sub = extractelement <4 x float> %19, i64 0
  %20 = tail call float @llvm.fabs.f32(float %sub)
  %cmp23 = fcmp ule float %20, 0x3C670EF540000000
  br i1 %cmp23, label %while.end, label %while.body, !llvm.loop !5

while.end:                                        ; preds = %while.body
  %21 = extractelement <4 x float> %18, i64 2
  ret float %21
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <4 x float> @llvm.fmuladd.v4f32(<4 x float>, <4 x float>, <4 x float>) #2

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
