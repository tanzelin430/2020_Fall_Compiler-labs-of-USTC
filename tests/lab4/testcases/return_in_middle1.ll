; ModuleID = 'cminus'
source_filename = "return_in_middle1.cminus"

@x = global [1 x i32] zeroinitializer
declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define i32 @call(i32* %0) {
label.entry:
  %1 = alloca i32*
  store i32* %0, i32** %1
  %2 = icmp sge i32 0, 0
  br i1 %2, label %3, label %6
3:
  %4 = load i32*, i32** %1
  %5 = getelementptr i32, i32* %4, i32 0
  br label %7
6:
  call void @neg_idx_except()
  br label %7
7:
  %8 = load i32, i32* %5
  call void @output(i32 %8)
  ret i32 0
}
define void @main() {
label.entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = icmp ne i32 1, 0
  br i1 %1, label %2, label %3
2:
  br label %5
3:
  store i32 4, i32* %0
  ret void
4:
  ret void
5:
  %6 = load i32, i32* %0
  %7 = icmp sgt i32 %6, 3
  %8 = zext i1 %7 to i32
  %9 = icmp ne i32 %8, 0
  br i1 %9, label %10, label %11
10:
  store i32 1, i32* %0
  ret void
11:
  store i32 3, i32* %0
  ret void
}
