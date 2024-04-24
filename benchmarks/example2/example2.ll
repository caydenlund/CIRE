; ModuleID = 'example2.cpp'
source_filename = "example2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%"class.std::basic_ostream" = type { i32 (...)**, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", %"class.std::basic_ostream"*, i8, i8, %"class.std::basic_streambuf"*, %"class.std::ctype"*, %"class.std::num_put"*, %"class.std::num_get"* }
%"class.std::ios_base" = type { i32 (...)**, i64, i64, i32, i32, i32, %"struct.std::ios_base::_Callback_list"*, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, %"struct.std::ios_base::_Words"*, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { %"struct.std::ios_base::_Callback_list"*, void (i32, %"class.std::ios_base"*, i32)*, i32, i32 }
%"struct.std::ios_base::_Words" = type { i8*, i64 }
%"class.std::locale" = type { %"class.std::locale::_Impl"* }
%"class.std::locale::_Impl" = type { i32, %"class.std::locale::facet"**, i64, %"class.std::locale::facet"**, i8** }
%"class.std::locale::facet" = type <{ i32 (...)**, i32, [4 x i8] }>
%"class.std::basic_streambuf" = type { i32 (...)**, i8*, i8*, i8*, i8*, i8*, i8*, %"class.std::locale" }
%"class.std::ctype" = type <{ %"class.std::locale::facet.base", [4 x i8], %struct.__locale_struct*, i8, [7 x i8], i32*, i32*, i16*, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class.std::locale::facet.base" = type <{ i32 (...)**, i32 }>
%struct.__locale_struct = type { [13 x %struct.__locale_data*], i16*, i32*, i32*, [13 x i8*] }
%struct.__locale_data = type opaque
%"class.std::num_put" = type { %"class.std::locale::facet.base", [4 x i8] }
%"class.std::num_get" = type { %"class.std::locale::facet.base", [4 x i8] }

$_ZSt13__check_facetISt5ctypeIcEERKT_PS3_ = comdat any

$_ZNKSt5ctypeIcE5widenEc = comdat any

@_ZStL8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@_ZSt4cout = external dso_local global %"class.std::basic_ostream", align 8
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_example2.cpp, i8* null }]

; Function Attrs: uwtable
define internal fastcc void @__cxx_global_var_init() unnamed_addr #0 section ".text.startup" {
  call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* nonnull dereferenceable(1) @_ZStL8__ioinit)
  %1 = call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZStL8__ioinit, i64 0, i32 0), i8* nonnull @__dso_handle) #9
  ret void
}

declare dso_local void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* nonnull dereferenceable(1)) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"* nonnull dereferenceable(1)) unnamed_addr #2

; Function Attrs: nofree nounwind
declare dso_local i32 @__cxa_atexit(void (i8*)*, i8*, i8*) local_unnamed_addr #3

; Function Attrs: norecurse nounwind readnone uwtable willreturn mustprogress
define dso_local double @src(double %x, double %y) local_unnamed_addr #4 {
  %1 = fsub double %x, %y
  %2 = fadd double %1, %y
  ret double %2
}

; Function Attrs: norecurse uwtable mustprogress
define dso_local i32 @main() local_unnamed_addr #5 {
  %1 = call double @src(double 2.000000e+00, double 3.000000e+00)
  %2 = call nonnull align 8 dereferenceable(8) %"class.std::basic_ostream"* @_ZNSolsEd(%"class.std::basic_ostream"* nonnull dereferenceable(8) @_ZSt4cout, double %1)
  %3 = call nonnull align 8 dereferenceable(8) %"class.std::basic_ostream"* @_ZNSolsEPFRSoS_E(%"class.std::basic_ostream"* nonnull dereferenceable(8) %2, %"class.std::basic_ostream"* (%"class.std::basic_ostream"*)* nonnull @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_)
  ret i32 0
}

; Function Attrs: uwtable mustprogress
define available_externally dso_local nonnull align 8 dereferenceable(8) %"class.std::basic_ostream"* @_ZNSolsEd(%"class.std::basic_ostream"* nonnull dereferenceable(8) %0, double %1) local_unnamed_addr #6 align 2 {
  %3 = call nonnull align 8 dereferenceable(8) %"class.std::basic_ostream"* @_ZNSo9_M_insertIdEERSoT_(%"class.std::basic_ostream"* nonnull dereferenceable(8) %0, double %1)
  ret %"class.std::basic_ostream"* %3
}

