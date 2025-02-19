#pragma once

// Provides conversions between Python tensor objects and at::Tensor.

#include <torch/csrc/python_headers.h>

#include <ATen/Device.h>
#include <c10/core/ScalarType.h>
#include <c10/core/Backend.h>

#include <memory>
#include <string>

struct THPDtype;
struct THPLayout;

namespace c10 {
struct Storage;
}

namespace torch {
// Register a PyTypeObject* with the given attributes
void registerStoragePyTypeObject(
    PyTypeObject *pytype, at::Backend backend, at::ScalarType scalarType);

void registerDtypeObject(THPDtype *dtype, at::ScalarType scalarType);
void registerLayoutObject(THPLayout *layout, at::Backend backend);

PyObject* createPyObject(const at::Storage& storage);
at::Storage createStorage(PyObject* obj);
bool isStorage(PyObject* obj);

THPDtype* getDtype(at::ScalarType scalarType);
THPLayout* getLayout(at::Backend backend);
}  // namespace torch
