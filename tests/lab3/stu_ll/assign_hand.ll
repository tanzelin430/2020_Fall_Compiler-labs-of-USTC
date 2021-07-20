; ModuleID and source_filename are fixed to a same C source file
; ModuleID = 'assign.c'
source_filename = "assign.c"
; These two lines are the determained by machine
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; No global variables

; for main function
; This line is not even English!
; dso_local is a 'Runtime Preemption' description sign，表明该函数会在同一个链接单元（即该函数所在的文件以及包含的头文件）内解析符号
; #0 stands for the attribute group this function is in
; Function Attrs: noinline nounwind optnone uwtable
; int main()
define dso_local i32 @main() #0 {
	%1 = alloca [10 x i32] ; allocate memory for the array
	%2 = getelementptr inbounds [10 x i32], [10 x i32]* %1, i32 0, i32 0 ; get the address of a[0]
	store i32 10, i32* %2 ; sw a[0], 10
	%3 = load i32, i32* %2 ; lw a[0], %3
	%4 = alloca i32 ; allocate memory for immediate '2'
	store i32 2, i32* %4 ; sw, use memory to put immediate into register
	%5 = load i32, i32* %4 ; lw
	%6 = mul i32 %3, %5 ; multiple calaulation between registers
	%7 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 1 ; get the address of a[1]
	store i32 %6, i32* %7 ; save the mul result to memory
	ret i32 %6 ; return mul result
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}
