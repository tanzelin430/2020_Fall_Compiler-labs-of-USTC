define i32 @main() {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca float
  store float 0x40163851e0000000, float* %1
  %2 = load float, float* %1
  %3 = fcmp ugt float %2,0x3ff0000000000000
  br i1 %3, label %trueBB, label %falseBB
trueBB:
  store i32 233, i32* %0
  br label %4
falseBB:
  store i32 0, i32* %0
  br label %4
4:
  %5 = load i32, i32* %0
  ret i32 %5
}