; Function Attrs: uwtable mustprogress
define available_externally dso_local nonnull align 8 dereferenceable(8) %"class.std::basic_ostream"* @_ZNSolsEPFRSoS_E(%"class.std::basic_ostream"* nonnull dereferenceable(8) %0, %"class.std::basic_ostream"* (%"class.std::basic_ostream"*)* %1) local_unnamed_addr #6 align 2 {
  %3 = call nonnull align 8 dereferenceable(8) %"class.std::basic_ostream"* %1(%"class.std::basic_ostream"* nonnull align 8 dereferenceable(8) %0)
  ret %"class.std::basic_ostream"* %3
}

; Function Attrs: inlinehint uwtable mustprogress
define available_externally dso_local nonnull align 8 dereferenceable(8) %"class.std::basic_ostream"* @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_(%"class.std::basic_ostream"* nonnull align 8 dereferenceable(8) %0) #7 {
  %2 = bitcast %"class.std::basic_ostream"* %0 to i8**
  %3 = load i8*, i8** %2, align 8, !tbaa !2
  %4 = getelementptr i8, i8* %3, i64 -24
  %5 = bitcast i8* %4 to i64*
  %6 = load i64, i64* %5, align 8
  %7 = bitcast %"class.std::basic_ostream"* %0 to i8*
  %8 = getelementptr inbounds i8, i8* %7, i64 %6
  %9 = bitcast i8* %8 to %"class.std::basic_ios"*
  %10 = call signext i8 @_ZNKSt9basic_iosIcSt11char_traitsIcEE5widenEc(%"class.std::basic_ios"* nonnull dereferenceable(264) %9, i8 signext 10)
  %11 = call nonnull align 8 dereferenceable(8) %"class.std::basic_ostream"* @_ZNSo3putEc(%"class.std::basic_ostream"* nonnull dereferenceable(8) %0, i8 signext %10)
  %12 = call nonnull align 8 dereferenceable(8) %"class.std::basic_ostream"* @_ZSt5flushIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_(%"class.std::basic_ostream"* nonnull align 8 dereferenceable(8) %11)
  ret %"class.std::basic_ostream"* %12
}

declare dso_local nonnull align 8 dereferenceable(8) %"class.std::basic_ostream"* @_ZNSo9_M_insertIdEERSoT_(%"class.std::basic_ostream"* nonnull dereferenceable(8), double) local_unnamed_addr #1

; Function Attrs: inlinehint uwtable mustprogress
define available_externally dso_local nonnull align 8 dereferenceable(8) %"class.std::basic_ostream"* @_ZSt5flushIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_(%"class.std::basic_ostream"* nonnull align 8 dereferenceable(8) %0) local_unnamed_addr #7 {
  %2 = call nonnull align 8 dereferenceable(8) %"class.std::basic_ostream"* @_ZNSo5flushEv(%"class.std::basic_ostream"* nonnull dereferenceable(8) %0)
  ret %"class.std::basic_ostream"* %2
}

declare dso_local nonnull align 8 dereferenceable(8) %"class.std::basic_ostream"* @_ZNSo3putEc(%"class.std::basic_ostream"* nonnull dereferenceable(8), i8 signext) local_unnamed_addr #1

; Function Attrs: uwtable mustprogress
define available_externally dso_local signext i8 @_ZNKSt9basic_iosIcSt11char_traitsIcEE5widenEc(%"class.std::basic_ios"* nonnull dereferenceable(264) %0, i8 signext %1) local_unnamed_addr #6 align 2 {
  %3 = getelementptr inbounds %"class.std::basic_ios", %"class.std::basic_ios"* %0, i64 0, i32 5
  %4 = load %"class.std::ctype"*, %"class.std::ctype"** %3, align 8, !tbaa !5
  %5 = call nonnull align 8 dereferenceable(570) %"class.std::ctype"* @_ZSt13__check_facetISt5ctypeIcEERKT_PS3_(%"class.std::ctype"* %4)
  %6 = call signext i8 @_ZNKSt5ctypeIcE5widenEc(%"class.std::ctype"* nonnull dereferenceable(570) %5, i8 signext %1)
  ret i8 %6
}

