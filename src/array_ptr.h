#pragma once

#include <algorithm>
#include <cassert>
#include <cstdlib>

using namespace std;

template <typename Type>
class ArrayPtr {
public:
    ArrayPtr() = default;

    explicit ArrayPtr(size_t size) {
        if (size) {
            try {
                raw_ptr_ = new Type[size](); // () инициализируют выделенный массив нулевыми значениями (для типов, поддерживающих значение по умолчанию)
            }
            catch (...) {
                throw;
            }
        }
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept : raw_ptr_(raw_ptr) {}

    ArrayPtr(const ArrayPtr&) = delete;
    ArrayPtr& operator=(const ArrayPtr&) = delete;

    ArrayPtr(ArrayPtr&& other) noexcept : raw_ptr_(std::move(other.raw_ptr_)) {}
    ArrayPtr& operator=(ArrayPtr&& rhs) noexcept {
        if (this != &rhs) {
            auto copy_rhs(rhs);
            swap(std::move(copy_rhs));
        }
        return *this;
    }

    Type* Release() noexcept {
        Type* ptr = raw_ptr_;
        raw_ptr_ = nullptr;
        return ptr;
    }

    Type& operator[](size_t index) noexcept {
        return raw_ptr_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return raw_ptr_[index];
    }

    explicit operator bool() const {
        return Get() ? 1 : 0;
    }

    Type* Get() const noexcept {
        return raw_ptr_;
    }

    void swap(ArrayPtr& other) noexcept {
        std::swap(raw_ptr_, other.raw_ptr_);
    }

    ~ArrayPtr() {
        delete [] raw_ptr_;
    }

private:
    Type* raw_ptr_ = nullptr;
};
