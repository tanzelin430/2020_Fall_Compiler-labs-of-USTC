; ModuleID = 'if.c'
source_filename = "if.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
	%1 = alloca float ; allocate mem space for variable a
	store float 0x40163851E0000000, float* %1 ; store the radix-16 of float 5.555 into mem
	%2 = load float, float* %1 ; load the value back into register
	%3 = fcmp ogt float %2, 1.0000000e+00 ; compare 5.555 with 1.0000000
	%4 = alloca i32
	br i1 %3, label %5, label %6 ; branch depending on cases
5:
	store i32 233, i32* %4 ; save the return value into memory
	br label %7
6:
	store i32 0, i32* %4 ; save the return value into memory
	br label %7
7: ; return state
	%8 = load i32, i32* %4 ; fetch return value
	ret i32 %8
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}
