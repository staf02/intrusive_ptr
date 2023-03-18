#pragma once

#include <algorithm>

template <typename T>
struct intrusive_ref_counter;

template <typename T>
struct intrusive_ptr {
    using element_type = T;

    intrusive_ptr() noexcept = default;

    intrusive_ptr(T* p, bool add_ref = true) : ptr(p) {
        if (add_ref) {
            intrusive_ptr_add_ref(ptr);
        }
    }

    intrusive_ptr(intrusive_ptr const& r) : ptr(r.ptr) {
        if (ptr != nullptr) {
            intrusive_ptr_add_ref(ptr);
        }
    }

    template <class Y>
    intrusive_ptr(intrusive_ptr<Y> const& r) : ptr(r.ptr) {
        if (ptr != nullptr) {
            intrusive_ptr_add_ref(ptr);
        }
    }

    intrusive_ptr(intrusive_ptr&& r) : ptr(r.ptr) {
        r.ptr = nullptr;
    }

    template <class Y>
    intrusive_ptr(intrusive_ptr<Y>&& r) : ptr(r.ptr) {
        r.ptr = nullptr;
    }

    ~intrusive_ptr() {
        if (ptr != nullptr) {
            intrusive_ptr_release(ptr);
        }
    }

    intrusive_ptr& operator=(intrusive_ptr const& r) {
        intrusive_ptr(r).swap(*this);
        return *this;
    }

    template <class Y>
    intrusive_ptr& operator=(intrusive_ptr<Y> const& r) {
        intrusive_ptr(r).swap(*this);
        return *this;
    }

    intrusive_ptr& operator=(T* r) {
        intrusive_ptr(r).swap(*this);
        return *this;
    }

    intrusive_ptr& operator=(intrusive_ptr&& r) {
        intrusive_ptr(std::move(r)).swap(*this);
        return *this;
    }

    template <class Y>
    intrusive_ptr& operator=(intrusive_ptr<Y>&& r) {
        intrusive_ptr(std::move(r)).swap(*this);
        return *this;
    }

    void reset() {
        intrusive_ptr().swap(*this);
    }

    void reset(T* r) {
        intrusive_ptr(r).swap(*this);
    }

    void reset(T* r, bool add_ref) {
        intrusive_ptr(r, add_ref).swap(*this);
    }

    T& operator*() const noexcept {
        return *get();
    }

    T* operator->() const noexcept {
        return get();
    }

    T* get() const noexcept {
        return ptr;
    }

    T* detach() noexcept {
        T* res = ptr;
        ptr = nullptr;
        return res;
    }

    explicit operator bool() const noexcept {
        return get() != nullptr;
    }

    void swap(intrusive_ptr& b) noexcept {
        std::swap(ptr, b.ptr);
    }

    template <class E>
    friend struct intrusive_ptr;

private:
    T* ptr{ nullptr };
};

template <class T, class U>
bool operator==(intrusive_ptr<T> const& a, intrusive_ptr<U> const& b) noexcept {
    return a.get() == b.get();
}

template <class T, class U>
bool operator!=(intrusive_ptr<T> const& a, intrusive_ptr<U> const& b) noexcept {
    return a.get() != b.get();
}

template <class T, class U>
bool operator==(intrusive_ptr<T> const& a, U* b) noexcept {
    return a.get() == b;
}

template <class T, class U>
bool operator!=(intrusive_ptr<T> const& a, U* b) noexcept {
    return a.get() != b;
}

template <class T, class U>
bool operator==(T* a, intrusive_ptr<U> const& b) noexcept {
    return a == b.get();
}

template <class T, class U>
bool operator!=(T* a, intrusive_ptr<U> const& b) noexcept {
    return a != b.get();
}

template <class T>
bool operator<(intrusive_ptr<T> const& a, intrusive_ptr<T> const& b) noexcept {
    return a.get() < b.get();
}

template <class T>
void swap(intrusive_ptr<T>& a, intrusive_ptr<T>& b) noexcept {
    a.swap(b);
}

template <typename T>
struct intrusive_ref_counter {
    intrusive_ref_counter() noexcept = default;

    intrusive_ref_counter(const intrusive_ref_counter& v) noexcept
        : ref_count(0) {}

    intrusive_ref_counter& operator=(const intrusive_ref_counter& v) noexcept {
        return *this;
    }

    unsigned int use_count() const noexcept {
        return static_cast<unsigned int>(ref_count.load(std::memory_order_relaxed));
    }

    template <class Derived>
    friend void
        intrusive_ptr_add_ref(const intrusive_ref_counter<Derived>* p) noexcept;

    template <class Derived>
    friend void
        intrusive_ptr_release(const intrusive_ref_counter<Derived>* p) noexcept;

protected:
    mutable std::atomic<size_t> ref_count{ 0 };
    ~intrusive_ref_counter() = default;
};

template <class Derived>
void intrusive_ptr_add_ref(const intrusive_ref_counter<Derived>* p) noexcept {
    p->ref_count.fetch_add(1, std::memory_order_relaxed);
}

template <class Derived>
void intrusive_ptr_release(const intrusive_ref_counter<Derived>* p) noexcept {
    p->ref_count.fetch_sub(1, std::memory_order_acq_rel);
    if (p->use_count() == 0) {
        delete p;
    }
}
