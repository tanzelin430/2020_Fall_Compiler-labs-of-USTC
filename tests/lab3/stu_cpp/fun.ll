define i32 @callee(i32 %0) {
entry:
  %1 = alloca i32
  store i32 0, i32* %1
  %2 = alloca i32
  store i32 %0, i32* %2
  %3 = load i32, i32* %2
  %4 = mul i32 %3, 2
  store i32 %4, i32* %1
  br label %5
5:
  %6 = load i32, i32* %1
  ret i32 %6
}
define i32 @main() {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = call i32 @callee(i32 110)
  ret i32 %1
}
