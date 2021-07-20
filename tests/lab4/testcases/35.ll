; ModuleID = 'cminus'
source_filename = "35.cminus"

@x = global [1 x i32] zeroinitializer
@y = global [1 x i32] zeroinitializer
declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define i32 @gcd(i32 %0, i32 %1) {
label.entry:
  %2 = alloca i32
  %3 = alloca i32
  store i32 %0, i32* %2
  store i32 %1, i32* %3
  %4 = load i32, i32* %3
  %5 = icmp eq i32 %4, 0
  %6 = zext i1 %5 to i32
  %7 = icmp ne i32 %6, 0
  br i1 %7, label %8, label %10
8:
  %9 = load i32, i32* %2
  ret i32 %9
10:
  %11 = load i32, i32* %3
  %12 = load i32, i32* %2
  %13 = load i32, i32* %2
  %14 = load i32, i32* %3
  %15 = sdiv i32 %13, %14
  %16 = load i32, i32* %3
  %17 = mul i32 %15, %16
  %18 = sub i32 %12, %17
  %19 = call i32 @gcd(i32 %11, i32 %18)
  ret i32 %19
20:
  ret i32 0
}
define i32 @funArray(i32* %0, i32* %1) {
label.entry:
  %2 = alloca i32*
  %3 = alloca i32*
  store i32* %0, i32** %2
  store i32* %1, i32** %3
  %4 = alloca i32
  %5 = alloca i32
  %6 = alloca i32
  %7 = icmp sge i32 0, 0
  br i1 %7, label %8, label %11
8:
  %9 = load i32*, i32** %2
  %10 = getelementptr i32, i32* %9, i32 0
  br label %12
11:
  call void @neg_idx_except()
  br label %12
12:
  %13 = load i32, i32* %10
  store i32 %13, i32* %4
  %14 = icmp sge i32 0, 0
  br i1 %14, label %15, label %18
15:
  %16 = load i32*, i32** %3
  %17 = getelementptr i32, i32* %16, i32 0
  br label %19
18:
  call void @neg_idx_except()
  br label %19
19:
  %20 = load i32, i32* %17
  store i32 %20, i32* %5
  %21 = load i32, i32* %4
  %22 = load i32, i32* %5
  %23 = icmp slt i32 %21, %22
  %24 = zext i1 %23 to i32
  %25 = icmp ne i32 %24, 0
  br i1 %25, label %26, label %30
26:
  %27 = load i32, i32* %4
  store i32 %27, i32* %6
  %28 = load i32, i32* %5
  store i32 %28, i32* %4
  %29 = load i32, i32* %6
  store i32 %29, i32* %5
  br label %30
30:
  %31 = load i32, i32* %4
  %32 = load i32, i32* %5
  %33 = call i32 @gcd(i32 %31, i32 %32)
  ret i32 %33
}
define i32 @main() {
label.entry:
  %0 = icmp sge i32 0, 0
  br i1 %0, label %1, label %5
1:
  %2 = load [1 x i32], [1 x i32]* @x
  %3 = getelementptr [1 x i32], [1 x i32]* @x, i32 0, i32 0
  %4 = getelementptr i32, i32* %3, i32 0
  br label %6
5:
  call void @neg_idx_except()
  br label %6
6:
  store i32 90, i32* %4
  %7 = icmp sge i32 0, 0
  br i1 %7, label %8, label %12
8:
  %9 = load [1 x i32], [1 x i32]* @y
  %10 = getelementptr [1 x i32], [1 x i32]* @y, i32 0, i32 0
  %11 = getelementptr i32, i32* %10, i32 0
  br label %13
12:
  call void @neg_idx_except()
  br label %13
13:
  store i32 18, i32* %11
  %14 = getelementptr [1 x i32], [1 x i32]* @x, i32 0, i32 0
  %15 = getelementptr [1 x i32], [1 x i32]* @y, i32 0, i32 0
  %16 = call i32 @funArray(i32* %14, i32* %15)
  ret i32 %16
}