declare dso_local nonnull align 8 dereferenceable(8) %"class.std::basic_ostream"* @_ZNSo5flushEv(%"class.std::basic_ostream"* nonnull dereferenceable(8)) local_unnamed_addr #1

; Function Attrs: inlinehint uwtable mustprogress
define linkonce_odr dso_local nonnull align 8 dereferenceable(570) %"class.std::ctype"* @_ZSt13__check_facetISt5ctypeIcEERKT_PS3_(%"class.std::ctype"* %0) local_unnamed_addr #7 comdat {
  %2 = icmp eq %"class.std::ctype"* %0, null
  br i1 %2, label %3, label %4

3:                                                ; preds = %1
  call void @_ZSt16__throw_bad_castv() #10
  unreachable

4:                                                ; preds = %1
  ret %"class.std::ctype"* %0
}

; Function Attrs: uwtable mustprogress
define linkonce_odr dso_local signext i8 @_ZNKSt5ctypeIcE5widenEc(%"class.std::ctype"* nonnull dereferenceable(570) %0, i8 signext %1) local_unnamed_addr #6 comdat align 2 {
  %3 = getelementptr inbounds %"class.std::ctype", %"class.std::ctype"* %0, i64 0, i32 8
  %4 = load i8, i8* %3, align 8, !tbaa !10
  %5 = icmp eq i8 %4, 0
  br i1 %5, label %10, label %6

6:                                                ; preds = %2
  %7 = zext i8 %1 to i64
  %8 = getelementptr inbounds %"class.std::ctype", %"class.std::ctype"* %0, i64 0, i32 9, i64 %7
  %9 = load i8, i8* %8, align 1, !tbaa !12
  br label %16

10:                                               ; preds = %2
  call void @_ZNKSt5ctypeIcE13_M_widen_initEv(%"class.std::ctype"* nonnull dereferenceable(570) %0)
  %11 = bitcast %"class.std::ctype"* %0 to i8 (%"class.std::ctype"*, i8)***
  %12 = load i8 (%"class.std::ctype"*, i8)**, i8 (%"class.std::ctype"*, i8)*** %11, align 8, !tbaa !2
  %13 = getelementptr inbounds i8 (%"class.std::ctype"*, i8)*, i8 (%"class.std::ctype"*, i8)** %12, i64 6
  %14 = load i8 (%"class.std::ctype"*, i8)*, i8 (%"class.std::ctype"*, i8)** %13, align 8
  %15 = call signext i8 %14(%"class.std::ctype"* nonnull dereferenceable(570) %0, i8 signext %1)
  br label %16

16:                                               ; preds = %10, %6
  %17 = phi i8 [ %9, %6 ], [ %15, %10 ]
  ret i8 %17
}

; Function Attrs: noreturn
declare dso_local void @_ZSt16__throw_bad_castv() local_unnamed_addr #8

declare dso_local void @_ZNKSt5ctypeIcE13_M_widen_initEv(%"class.std::ctype"* nonnull dereferenceable(570)) local_unnamed_addr #1

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_example2.cpp() #0 section ".text.startup" {
  call fastcc void @__cxx_global_var_init()
  ret void
}

attributes #0 = { uwtable "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nofree nounwind }
attributes #4 = { norecurse nounwind readnone uwtable willreturn mustprogress "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { norecurse uwtable mustprogress "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { uwtable mustprogress "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { inlinehint uwtable mustprogress "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { noreturn "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #9 = { nounwind }
attributes #10 = { noreturn }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Ubuntu clang version 12.0.1-19ubuntu3"}
!2 = !{!3, !3, i64 0}
!3 = !{!"vtable pointer", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
!5 = !{!6, !7, i64 240}
!6 = !{!"_ZTSSt9basic_iosIcSt11char_traitsIcEE", !7, i64 216, !8, i64 224, !9, i64 225, !7, i64 232, !7, i64 240, !7, i64 248, !7, i64 256}
!7 = !{!"any pointer", !8, i64 0}
!8 = !{!"omnipotent char", !4, i64 0}
!9 = !{!"bool", !8, i64 0}
!10 = !{!11, !8, i64 56}
!11 = !{!"_ZTSSt5ctypeIcE", !7, i64 16, !9, i64 24, !7, i64 32, !7, i64 40, !7, i64 48, !8, i64 56, !8, i64 57, !8, i64 313, !8, i64 569}
!12 = !{!8, !8, i64 0}
