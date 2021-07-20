; ModuleID = 'cminus'
source_filename = "testcase-3.cminus"

@global = global [25 x i32] zeroinitializer
declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  %op2 = alloca [25 x i32]
  br label %label3
label3:                                                ; preds = %label_entry, %label14
  %op117 = phi i32 [ 0, %label_entry ], [ %op17, %label14 ]
  %op5 = icmp slt i32 %op117, 25
  %op6 = zext i1 %op5 to i32
  %op7 = icmp ne i32 %op6, 0
  br i1 %op7, label %label8, label %label12
label8:                                                ; preds = %label3
  %op11 = icmp slt i32 %op117, 0
  br i1 %op11, label %label13, label %label14
label12:                                                ; preds = %label3
  br label %label18
label13:                                                ; preds = %label8
  call void @neg_idx_except()
  ret void
label14:                                                ; preds = %label8
  %op15 = getelementptr [25 x i32], [25 x i32]* %op2, i32 0, i32 %op117
  store i32 %op117, i32* %op15
  %op17 = add i32 %op117, 1
  br label %label3
label18:                                                ; preds = %label12, %label41
  %op118 = phi i32 [ %op122, %label41 ], [ undef, %label12 ]
  %op119 = phi i32 [ %op123, %label41 ], [ undef, %label12 ]
  %op120 = phi i32 [ 0, %label12 ], [ %op43, %label41 ]
  %op121 = phi i32 [ %op124, %label41 ], [ undef, %label12 ]
  %op20 = icmp slt i32 %op120, 5
  %op21 = zext i1 %op20 to i32
  %op22 = icmp ne i32 %op21, 0
  br i1 %op22, label %label23, label %label24
label23:                                                ; preds = %label18
  br label %label25
label24:                                                ; preds = %label18
  br label %label90
label25:                                                ; preds = %label23, %label63
  %op122 = phi i32 [ %op118, %label23 ], [ %op38, %label63 ]
  %op123 = phi i32 [ %op119, %label23 ], [ %op35, %label63 ]
  %op124 = phi i32 [ 0, %label23 ], [ %op65, %label63 ]
  %op27 = icmp slt i32 %op124, 5
  %op28 = zext i1 %op27 to i32
  %op29 = icmp ne i32 %op28, 0
  br i1 %op29, label %label30, label %label41
label30:                                                ; preds = %label25
  %op35 = add i32 %op120, %op124
  %op38 = mul i32 %op120, %op124
  %op40 = icmp slt i32 %op35, 0
  br i1 %op40, label %label44, label %label45
label41:                                                ; preds = %label25
  %op43 = add i32 1, %op120
  br label %label18
label44:                                                ; preds = %label30
  call void @neg_idx_except()
  ret void
label45:                                                ; preds = %label30
  %op46 = getelementptr [25 x i32], [25 x i32]* %op2, i32 0, i32 %op35
  %op47 = load i32, i32* %op46
  %op49 = icmp slt i32 %op38, 0
  br i1 %op49, label %label50, label %label51
label50:                                                ; preds = %label45
  call void @neg_idx_except()
  ret void
label51:                                                ; preds = %label45
  %op52 = getelementptr [25 x i32], [25 x i32]* %op2, i32 0, i32 %op38
  %op53 = load i32, i32* %op52
  %op54 = icmp sgt i32 %op47, %op53
  %op55 = zext i1 %op54 to i32
  %op56 = icmp ne i32 %op55, 0
  br i1 %op56, label %label57, label %label60
label57:                                                ; preds = %label51
  %op59 = icmp slt i32 %op35, 0
  br i1 %op59, label %label66, label %label67
label60:                                                ; preds = %label51
  %op62 = icmp slt i32 %op38, 0
  br i1 %op62, label %label78, label %label79
label63:                                                ; preds = %label76, %label88
  %op65 = add i32 %op124, 1
  br label %label25
label66:                                                ; preds = %label57
  call void @neg_idx_except()
  ret void
label67:                                                ; preds = %label57
  %op68 = getelementptr [25 x i32], [25 x i32]* %op2, i32 0, i32 %op35
  %op69 = load i32, i32* %op68
  %op71 = mul i32 %op120, 5
  %op73 = add i32 %op71, %op124
  %op74 = icmp slt i32 %op73, 0
  br i1 %op74, label %label75, label %label76
label75:                                                ; preds = %label67
  call void @neg_idx_except()
  ret void
label76:                                                ; preds = %label67
  %op77 = getelementptr [25 x i32], [25 x i32]* @global, i32 0, i32 %op73
  store i32 %op69, i32* %op77
  br label %label63
label78:                                                ; preds = %label60
  call void @neg_idx_except()
  ret void
label79:                                                ; preds = %label60
  %op80 = getelementptr [25 x i32], [25 x i32]* %op2, i32 0, i32 %op38
  %op81 = load i32, i32* %op80
  %op83 = mul i32 %op120, 5
  %op85 = add i32 %op83, %op124
  %op86 = icmp slt i32 %op85, 0
  br i1 %op86, label %label87, label %label88
label87:                                                ; preds = %label79
  call void @neg_idx_except()
  ret void
label88:                                                ; preds = %label79
  %op89 = getelementptr [25 x i32], [25 x i32]* @global, i32 0, i32 %op85
  store i32 %op81, i32* %op89
  br label %label63
label90:                                                ; preds = %label24, %label108
  %op125 = phi i32 [ 0, %label24 ], [ %op110, %label108 ]
  %op126 = phi i32 [ %op121, %label24 ], [ %op127, %label108 ]
  %op92 = icmp slt i32 %op125, 5
  %op93 = zext i1 %op92 to i32
  %op94 = icmp ne i32 %op93, 0
  br i1 %op94, label %label95, label %label96
label95:                                                ; preds = %label90
  br label %label97
label96:                                                ; preds = %label90
  ret void
label97:                                                ; preds = %label95, %label112
  %op127 = phi i32 [ 0, %label95 ], [ %op116, %label112 ]
  %op99 = icmp slt i32 %op127, 5
  %op100 = zext i1 %op99 to i32
  %op101 = icmp ne i32 %op100, 0
  br i1 %op101, label %label102, label %label108
label102:                                                ; preds = %label97
  %op104 = mul i32 %op125, 5
  %op106 = add i32 %op104, %op127
  %op107 = icmp slt i32 %op106, 0
  br i1 %op107, label %label111, label %label112
label108:                                                ; preds = %label97
  %op110 = add i32 1, %op125
  br label %label90
label111:                                                ; preds = %label102
  call void @neg_idx_except()
  ret void
label112:                                                ; preds = %label102
  %op113 = getelementptr [25 x i32], [25 x i32]* @global, i32 0, i32 %op106
  %op114 = load i32, i32* %op113
  call void @output(i32 %op114)
  %op116 = add i32 %op127, 1
  br label %label97
}
