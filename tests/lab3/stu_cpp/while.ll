define i32 @main() {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca i32
  store i32 10, i32* %1
  %2 = alloca i32
  store i32 0, i32* %2
  %3 = load i32, i32* %1
  %4 = load i32, i32* %2
  br label %loop
loop:
  %5 = load i32, i32* %2
  %6 = icmp slt i32 %5, 10
  br i1 %6, label %trueBB, label %falseBB
trueBB:
  %7 = load i32, i32* %2
  %8 = add i32 %7, 1
  store i32 %8, i32* %2
  %9 = load i32, i32* %1
  %10 = load i32, i32* %2
  %11 = add i32 %10, %9
  store i32 %11, i32* %1
  br label %loop
falseBB:
  %12 = load i32, i32* %1
  store i32 %12, i32* %0
  br label %13
13:
  %14 = load i32, i32* %0
  ret i32 %14
}
