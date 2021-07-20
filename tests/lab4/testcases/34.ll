; ModuleID = 'cminus'
source_filename = "34.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define i32 @f(i32* %0, float %1) {
label.entry:
  %2 = alloca i32*
  %3 = alloca float
  store i32* %0, i32** %2
  store float %1, float* %3
  %4 = icmp sge i32 0, 0
  br i1 %4, label %5, label %8
5:
  %6 = load i32*, i32** %2
  %7 = getelementptr i32, i32* %6, i32 0
  br label %9
8:
  call void @neg_idx_except()
  br label %9
9:
  %10 = load float, float* %3
  %11 = fptosi float %10 to i32
  store i32 %11, i32* %7
  %12 = sitofp i32 1 to float
  store float %12, float* %3
  %13 = load float, float* %3
  %14 = fptosi float %13 to i32
  ret i32 %14
}
define void @main() {
label.entry:
  %0 = alloca [4 x i32]
  %1 = alloca i32
  %2 = alloca float
  %3 = icmp sge i32 0, 0
  br i1 %3, label %4, label %8
4:
  %5 = load [4 x i32], [4 x i32]* %0
  %6 = getelementptr [4 x i32], [4 x i32]* %0, i32 0, i32 0
  %7 = getelementptr i32, i32* %6, i32 0
  br label %9
8:
  call void @neg_idx_except()
  br label %9
9:
  %10 = fptosi float 0x4010000000000000 to i32
  store i32 %10, i32* %7
  %11 = icmp sge i32 1, 0
  br i1 %11, label %12, label %16
12:
  %13 = load [4 x i32], [4 x i32]* %0
  %14 = getelementptr [4 x i32], [4 x i32]* %0, i32 0, i32 0
  %15 = getelementptr i32, i32* %14, i32 1
  br label %17
16:
  call void @neg_idx_except()
  br label %17
17:
  store i32 2, i32* %15
  %18 = icmp sge i32 2, 0
  br i1 %18, label %19, label %23
19:
  %20 = load [4 x i32], [4 x i32]* %0
  %21 = getelementptr [4 x i32], [4 x i32]* %0, i32 0, i32 0
  %22 = getelementptr i32, i32* %21, i32 2
  br label %24
23:
  call void @neg_idx_except()
  br label %24
24:
  %25 = icmp sge i32 0, 0
  br i1 %25, label %26, label %30
26:
  %27 = load [4 x i32], [4 x i32]* %0
  %28 = getelementptr [4 x i32], [4 x i32]* %0, i32 0, i32 0
  %29 = getelementptr i32, i32* %28, i32 0
  br label %31
30:
  call void @neg_idx_except()
  br label %31
31:
  %32 = load i32, i32* %29
  %33 = icmp sge i32 1, 0
  br i1 %33, label %34, label %38
34:
  %35 = load [4 x i32], [4 x i32]* %0
  %36 = getelementptr [4 x i32], [4 x i32]* %0, i32 0, i32 0
  %37 = getelementptr i32, i32* %36, i32 1
  br label %39
38:
  call void @neg_idx_except()
  br label %39
39:
  %40 = load i32, i32* %37
  %41 = sdiv i32 %32, %40
  store i32 %41, i32* %22
  store float 0x4000000000000000, float* %2
  %42 = icmp sge i32 3, 0
  br i1 %42, label %43, label %47
43:
  %44 = load [4 x i32], [4 x i32]* %0
  %45 = getelementptr [4 x i32], [4 x i32]* %0, i32 0, i32 0
  %46 = getelementptr i32, i32* %45, i32 3
  br label %48
47:
  call void @neg_idx_except()
  br label %48
48:
  %49 = icmp sge i32 0, 0
  br i1 %49, label %50, label %54
50:
  %51 = load [4 x i32], [4 x i32]* %0
  %52 = getelementptr [4 x i32], [4 x i32]* %0, i32 0, i32 0
  %53 = getelementptr i32, i32* %52, i32 0
  br label %55
54:
  call void @neg_idx_except()
  br label %55
55:
  %56 = load i32, i32* %53
  %57 = load float, float* %2
  %58 = sitofp i32 %56 to float
  %59 = fmul float %58, %57
  %60 = fptosi float %59 to i32
  store i32 %60, i32* %46
  %61 = icmp sge i32 2, 0
  br i1 %61, label %62, label %66
62:
  %63 = load [4 x i32], [4 x i32]* %0
  %64 = getelementptr [4 x i32], [4 x i32]* %0, i32 0, i32 0
  %65 = getelementptr i32, i32* %64, i32 2
  br label %67
66:
  call void @neg_idx_except()
  br label %67
67:
  %68 = load i32, i32* %65
  store i32 %68, i32* %1
  %69 = getelementptr [4 x i32], [4 x i32]* %0, i32 0, i32 0
  %70 = load i32, i32* %1
  %71 = sitofp i32 %70 to float
  %72 = call i32 @f(i32* %69, float %71)
  store i32 %72, i32* %1
  %73 = load i32, i32* %1
  ret void
}
