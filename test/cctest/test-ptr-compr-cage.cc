// Copyright 2021 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/common/globals.h"
#include "src/execution/isolate-inl.h"
#include "src/heap/heap-inl.h"
#include "test/cctest/cctest.h"

#ifdef V8_COMPRESS_POINTERS

namespace v8 {
namespace internal {

UNINITIALIZED_TEST(PtrComprCageAndIsolateRoot) {
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = CcTest::array_buffer_allocator();

  v8::Isolate* isolate1 = v8::Isolate::New(create_params);
  Isolate* i_isolate1 = reinterpret_cast<Isolate*>(isolate1);
  v8::Isolate* isolate2 = v8::Isolate::New(create_params);
  Isolate* i_isolate2 = reinterpret_cast<Isolate*>(isolate2);

#ifdef V8_COMPRESS_POINTERS_IN_ISOLATE_CAGE
  CHECK_EQ(i_isolate1->isolate_root(), i_isolate1->cage_base());
  CHECK_EQ(i_isolate2->isolate_root(), i_isolate2->cage_base());
  CHECK_NE(i_isolate1->cage_base(), i_isolate2->cage_base());
#endif

#ifdef V8_COMPRESS_POINTERS_IN_SHARED_CAGE
  CHECK_NE(i_isolate1->isolate_root(), i_isolate1->cage_base());
  CHECK_NE(i_isolate2->isolate_root(), i_isolate2->cage_base());
  CHECK_NE(i_isolate1->isolate_root(), i_isolate2->isolate_root());
  CHECK_EQ(i_isolate1->cage_base(), i_isolate2->cage_base());
#endif

  isolate1->Dispose();
  isolate2->Dispose();
}

UNINITIALIZED_TEST(PtrComprCageCodeRange) {
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = CcTest::array_buffer_allocator();

  v8::Isolate* isolate = v8::Isolate::New(create_params);
  Isolate* i_isolate = reinterpret_cast<Isolate*>(isolate);

  VirtualMemoryCage* cage = i_isolate->GetPtrComprCage();
  if (i_isolate->RequiresCodeRange()) {
    CHECK(!i_isolate->heap()->code_region().is_empty());
    CHECK(cage->reservation()->InVM(i_isolate->heap()->code_region().begin(),
                                    i_isolate->heap()->code_region().size()));
  }

  isolate->Dispose();
}

#ifdef V8_COMPRESS_POINTERS_IN_SHARED_CAGE
UNINITIALIZED_TEST(SharedPtrComprCage) {
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = CcTest::array_buffer_allocator();

  v8::Isolate* isolate1 = v8::Isolate::New(create_params);
  Isolate* i_isolate1 = reinterpret_cast<Isolate*>(isolate1);
  v8::Isolate* isolate2 = v8::Isolate::New(create_params);
  Isolate* i_isolate2 = reinterpret_cast<Isolate*>(isolate2);

  Factory* factory1 = i_isolate1->factory();
  Factory* factory2 = i_isolate2->factory();

  {
    HandleScope scope1(i_isolate1);
    HandleScope scope2(i_isolate2);

    Handle<FixedArray> isolate1_object = factory1->NewFixedArray(100);
    Handle<FixedArray> isolate2_object = factory2->NewFixedArray(100);

    CHECK_EQ(GetPtrComprCageBase(*isolate1_object),
             GetPtrComprCageBase(*isolate2_object));
  }

  isolate1->Dispose();
  isolate2->Dispose();
}

UNINITIALIZED_TEST(SharedPtrComprCageCodeRange) {
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = CcTest::array_buffer_allocator();

  v8::Isolate* isolate1 = v8::Isolate::New(create_params);
  Isolate* i_isolate1 = reinterpret_cast<Isolate*>(isolate1);
  v8::Isolate* isolate2 = v8::Isolate::New(create_params);
  Isolate* i_isolate2 = reinterpret_cast<Isolate*>(isolate2);

  if (i_isolate1->RequiresCodeRange() || i_isolate2->RequiresCodeRange()) {
    CHECK_EQ(i_isolate1->heap()->code_region(),
             i_isolate2->heap()->code_region());
  }

  isolate1->Dispose();
  isolate2->Dispose();
}
#endif  // V8_COMPRESS_POINTERS_IN_SHARED_CAGE

}  // namespace internal
}  // namespace v8

#endif  // V8_COMPRESS_POINTERS
