; ModuleID = 'cminus'
source_filename = "testcase-1.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define i32 @gcd(i32 %arg0, i32 %arg1) {
label_entry:
  %op5 = icmp eq i32 %arg1, 0
  %op6 = zext i1 %op5 to i32
  %op7 = icmp ne i32 %op6, 0
  br i1 %op7, label %label8, label %label10
label8:                                                ; preds = %label_entry
  ret i32 %arg0
label10:                                                ; preds = %label_entry
  %op15 = sdiv i32 %arg0, %arg1
  %op17 = mul i32 %op15, %arg1
  %op18 = sub i32 %arg0, %op17
  %op19 = call i32 @gcd(i32 %arg1, i32 %op18)
  ret i32 %op19
label20:
  ret i32 0
}
define void @main() {
label_entry:
  %op5 = icmp slt i32 7, 8
  %op6 = zext i1 %op5 to i32
  %op7 = icmp ne i32 %op6, 0
  br i1 %op7, label %label8, label %label12
label8:                                                ; preds = %label_entry
  br label %label12
label12:                                                ; preds = %label_entry, %label8
  %op17 = phi i32 [ 7, %label8 ], [ undef, %label_entry ]
  %op18 = phi i32 [ 8, %label_entry ], [ 7, %label8 ]
  %op19 = phi i32 [ 7, %label_entry ], [ 8, %label8 ]
  %op15 = call i32 @gcd(i32 %op19, i32 %op18)
  call void @output(i32 %op15)
  ret void
}
