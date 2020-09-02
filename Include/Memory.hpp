/*
 * Copyright (c) 2020 Hani Ammar
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Common.hpp"

#include <malloc.h>
#include <new>

#ifndef QENTEM_MEMORY_H_
#define QENTEM_MEMORY_H_

namespace Qentem {
namespace Memory {

inline static void SetToZero(void *ptr, ULong size) noexcept {
#ifdef QENTEM_SIMD_ENABLED_
    const ULong m_size    = (size >> QMM_SHIFTSIZE_);
    const ULong remaining = (size ^ (m_size << QMM_SHIFTSIZE_));

    if (m_size != 0) {
        QMM_VAR_ *      m_ptr  = static_cast<QMM_VAR_ *>(ptr);
        const QMM_VAR_ *end    = (m_ptr + m_size);
        const QMM_VAR_  m_zero = QMM_SETZERO_();

        do {
            QMM_STOREU_(m_ptr, m_zero);
            ++m_ptr;
        } while (m_ptr != end);
    }

    if (remaining != 0) {
        char *      des = (static_cast<char *>(ptr) + (size - remaining));
        const char *end = (des + remaining);

        do {
            *des = 0;
            ++des;
        } while (des != end);
    }
#else
    char *      des = static_cast<char *>(ptr);
    const char *end = (des + size);

    while (des != end) {
        *des = 0;
        ++des;
    }
#endif
}

static void Copy(void *to, const void *form, ULong size) noexcept {
#ifdef QENTEM_SIMD_ENABLED_
    const ULong m_size    = (size >> QMM_SHIFTSIZE_);
    const ULong remaining = (size ^ (m_size << QMM_SHIFTSIZE_));

    if (m_size != 0) {
        QMM_VAR_ *      m_to   = static_cast<QMM_VAR_ *>(to);
        const QMM_VAR_ *m_form = static_cast<const QMM_VAR_ *>(form);
        const QMM_VAR_ *end    = (m_form + m_size);

        do {
            QMM_STOREU_(m_to, QMM_LOAD_(m_form));
            ++m_form;
            ++m_to;
        } while (m_form != end);
    }

    if (remaining != 0) {
        const ULong start = (size - remaining);
        const char *src   = static_cast<const char *>(form) + start;
        const char *end   = (src + remaining);

        char *des = static_cast<char *>(to) + start;

        do {
            *des = *src;
            ++des;
            ++src;
        } while (src != end);
    }
#else
    const char *src = static_cast<const char *>(form);
    const char *end = (src + size);
    char *      des = static_cast<char *>(to);

    while (src != end) {
        *des = *src;
        ++des;
        ++src;
    }
#endif
}

inline static bool Compare(const void *left, const void *right,
                           ULong length) noexcept {
    ULong offset = 0;
    if ((left != nullptr) && (right != nullptr) && (offset != length)) {
#ifdef QENTEM_SIMD_ENABLED_
        const QMM_VAR_ *m_left  = static_cast<const QMM_VAR_ *>(left);
        const QMM_VAR_ *m_right = static_cast<const QMM_VAR_ *>(right);

        do {
            QMM_Number_T bits =
                QMM_COMPARE_8_MASK_(QMM_LOAD_(m_left), QMM_LOAD_(m_right));

            if (bits == QMM_MAX_NUMBER_) {
                offset += QMM_SIZE_;

                if (offset >= length) {
                    return true;
                }
            } else {
                bits ^= QMM_MAX_NUMBER_;
                return ((Q_CTZL(bits) + offset) >= length);
            }

            ++m_left;
            ++m_right;
        } while (offset < length);
#else
        const char *c_left  = static_cast<const char *>(left);
        const char *c_right = static_cast<const char *>(right);

        do {
            if (c_left[offset] != c_right[offset]) {
                break;
            }

            ++offset;
        } while (offset != length);
#endif
    }
    return (offset == length);
}

} // namespace Memory

class HAllocator {
  public:
    template <typename Type_>
    inline static Type_ *Allocate(ULong size) {
        return static_cast<Type_ *>(malloc(size * sizeof(Type_)));
    }

    template <typename Type_>
    inline static Type_ *Allocate(Type_ &&value) {
        Type_ *ptr = Allocate<Type_>(1);
        new (ptr) Type_(static_cast<Type_ &&>(value));
        return ptr;
    }

    // template <typename Type_>
    // static Type_ *Allocate(ULong size, Type_ &&value) {
    //     Type_ *ptr = Allocate<Type_>(size);

    //     ULong i = 0;
    //     while (i != size) {
    //         *(ptr + i) = value;
    //         ++i;
    //     }

    //     return ptr;
    // }

    template <typename Type_>
    inline static Type_ *AllocateClear(ULong size) noexcept {
        const ULong c_size = (size * sizeof(Type_));
        void *      vptr   = malloc(c_size);
        Memory::SetToZero(vptr, c_size);

        return static_cast<Type_ *>(vptr);
    }

    template <typename Type_>
    inline static Type_ *AllocatePointers(ULong size) noexcept {
        const ULong c_size = (size * sizeof(void *));
        void *      vptr   = malloc(c_size);
        Memory::SetToZero(vptr, c_size);

        return static_cast<Type_ *>(vptr);
    }

    // template <typename Type_>
    // inline static Type_ *AllocateInit() {
    //     Type_ *ptr = Allocate<Type_>(1);
    //     new (ptr) Type_();
    //     return ptr;
    // }

    // template <typename Type_>
    // static Type_ *AllocateInit(ULong size) {
    //     Type_ *ptr = Allocate<Type_>(size);

    //     ULong i = 0;
    //     while (i != size) {
    //         new (ptr + i) Type_();
    //         ++i;
    //     }

    //     return ptr;
    // }

    template <typename Type_>
    inline static void Construct(Type_ *ptr, Type_ &&value) noexcept {
        new (ptr) Type_(static_cast<Type_ &&>(value));
    }

    template <typename Type_>
    inline static void Construct(Type_ *start, const Type_ *end,
                                 Type_ &&value) noexcept {
        while (start != end) {
            new (start) Type_(static_cast<Type_ &&>(value));
            ++start;
        }
    }

    template <typename Type_>
    inline static void Destruct(Type_ *ptr) noexcept {
        ptr->~Type_();
    }

    template <typename Type_>
    inline static void Destruct(Type_ *start, const Type_ *end) noexcept {
        while (start != end) {
            start->~Type_();
            ++start;
        }
    }

    inline static void Deallocate(void *ptr) noexcept {
        if (ptr != nullptr) {
            free(ptr);
        }
    }
};

} // namespace Qentem

#endif
