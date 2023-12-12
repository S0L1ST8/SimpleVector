#pragma once

#include "array_ptr.h"

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <utility>

class ReserveProxyObj {
public:
    ReserveProxyObj() = delete;

    ReserveProxyObj(size_t new_capacity) : capacity(new_capacity) {}

    size_t GetCapacity() {
        return capacity;
    }

private:
    size_t capacity = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) : SimpleVector(size, std::move(Type{})) {}

    SimpleVector(size_t size, const Type& value) : items_(size), size_(size), capacity_(size) {
        std::fill(begin(), end(), value);
    }

    SimpleVector(size_t size, Type&& value) : items_(std::move(size)), size_(std::move(size)), capacity_(std::move(size)) {
        for (auto it = items_.Get(); it != items_.Get() + size; ++it ) {
            *it = std::move(value);
        }
    }

    SimpleVector(std::initializer_list<Type> init) : items_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::move(init.begin(), init.end(), items_.Get());
    }

    SimpleVector(const SimpleVector& other) {
        SimpleVector tmp(other.GetSize());
        std::copy(other.begin(), other.end(), tmp.begin());
        tmp.capacity_ = other.GetCapacity();
        swap(tmp);
    }

    SimpleVector(SimpleVector&& other) noexcept :
    items_(other.size_) {
        if (this != &other) {
            items_.swap(other.items_);
            size_ = std::move(other.size_);
            capacity_ = std::move(other.capacity_);
            other.Clear();
        }
    }

    SimpleVector(ReserveProxyObj obj) : items_(obj.GetCapacity()), size_(0), capacity_(obj.GetCapacity()) {}

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            auto copy_rhs(rhs);
            swap(copy_rhs);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            auto copy_rhs(rhs);
            swap(std::move(copy_rhs));
        }
        return *this;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& At(size_t index) {
        if (index >= size_)
            throw std::out_of_range("Invalid index");
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_)
            throw std::out_of_range("Invalid index");
        return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

   void Resize(size_t new_size) {
        if (new_size < size_) {
            size_ = new_size;
            return;
        }

        if (new_size > size_ && new_size < capacity_) {
            for(auto it = end(); it != begin() + new_size; ++it) {
                *it = std::move(Type{});
            }
            size_ = new_size;
            return;
        }

        ArrayPtr<Type> tmp(new_size);
        std::move(begin(), end(), &tmp[0]);
        items_.swap(tmp);
        size_ = new_size;
        capacity_ = new_size;
    }

    void PushBack(const Type& item) {
        Resize(size_ + 1);
        items_[size_ - 1] = item;
    }

    void PushBack(Type&& item) {
        Resize(size_ + 1);
        items_[size_ - 1] = std::move(item);
    }

    void PopBack() noexcept {
        if (!IsEmpty()) {
            --size_;
        }
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        int index = std::distance(cbegin(), pos);
        if (size_ == capacity_) {
            size_ != 0 ? ChangeCapacity(2 * size_) : ChangeCapacity(1);
        }
        auto it = begin() + index;
        std::copy_backward(it, end(), end() + 1);
        items_[index] = value;
        ++size_;
        return Iterator(begin() + index);
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        int index = std::distance(cbegin(), pos);
        if (size_ == capacity_) {
            size_ != 0 ? ChangeCapacity(2 * size_) : ChangeCapacity(1);
        }
        auto it = begin() + index;
        std::move_backward(it, end(), end() + 1);
        items_[index] = std::move(value);
        ++size_;
        return Iterator(begin() + index);
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos <= end());
        std::move(std::next(Iterator(pos)), end(), Iterator(pos));
        --size_;
        return Iterator(pos);
    }

    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        items_.swap(other.items_);
    }

    void swap(SimpleVector&& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        items_.swap(other.items_);
    }

    void Reserve(size_t new_capacity) {
        if (capacity_ < new_capacity) {
            ChangeCapacity(new_capacity);
        }
    }

    Iterator begin() noexcept {
        return &items_[0];
    }

    Iterator end() noexcept {
        return &items_[size_];
    }

    ConstIterator begin() const noexcept {
        return cbegin();
    }

    ConstIterator end() const noexcept {
        return cend();
    }

    ConstIterator cbegin() const noexcept {
        return &items_[0];
    }

    ConstIterator cend() const noexcept {
        return &items_[size_];
    }
private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;

    void ChangeCapacity(const size_t new_size) {
        SimpleVector new_items(new_size);
        size_t prev_size = size_;
        std::move(begin(), end(), new_items.begin());
        swap(std::move(new_items));
        size_ = prev_size;
    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
