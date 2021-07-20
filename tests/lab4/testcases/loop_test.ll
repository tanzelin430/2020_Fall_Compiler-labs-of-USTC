; ModuleID = 'cminus'
source_filename = "loop_test.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label.entry:
  %0 = alloca i32
  %1 = alloca i32
  %2 = alloca i32
  store i32 1, i32* %0
  br label %3
3:
  %4 = load i32, i32* %0
  %5 = icmp slt i32 %4, 10000
  %6 = zext i1 %5 to i32
  %7 = icmp ne i32 %6, 0
  br i1 %7, label %8, label %9
8:
  store i32 0, i32* %1
  br label %11
9:
  %10 = load i32, i32* %2
  call void @output(i32 %10)
  ret void
11:
  %12 = load i32, i32* %1
  %13 = icmp slt i32 %12, 10000
  %14 = zext i1 %13 to i32
  %15 = icmp ne i32 %14, 0
  br i1 %15, label %16, label %58
16:
  %17 = load i32, i32* %0
  %18 = load i32, i32* %0
  %19 = mul i32 %17, %18
  %20 = load i32, i32* %0
  %21 = mul i32 %19, %20
  %22 = load i32, i32* %0
  %23 = mul i32 %21, %22
  %24 = load i32, i32* %0
  %25 = mul i32 %23, %24
  %26 = load i32, i32* %0
  %27 = mul i32 %25, %26
  %28 = load i32, i32* %0
  %29 = mul i32 %27, %28
  %30 = load i32, i32* %0
  %31 = mul i32 %29, %30
  %32 = load i32, i32* %0
  %33 = mul i32 %31, %32
  %34 = load i32, i32* %0
  %35 = mul i32 %33, %34
  %36 = load i32, i32* %0
  %37 = sdiv i32 %35, %36
  %38 = load i32, i32* %0
  %39 = sdiv i32 %37, %38
  %40 = load i32, i32* %0
  %41 = sdiv i32 %39, %40
  %42 = load i32, i32* %0
  %43 = sdiv i32 %41, %42
  %44 = load i32, i32* %0
  %45 = sdiv i32 %43, %44
  %46 = load i32, i32* %0
  %47 = sdiv i32 %45, %46
  %48 = load i32, i32* %0
  %49 = sdiv i32 %47, %48
  %50 = load i32, i32* %0
  %51 = sdiv i32 %49, %50
  %52 = load i32, i32* %0
  %53 = sdiv i32 %51, %52
  %54 = load i32, i32* %0
  %55 = sdiv i32 %53, %54
  store i32 %55, i32* %2
  %56 = load i32, i32* %1
  %57 = add i32 %56, 1
  store i32 %57, i32* %1
  br label %11
58:
  %59 = load i32, i32* %0
  %60 = add i32 %59, 1
  store i32 %60, i32* %0
  br label %3
}
