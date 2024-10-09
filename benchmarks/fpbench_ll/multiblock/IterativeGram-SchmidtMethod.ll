; ModuleID = 'benchmarks_c/IterativeGram-SchmidtMethod.c'
source_filename = "benchmarks_c/IterativeGram-SchmidtMethod.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local float @ex0(float noundef %Q11, float noundef %Q12, float noundef %Q13, float noundef %Q21, float noundef %Q22, float noundef %Q23, float noundef %Q31, float noundef %Q32, float noundef %Q33) local_unnamed_addr #0 {
entry:
  %mul1 = fmul float %Q32, %Q32
  %0 = tail call float @llvm.fmuladd.f32(float %Q31, float %Q31, float %mul1)
  %1 = tail call float @llvm.fmuladd.f32(float %Q33, float %Q33, float %0)
  %sqrt = tail call float @llvm.sqrt.f32(float %1)
  %2 = insertelement <2 x float> poison, float %Q32, i64 0
  %3 = insertelement <2 x float> %2, float %Q31, i64 1
  %4 = insertelement <2 x float> poison, float %Q23, i64 0
  %5 = insertelement <2 x float> %4, float %Q13, i64 1
  %6 = insertelement <2 x float> poison, float %Q21, i64 0
  %7 = insertelement <2 x float> %6, float %Q11, i64 1
  %8 = insertelement <2 x float> poison, float %Q22, i64 0
  %9 = insertelement <2 x float> %8, float %Q12, i64 1
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %rold.069 = phi float [ %sqrt, %entry ], [ %sqrt65, %while.body ]
  %qj3.068 = phi float [ %Q33, %entry ], [ %sub9, %while.body ]
  %10 = phi <2 x float> [ %3, %entry ], [ %28, %while.body ]
  %11 = extractelement <2 x float> %10, i64 0
  %mul2 = fmul float %11, %Q21
  %12 = extractelement <2 x float> %10, i64 1
  %13 = tail call float @llvm.fmuladd.f32(float %Q11, float %12, float %mul2)
  %14 = tail call float @llvm.fmuladd.f32(float %Q31, float %qj3.068, float %13)
  %mul3 = fmul float %11, %Q22
  %15 = tail call float @llvm.fmuladd.f32(float %Q12, float %12, float %mul3)
  %16 = tail call float @llvm.fmuladd.f32(float %Q32, float %qj3.068, float %15)
  %mul4 = fmul float %11, %Q23
  %17 = tail call float @llvm.fmuladd.f32(float %Q13, float %12, float %mul4)
  %18 = tail call float @llvm.fmuladd.f32(float %Q33, float %qj3.068, float %17)
  %19 = insertelement <2 x float> poison, float %16, i64 0
  %20 = shufflevector <2 x float> %19, <2 x float> poison, <2 x i32> zeroinitializer
  %21 = fmul <2 x float> %20, %9
  %22 = insertelement <2 x float> poison, float %14, i64 0
  %23 = shufflevector <2 x float> %22, <2 x float> poison, <2 x i32> zeroinitializer
  %24 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %7, <2 x float> %23, <2 x float> %21)
  %25 = insertelement <2 x float> poison, float %18, i64 0
  %26 = shufflevector <2 x float> %25, <2 x float> poison, <2 x i32> zeroinitializer
  %27 = tail call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %5, <2 x float> %26, <2 x float> %24)
  %28 = fsub <2 x float> %10, %27
  %mul8 = fmul float %16, %Q32
  %29 = tail call float @llvm.fmuladd.f32(float %Q31, float %14, float %mul8)
  %30 = tail call float @llvm.fmuladd.f32(float %Q33, float %18, float %29)
  %sub9 = fsub float %qj3.068, %30
  %31 = fmul <2 x float> %28, %28
  %mul12 = extractelement <2 x float> %31, i64 0
  %32 = extractelement <2 x float> %28, i64 1
  %33 = tail call float @llvm.fmuladd.f32(float %32, float %32, float %mul12)
  %34 = tail call float @llvm.fmuladd.f32(float %sub9, float %sub9, float %33)
  %sqrt65 = tail call float @llvm.sqrt.f32(float %34)
  %div = fdiv float %sqrt65, %rold.069
  %sub14 = fsub float 1.000000e+00, %div
  %35 = tail call float @llvm.fabs.f32(float %sub14)
  %cmp16 = fcmp ule float %35, 0x3ED4F8B580000000
  br i1 %cmp16, label %while.end, label %while.body, !llvm.loop !5

while.end:                                        ; preds = %while.body
  ret float %32
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.sqrt.f32(float) #2

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
