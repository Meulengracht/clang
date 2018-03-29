// RUN: %clang_cc1 -fopenmp -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=nvptx64-nvidia-cuda -emit-llvm %s -o - | FileCheck %s --check-prefix HOST --check-prefix CHECK
// RUN: %clang_cc1 -fopenmp -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=nvptx64-nvidia-cuda -emit-llvm-bc %s -o %t-ppc-host.bc
// RUN: %clang_cc1 -fopenmp -x c++ -triple nvptx64-nvidia-cuda -fopenmp-targets=nvptx64-nvidia-cuda -emit-llvm %s -fopenmp-is-device -fopenmp-host-ir-file-path %t-ppc-host.bc -o - | FileCheck %s --check-prefix DEVICE --check-prefix CHECK
// RUN: %clang_cc1 -fopenmp -x c++ -triple nvptx64-nvidia-cuda -fopenmp-targets=nvptx64-nvidia-cuda -emit-llvm %s -fopenmp-is-device -fopenmp-host-ir-file-path %t-ppc-host.bc -emit-pch -o %t
// RUN: %clang_cc1 -fopenmp -x c++ -triple nvptx64-nvidia-cuda -fopenmp-targets=nvptx64-nvidia-cuda -emit-llvm %s -fopenmp-is-device -fopenmp-host-ir-file-path %t-ppc-host.bc -include-pch %t -o - | FileCheck %s --check-prefix DEVICE --check-prefix CHECK

// RUN: %clang_cc1 -fopenmp-simd -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=nvptx64-nvidia-cuda -emit-llvm-bc %s -o - | FileCheck %s --check-prefix SIMD-ONLY
// RUN: %clang_cc1 -fopenmp-simd -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=nvptx64-nvidia-cuda -emit-llvm-bc %s -o %t-ppc-host.bc
// RUN: %clang_cc1 -fopenmp-simd -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=nvptx64-nvidia-cuda -emit-llvm %s -fopenmp-is-device -fopenmp-host-ir-file-path %t-ppc-host.bc -o -| FileCheck %s --check-prefix SIMD-ONLY
// RUN: %clang_cc1 -fopenmp-simd -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=nvptx64-nvidia-cuda -emit-llvm %s -fopenmp-is-device -fopenmp-host-ir-file-path %t-ppc-host.bc -emit-pch -o %t
// RUN: %clang_cc1 -fopenmp-simd -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=nvptx64-nvidia-cuda -emit-llvm %s -fopenmp-is-device -fopenmp-host-ir-file-path %t-ppc-host.bc -include-pch %t -o - | FileCheck %s --check-prefix SIMD-ONLY

#ifndef HEADER
#define HEADER

// SIMD-ONLY-NOT: {{__kmpc|__tgt}}

// DEVICE-DAG: [[C_ADDR:@.+]] = internal global i32 0,
// DEVICE-DAG: [[CD_ADDR:@.+]] = global %struct.S zeroinitializer,
// HOST-DAG: [[C_ADDR:@.+]] = internal global i32 0,
// HOST-DAG: [[CD_ADDR:@.+]] = global %struct.S zeroinitializer,

#pragma omp declare target
int foo() { return 0; }
#pragma omp end declare target
int bar() { return 0; }
#pragma omp declare target (bar)
int baz() { return 0; }

#pragma omp declare target
int doo() { return 0; }
#pragma omp end declare target
int car() { return 0; }
#pragma omp declare target (bar)
int caz() { return 0; }

// DEVICE-DAG: define i32 [[FOO:@.*foo.*]]()
// DEVICE-DAG: define i32 [[BAR:@.*bar.*]]()
// DEVICE-DAG: define i32 [[BAZ:@.*baz.*]]()
// DEVICE-DAG: define i32 [[DOO:@.*doo.*]]()
// DEVICE-DAG: define i32 [[CAR:@.*car.*]]()
// DEVICE-DAG: define i32 [[CAZ:@.*caz.*]]()

