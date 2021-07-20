; ModuleID = 'cminus'
source_filename = "32.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define i32 @main() {
label.entry:
  %0 = alloca i32
  %1 = icmp ne i32 1, 0
  br i1 %1, label %2, label %4
2:
  %3 = icmp ne i32 0, 0
  br i1 %3, label %6, label %7
4:
  store i32 8, i32* %0
  br label %5
5:
  ret i32 0
6:
  store i32 2, i32* %0
  br label %8
7:
  store i32 6, i32* %0
  br label %8
8:
  %9 = load i32, i32* %0
  %10 = add i32 %9, 1
  store i32 %10, i32* %0
  br label %5
}
