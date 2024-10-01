; ModuleID = 'benchmarks_c/hartman6.c'
source_filename = "benchmarks_c/hartman6.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nounwind willreturn memory(write) uwtable
define dso_local double @ex0(double noundef %x1, double noundef %x2, double noundef %x3, double noundef %x4, double noundef %x5, double noundef %x6) local_unnamed_addr #0 {
entry:
  %sub = fadd double %x1, -1.312000e-01
  %mul = fmul double %sub, %sub
  %sub3 = fadd double %x2, -1.696000e-01
  %mul5 = fmul double %sub3, %sub3
  %mul6 = fmul double %mul5, 3.000000e+00
  %0 = tail call double @llvm.fmuladd.f64(double %mul, double 1.000000e+01, double %mul6)
  %sub7 = fadd double %x3, -5.569000e-01
  %mul9 = fmul double %sub7, %sub7
  %1 = tail call double @llvm.fmuladd.f64(double %mul9, double 1.700000e+01, double %0)
  %sub11 = fadd double %x4, -1.240000e-02
  %mul13 = fmul double %sub11, %sub11
  %2 = tail call double @llvm.fmuladd.f64(double %mul13, double 3.500000e+00, double %1)
  %sub15 = fadd double %x5, -8.283000e-01
  %mul17 = fmul double %sub15, %sub15
  %3 = tail call double @llvm.fmuladd.f64(double %mul17, double 1.700000e+00, double %2)
  %sub19 = fadd double %x6, -5.886000e-01
  %mul21 = fmul double %sub19, %sub19
  %4 = tail call double @llvm.fmuladd.f64(double %mul21, double 8.000000e+00, double %3)
  %sub23 = fadd double %x1, -2.329000e-01
  %mul25 = fmul double %sub23, %sub23
  %sub27 = fadd double %x2, -4.135000e-01
  %mul29 = fmul double %sub27, %sub27
  %mul30 = fmul double %mul29, 1.000000e+01
  %5 = tail call double @llvm.fmuladd.f64(double %mul25, double 5.000000e-02, double %mul30)
  %sub31 = fadd double %x3, 0xBFEA95182A9930BE
  %mul33 = fmul double %sub31, %sub31
  %6 = tail call double @llvm.fmuladd.f64(double %mul33, double 1.700000e+01, double %5)
  %sub35 = fadd double %x4, -3.736000e-01
  %mul37 = fmul double %sub35, %sub35
  %7 = tail call double @llvm.fmuladd.f64(double %mul37, double 1.000000e-01, double %6)
  %sub39 = fadd double %x5, -1.004000e-01
  %mul41 = fmul double %sub39, %sub39
  %8 = tail call double @llvm.fmuladd.f64(double %mul41, double 8.000000e+00, double %7)
  %sub43 = fadd double %x6, 0xBFEFF8A0902DE00D
  %mul45 = fmul double %sub43, %sub43
  %9 = tail call double @llvm.fmuladd.f64(double %mul45, double 1.400000e+01, double %8)
  %sub47 = fadd double %x1, -2.348000e-01
  %mul49 = fmul double %sub47, %sub47
  %sub51 = fadd double %x2, -1.451000e-01
  %mul53 = fmul double %sub51, %sub51
  %mul54 = fmul double %mul53, 3.500000e+00
  %10 = tail call double @llvm.fmuladd.f64(double %mul49, double 3.000000e+00, double %mul54)
  %sub55 = fadd double %x3, -3.522000e-01
  %mul57 = fmul double %sub55, %sub55
  %11 = tail call double @llvm.fmuladd.f64(double %mul57, double 1.700000e+00, double %10)
  %sub59 = fadd double %x4, -2.883000e-01
  %mul61 = fmul double %sub59, %sub59
  %12 = tail call double @llvm.fmuladd.f64(double %mul61, double 1.000000e+01, double %11)
  %sub63 = fadd double %x5, -3.047000e-01
  %mul65 = fmul double %sub63, %sub63
  %13 = tail call double @llvm.fmuladd.f64(double %mul65, double 1.700000e+01, double %12)
  %sub67 = fadd double %x6, -6.650000e-01
  %mul69 = fmul double %sub67, %sub67
  %14 = tail call double @llvm.fmuladd.f64(double %mul69, double 8.000000e+00, double %13)
  %sub71 = fadd double %x1, -4.047000e-01
  %mul73 = fmul double %sub71, %sub71
  %sub75 = fadd double %x2, -8.828000e-01
  %mul77 = fmul double %sub75, %sub75
  %mul78 = fmul double %mul77, 8.000000e+00
  %15 = tail call double @llvm.fmuladd.f64(double %mul73, double 1.700000e+01, double %mul78)
  %sub79 = fadd double %x3, 0xBFEBF141205BC01A
  %mul81 = fmul double %sub79, %sub79
  %16 = tail call double @llvm.fmuladd.f64(double %mul81, double 5.000000e-02, double %15)
  %sub83 = fadd double %x4, -5.743000e-01
  %mul85 = fmul double %sub83, %sub83
  %17 = tail call double @llvm.fmuladd.f64(double %mul85, double 1.000000e+01, double %16)
  %sub87 = fadd double %x5, -1.091000e-01
  %mul89 = fmul double %sub87, %sub87
  %18 = tail call double @llvm.fmuladd.f64(double %mul89, double 1.000000e-01, double %17)
  %sub91 = fadd double %x6, -3.810000e-02
  %mul93 = fmul double %sub91, %sub91
  %19 = tail call double @llvm.fmuladd.f64(double %mul93, double 1.400000e+01, double %18)
  %fneg = fneg double %4
  %call = tail call double @exp(double noundef %fneg) #3, !tbaa !5
  %fneg95 = fneg double %9
  %call96 = tail call double @exp(double noundef %fneg95) #3, !tbaa !5
  %fneg97 = fneg double %14
  %call98 = tail call double @exp(double noundef %fneg97) #3, !tbaa !5
  %fneg99 = fneg double %19
  %call100 = tail call double @exp(double noundef %fneg99) #3, !tbaa !5
  %mul102 = fmul double %call96, 1.200000e+00
  %20 = fadd double %call, %mul102
  %21 = tail call double @llvm.fmuladd.f64(double %call98, double 3.000000e+00, double %20)
  %22 = tail call double @llvm.fmuladd.f64(double %call100, double 3.200000e+00, double %21)
  %fneg105 = fneg double %22
  ret double %fneg105
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

; Function Attrs: mustprogress nofree nounwind willreturn memory(write)
declare double @exp(double noundef) local_unnamed_addr #2

attributes #0 = { mustprogress nofree nounwind willreturn memory(write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { mustprogress nofree nounwind willreturn memory(write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
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
