; ModuleID = 'hello.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: uwtable
define void @_Z7notmainii(i32 %a, i32 %b) #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 %a, i32* %1, align 4
  call void @llvm.dbg.declare(metadata !{i32* %1}, metadata !15), !dbg !16
  store i32 %b, i32* %2, align 4
  call void @llvm.dbg.declare(metadata !{i32* %2}, metadata !17), !dbg !18
  %3 = load i32* %1, align 4, !dbg !19
  %4 = load i32* %2, align 4, !dbg !19
  %5 = icmp sgt i32 %3, %4, !dbg !19
  br i1 %5, label %6, label %25, !dbg !19

; <label>:6                                       ; preds = %0
  %7 = load i32* %1, align 4, !dbg !21
  %8 = add nsw i32 %7, 1, !dbg !21
  %9 = load i32* %2, align 4, !dbg !21
  %10 = icmp sgt i32 %8, %9, !dbg !21
  br i1 %10, label %11, label %14, !dbg !21

; <label>:11                                      ; preds = %6
  %12 = load i32* %2, align 4, !dbg !24
  %13 = add nsw i32 %12, 1, !dbg !24
  store i32 %13, i32* %2, align 4, !dbg !24
  br label %24, !dbg !24

; <label>:14                                      ; preds = %6
  %15 = load i32* %1, align 4, !dbg !25
  %16 = add nsw i32 %15, 1, !dbg !25
  %17 = load i32* %2, align 4, !dbg !25
  %18 = add nsw i32 %17, 2, !dbg !25
  %19 = call i32 @_Z8notmain2ii(i32 %16, i32 %18), !dbg !25
  store i32 %19, i32* %2, align 4, !dbg !25
  %20 = load i32* %2, align 4, !dbg !27
  %21 = load i32* %1, align 4, !dbg !28
  %22 = call i32 @abs(i32 %21) #1, !dbg !28
  %23 = add nsw i32 %20, %22, !dbg !28
  store i32 %23, i32* %2, align 4, !dbg !28
  br label %24

; <label>:24                                      ; preds = %14, %11
  br label %37, !dbg !29

; <label>:25                                      ; preds = %0
  call void @llvm.dbg.declare(metadata !{i32* %x}, metadata !30), !dbg !32
  store i32 1, i32* %x, align 4, !dbg !33
  br label %26, !dbg !34

; <label>:26                                      ; preds = %29, %25
  %27 = load i32* %x, align 4, !dbg !35
  %28 = icmp sgt i32 %27, 0, !dbg !35
  br i1 %28, label %29, label %34, !dbg !35

; <label>:29                                      ; preds = %26
  %30 = load i32* %1, align 4, !dbg !37
  %31 = add nsw i32 %30, 1, !dbg !37
  store i32 %31, i32* %1, align 4, !dbg !37
  %32 = load i32* %x, align 4, !dbg !39
  %33 = add nsw i32 %32, -1, !dbg !39
  store i32 %33, i32* %x, align 4, !dbg !39
  br label %26, !dbg !40

; <label>:34                                      ; preds = %26
  %35 = load i32* %2, align 4, !dbg !41
  %36 = add nsw i32 %35, 2, !dbg !41
  store i32 %36, i32* %2, align 4, !dbg !41
  br label %37

; <label>:37                                      ; preds = %34, %24
  ret void, !dbg !42
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata) #1

; Function Attrs: nounwind uwtable
define i32 @_Z8notmain2ii(i32 %c, i32 %d) #2 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 %c, i32* %2, align 4
  call void @llvm.dbg.declare(metadata !{i32* %2}, metadata !43), !dbg !44
  store i32 %d, i32* %3, align 4
  call void @llvm.dbg.declare(metadata !{i32* %3}, metadata !45), !dbg !46
  %4 = load i32* %2, align 4, !dbg !47
  %5 = load i32* %3, align 4, !dbg !47
  %6 = icmp sgt i32 %4, %5, !dbg !47
  br i1 %6, label %7, label %11, !dbg !47

