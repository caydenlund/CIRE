; ModuleID = 'benchmarks_c/EigenvalueComputation.c'
source_filename = "benchmarks_c/EigenvalueComputation.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local float @ex0(float noundef %a11, float noundef %a12, float noundef %a13, float noundef %a14, float noundef %a21, float noundef %a22, float noundef %a23, float noundef %a24, float noundef %a31, float noundef %a32, float noundef %a33, float noundef %a34, float noundef %a41, float noundef %a42, float noundef %a43, float noundef %a44, float noundef %v1, float noundef %v2, float noundef %v3, float noundef %v4) local_unnamed_addr #0 {
entry:
  %0 = insertelement <2 x float> poison, float %v2, i64 0
  %1 = shufflevector <2 x float> %0, <2 x float> poison, <2 x i32> zeroinitializer
  %2 = insertelement <2 x float> poison, float %a12, i64 0
  %3 = insertelement <2 x float> %2, float %a42, i64 1
  %4 = fmul <2 x float> %1, %3
  %5 = insertelement <2 x float> poison, float %v4, i64 0
  %6 = shufflevector <2 x float> %5, <2 x float> poison, <2 x i32> zeroinitializer
  %7 = insertelement <2 x float> poison, float %a14, i64 0
  %8 = insertelement <2 x float> %7, float %a44, i64 1
  %9 = fmul <2 x float> %6, %8
  %10 = insertelement <2 x float> poison, float %a11, i64 0
  %11 = insertelement <2 x float> %10, float %a41, i64 1
  %12 = insertelement <2 x float> poison, float %v1, i64 0
  %13 = shufflevector <2 x float> %12, <2 x float> poison, <2 x i32> zeroinitializer
  %14 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %11, <2 x float> %13, <2 x float> %4)
  %15 = insertelement <2 x float> poison, float %a13, i64 0
  %16 = insertelement <2 x float> %15, float %a43, i64 1
  %17 = insertelement <2 x float> poison, float %v3, i64 0
  %18 = shufflevector <2 x float> %17, <2 x float> poison, <2 x i32> zeroinitializer
  %19 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %16, <2 x float> %18, <2 x float> %9)
  %20 = fadd <2 x float> %14, %19
  %21 = extractelement <2 x float> %20, i64 0
  %22 = extractelement <2 x float> %20, i64 1
  %div.peel = fdiv float %21, %22
  %sub.peel = fsub float 1.000000e+00, %div.peel
  %23 = tail call float @llvm.fabs.f32(float %sub.peel)
  %cmp15.peel = fcmp ule float %23, 0x3F40624DE0000000
  br i1 %cmp15.peel, label %while.end, label %while.body.peel.next

while.body.peel.next:                             ; preds = %entry
  %24 = insertelement <2 x float> poison, float %a32, i64 0
  %25 = insertelement <2 x float> %24, float %a22, i64 1
  %26 = fmul <2 x float> %1, %25
  %27 = insertelement <2 x float> poison, float %a34, i64 0
  %28 = insertelement <2 x float> %27, float %a24, i64 1
  %29 = fmul <2 x float> %6, %28
  %30 = insertelement <2 x float> poison, float %a31, i64 0
  %31 = insertelement <2 x float> %30, float %a21, i64 1
  %32 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %31, <2 x float> %13, <2 x float> %26)
  %33 = insertelement <2 x float> poison, float %a33, i64 0
  %34 = insertelement <2 x float> %33, float %a23, i64 1
  %35 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %34, <2 x float> %18, <2 x float> %29)
  %36 = fadd <2 x float> %32, %35
  %37 = shufflevector <2 x float> %20, <2 x float> poison, <2 x i32> <i32 1, i32 1>
  %38 = fdiv <2 x float> %36, %37
  br label %while.body

while.body:                                       ; preds = %while.body.peel.next, %while.body
  %v1_1.035 = phi float [ %div.peel, %while.body.peel.next ], [ %div, %while.body ]
  %39 = phi <2 x float> [ %38, %while.body.peel.next ], [ %55, %while.body ]
  %40 = shufflevector <2 x float> %39, <2 x float> poison, <2 x i32> <i32 1, i32 1>
  %41 = fmul <2 x float> %40, %3
  %42 = shufflevector <2 x float> %39, <2 x float> poison, <2 x i32> zeroinitializer
  %43 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %16, <2 x float> %42, <2 x float> %8)
  %44 = fmul <2 x float> %40, %25
  %45 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %34, <2 x float> %42, <2 x float> %28)
  %46 = insertelement <2 x float> poison, float %v1_1.035, i64 0
  %47 = shufflevector <2 x float> %46, <2 x float> poison, <2 x i32> zeroinitializer
  %48 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %31, <2 x float> %47, <2 x float> %44)
  %49 = fadd <2 x float> %48, %45
  %50 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %11, <2 x float> %47, <2 x float> %41)
  %51 = fadd <2 x float> %50, %43
  %52 = extractelement <2 x float> %51, i64 0
  %53 = extractelement <2 x float> %51, i64 1
  %div = fdiv float %52, %53
  %54 = shufflevector <2 x float> %51, <2 x float> poison, <2 x i32> <i32 1, i32 1>
  %55 = fdiv <2 x float> %49, %54
  %sub = fsub float 1.000000e+00, %div
  %56 = tail call float @llvm.fabs.f32(float %sub)
  %cmp15 = fcmp ule float %56, 0x3F40624DE0000000
  br i1 %cmp15, label %while.end, label %while.body, !llvm.loop !5

while.end:                                        ; preds = %while.body, %entry
  %div.lcssa = phi float [ %div.peel, %entry ], [ %div, %while.body ]
  ret float %div.lcssa
}

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
!5 = distinct !{!5, !6, !7}
!6 = !{!"llvm.loop.mustprogress"}
!7 = !{!"llvm.loop.peeled.count", i32 1}
