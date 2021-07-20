define i32 @main() {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca [10 x i32]
  %2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 0
  store i32 10, i32* %2
  %3 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 1
  %4 = load i32, i32* %2
  %5 = mul i32 %4, 2
  store i32 %5, i32* %3
  store i32 %5, i32* %0
  br label %6
6:
  %7 = load i32, i32* %0
  ret i32 %7
}