static int c = foo() + bar() + baz();
#pragma omp declare target (c)
// HOST-DAG: @[[C_CTOR:__omp_offloading__.+_c_l44_ctor]] = private constant i8 0
// DEVICE-DAG: define internal void [[C_CTOR:@__omp_offloading__.+_c_l44_ctor]]()
// DEVICE-DAG: call i32 [[FOO]]()
// DEVICE-DAG: call i32 [[BAR]]()
// DEVICE-DAG: call i32 [[BAZ]]()
// DEVICE-DAG: ret void

struct S {
  int a;
  S() = default;
  S(int a) : a(a) {}
  ~S() { a = 0; }
};

#pragma omp declare target
S cd = doo() + car() + caz() + baz();
#pragma omp end declare target
// HOST-DAG: @[[CD_CTOR:__omp_offloading__.+_cd_l61_ctor]] = private constant i8 0
// DEVICE-DAG: define internal void [[CD_CTOR:@__omp_offloading__.+_cd_l61_ctor]]()
// DEVICE-DAG: call i32 [[DOO]]()
// DEVICE-DAG: call i32 [[CAR]]()
// DEVICE-DAG: call i32 [[CAZ]]()
// DEVICE-DAG: ret void

// HOST-DAG: @[[CD_DTOR:__omp_offloading__.+_cd_l61_dtor]] = private constant i8 0
// DEVICE-DAG: define internal void [[CD_DTOR:@__omp_offloading__.+_cd_l61_dtor]]()
// DEVICE-DAG: call void
// DEVICE-DAG: ret void

// HOST: @.omp_offloading.entry_name{{.*}} = internal unnamed_addr constant [{{[0-9]+}} x i8] c"[[C_CTOR]]\00"
// HOST: @.omp_offloading.entry.[[C_CTOR]] = constant %struct.__tgt_offload_entry { i8* @[[C_CTOR]], i8* getelementptr inbounds ([{{[0-9]+}} x i8], [{{[0-9]+}} x i8]* @.omp_offloading.entry_name{{.*}}, i32 0, i32 0), i64 0, i32 2, i32 0 }, section ".omp_offloading.entries", align 1
// HOST: @.omp_offloading.entry_name{{.*}}= internal unnamed_addr constant [{{[0-9]+}} x i8] c"[[CD_CTOR]]\00"
// HOST: @.omp_offloading.entry.[[CD_CTOR]] = constant %struct.__tgt_offload_entry { i8* @[[CD_CTOR]], i8* getelementptr inbounds ([{{[0-9]+}} x i8], [{{[0-9]+}} x i8]* @.omp_offloading.entry_name{{.*}}, i32 0, i32 0), i64 0, i32 2, i32 0 }, section ".omp_offloading.entries", align 1
// HOST: @.omp_offloading.entry_name{{.*}}= internal unnamed_addr constant [{{[0-9]+}} x i8] c"[[CD_DTOR]]\00"
// HOST: @.omp_offloading.entry.[[CD_DTOR]] = constant %struct.__tgt_offload_entry { i8* @[[CD_DTOR]], i8* getelementptr inbounds ([{{[0-9]+}} x i8], [{{[0-9]+}} x i8]* @.omp_offloading.entry_name{{.*}}, i32 0, i32 0), i64 0, i32 4, i32 0 }, section ".omp_offloading.entries", align 1
int maini1() {
  int a;
#pragma omp target map(tofrom : a)
  {
    a = c;
  }
  return 0;
}

// DEVICE: define void @__omp_offloading_{{.*}}_{{.*}}maini1{{.*}}_l[[@LINE-7]](i32* dereferenceable{{[^,]*}}
// DEVICE: [[C:%.+]] = load i32, i32* [[C_ADDR]],
// DEVICE: store i32 [[C]], i32* %

// HOST: define internal void @__omp_offloading_{{.*}}_{{.*}}maini1{{.*}}_l[[@LINE-11]](i32* dereferenceable{{.*}})
// HOST: [[C:%.*]] = load i32, i32* [[C_ADDR]],
// HOST: store i32 [[C]], i32* %

// DEVICE: !nvvm.annotations
// DEVICE-DAG: !{void ()* [[C_CTOR]], !"kernel", i32 1}
// DEVICE-DAG: !{void ()* [[CD_CTOR]], !"kernel", i32 1}
// DEVICE-DAG: !{void ()* [[CD_DTOR]], !"kernel", i32 1}

#endif // HEADER