; <label>:7                                       ; preds = %0
  %8 = load i32* %2, align 4, !dbg !49
  %9 = load i32* %3, align 4, !dbg !49
  %10 = add nsw i32 %8, %9, !dbg !49
  store i32 %10, i32* %1, !dbg !49
  br label %15, !dbg !49

; <label>:11                                      ; preds = %0
  %12 = load i32* %2, align 4, !dbg !50
  %13 = load i32* %3, align 4, !dbg !50
  %14 = sub nsw i32 %12, %13, !dbg !50
  store i32 %14, i32* %1, !dbg !50
  br label %15, !dbg !50

; <label>:15                                      ; preds = %11, %7
  %16 = load i32* %1, !dbg !51
  ret i32 %16, !dbg !51
}

; Function Attrs: nounwind readnone
declare i32 @abs(i32) #3

attributes #0 = { uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!12, !13}
!llvm.ident = !{!14}

!0 = metadata !{i32 786449, metadata !1, i32 4, metadata !"Ubuntu clang version 3.5.2-svn232544-1~exp1 (branches/release_35) (based on LLVM 3.5.2)", i1 false, metadata !"", i32 0, metadata !2, metadata !2, metadata !3, metadata !2, metadata !2, metadata !"", i32 1} ; [ DW_TAG_compile_unit ] [/home/xilenteyex/sproj_git/sproj/src/webserver/samples/hello.cpp] [DW_LANG_C_plus_plus]
!1 = metadata !{metadata !"hello.cpp", metadata !"/home/xilenteyex/sproj_git/sproj/src/webserver/samples"}
!2 = metadata !{}
!3 = metadata !{metadata !4, metadata !9}
!4 = metadata !{i32 786478, metadata !1, metadata !5, metadata !"notmain", metadata !"notmain", metadata !"_Z7notmainii", i32 3, metadata !6, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (i32, i32)* @_Z7notmainii, null, null, metadata !2, i32 4} ; [ DW_TAG_subprogram ] [line 3] [def] [scope 4] [notmain]
!5 = metadata !{i32 786473, metadata !1}          ; [ DW_TAG_file_type ] [/home/xilenteyex/sproj_git/sproj/src/webserver/samples/hello.cpp]
!6 = metadata !{i32 786453, i32 0, null, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !7, i32 0, null, null, null} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!7 = metadata !{null, metadata !8, metadata !8}
!8 = metadata !{i32 786468, null, null, metadata !"int", i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ] [int] [line 0, size 32, align 32, offset 0, enc DW_ATE_signed]
!9 = metadata !{i32 786478, metadata !1, metadata !5, metadata !"notmain2", metadata !"notmain2", metadata !"_Z8notmain2ii", i32 26, metadata !10, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, i32 (i32, i32)* @_Z8notmain2ii, null, null, metadata !2, i32 27} ; [ DW_TAG_subprogram ] [line 26] [def] [scope 27] [notmain2]
!10 = metadata !{i32 786453, i32 0, null, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !11, i32 0, null, null, null} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!11 = metadata !{metadata !8, metadata !8, metadata !8}
!12 = metadata !{i32 2, metadata !"Dwarf Version", i32 4}
!13 = metadata !{i32 2, metadata !"Debug Info Version", i32 1}
!14 = metadata !{metadata !"Ubuntu clang version 3.5.2-svn232544-1~exp1 (branches/release_35) (based on LLVM 3.5.2)"}
!15 = metadata !{i32 786689, metadata !4, metadata !"a", metadata !5, i32 16777219, metadata !8, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [a] [line 3]
!16 = metadata !{i32 3, i32 18, metadata !4, null}
!17 = metadata !{i32 786689, metadata !4, metadata !"b", metadata !5, i32 33554435, metadata !8, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [b] [line 3]
!18 = metadata !{i32 3, i32 25, metadata !4, null}
!19 = metadata !{i32 5, i32 6, metadata !20, null}
!20 = metadata !{i32 786443, metadata !1, metadata !4, i32 5, i32 6, i32 0, i32 0} ; [ DW_TAG_lexical_block ] [/home/xilenteyex/sproj_git/sproj/src/webserver/samples/hello.cpp]
!21 = metadata !{i32 7, i32 6, metadata !22, null}
!22 = metadata !{i32 786443, metadata !1, metadata !23, i32 7, i32 6, i32 0, i32 2} ; [ DW_TAG_lexical_block ] [/home/xilenteyex/sproj_git/sproj/src/webserver/samples/hello.cpp]
!23 = metadata !{i32 786443, metadata !1, metadata !20, i32 6, i32 2, i32 0, i32 1} ; [ DW_TAG_lexical_block ] [/home/xilenteyex/sproj_git/sproj/src/webserver/samples/hello.cpp]
!24 = metadata !{i32 8, i32 4, metadata !22, null} ; [ DW_TAG_imported_declaration ]
!25 = metadata !{i32 11, i32 8, metadata !26, null}
!26 = metadata !{i32 786443, metadata !1, metadata !22, i32 10, i32 3, i32 0, i32 3} ; [ DW_TAG_lexical_block ] [/home/xilenteyex/sproj_git/sproj/src/webserver/samples/hello.cpp]
!27 = metadata !{i32 12, i32 4, metadata !26, null}
!28 = metadata !{i32 12, i32 12, metadata !26, null}
!29 = metadata !{i32 14, i32 2, metadata !23, null}
!30 = metadata !{i32 786688, metadata !31, metadata !"x", metadata !5, i32 17, metadata !8, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [x] [line 17]
!31 = metadata !{i32 786443, metadata !1, metadata !20, i32 16, i32 2, i32 0, i32 4} ; [ DW_TAG_lexical_block ] [/home/xilenteyex/sproj_git/sproj/src/webserver/samples/hello.cpp]
!32 = metadata !{i32 17, i32 7, metadata !31, null}
!33 = metadata !{i32 17, i32 3, metadata !31, null}
!34 = metadata !{i32 18, i32 3, metadata !31, null}
!35 = metadata !{i32 18, i32 3, metadata !36, null}
!36 = metadata !{i32 786443, metadata !1, metadata !31, i32 18, i32 3, i32 1, i32 7} ; [ DW_TAG_lexical_block ] [/home/xilenteyex/sproj_git/sproj/src/webserver/samples/hello.cpp]
!37 = metadata !{i32 20, i32 4, metadata !38, null}
!38 = metadata !{i32 786443, metadata !1, metadata !31, i32 19, i32 3, i32 0, i32 5} ; [ DW_TAG_lexical_block ] [/home/xilenteyex/sproj_git/sproj/src/webserver/samples/hello.cpp]
!39 = metadata !{i32 21, i32 4, metadata !38, null}
!40 = metadata !{i32 22, i32 3, metadata !38, null}
!41 = metadata !{i32 23, i32 3, metadata !31, null}
!42 = metadata !{i32 25, i32 1, metadata !4, null}
!43 = metadata !{i32 786689, metadata !9, metadata !"c", metadata !5, i32 16777242, metadata !8, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [c] [line 26]
!44 = metadata !{i32 26, i32 18, metadata !9, null}
!45 = metadata !{i32 786689, metadata !9, metadata !"d", metadata !5, i32 33554458, metadata !8, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [d] [line 26]
!46 = metadata !{i32 26, i32 25, metadata !9, null}
!47 = metadata !{i32 28, i32 5, metadata !48, null}
!48 = metadata !{i32 786443, metadata !1, metadata !9, i32 28, i32 5, i32 0, i32 6} ; [ DW_TAG_lexical_block ] [/home/xilenteyex/sproj_git/sproj/src/webserver/samples/hello.cpp]
!49 = metadata !{i32 29, i32 3, metadata !48, null}
!50 = metadata !{i32 31, i32 3, metadata !48, null}
!51 = metadata !{i32 32, i32 1, metadata !9, null}
