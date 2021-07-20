; ModuleID = 'cminus'
source_filename = "array_minus.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label.entry:
  call void @neg_idx_except()
  %0 = alloca [0 x i32]
  ret void
}
