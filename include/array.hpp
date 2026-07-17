/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#pragma once

#include <setalgorithms.hpp>
#include <containeralgorithms.hpp>
#include <iterator.hpp>

namespace Designar
{
    template <class Derived, class ArrayType, typename T, bool RET_CPY>
    class ArrayIterator : public RandomAccessIterator<Derived, T, RET_CPY>
    {
        friend class BasicIterator<Derived, T, RET_CPY>;
        friend class RandomAccessIterator<Derived, T, RET_CPY>;

    protected:
        ArrayType* array_ptr;
        nat_t curr;

        nat_t get_location() const
        {
            return curr;
        }

    public:
        ArrayIterator() : array_ptr(nullptr), curr(0)
        {
            // empty
        }

        ArrayIterator(const ArrayType& a)
            : array_ptr(&const_cast<ArrayType&>(a)), curr(0)
        {
            // empty
        }

        ArrayIterator(const ArrayType& a, nat_t c)
            : array_ptr(&const_cast<ArrayType&>(a)), curr(c)
        {
            // empty
        }

        ArrayIterator(const ArrayIterator& it)
            : array_ptr(it.array_ptr), curr(it.curr)
        {
            // empty
        }

        ArrayIterator(ArrayIterator&& it) : ArrayIterator()
        {
            swap(it);
        }

        ArrayIterator& operator=(const ArrayIterator& it)
        {
            if (this == &it)
            {
                return *this;
            }

            array_ptr = it.array_ptr;
            curr = it.curr;
            return *this;
        }

        ArrayIterator& operator=(ArrayIterator&& it)
        {
            swap(it);
            return *this;
        }

        void swap(ArrayIterator& it)
        {
            std::swap(array_ptr, it.array_ptr);
            std::swap(curr, it.curr);
        }

        bool has_current() const
        {
            return curr < array_ptr->size();
        }

        void next()
        {
            if (!has_current())
            {
                return;
            }

            ++curr;
        }

        void next_n(nat_t p)
        {
            curr = std::min(curr + p, array_ptr->size());
        }

        void prev()
        {
            if (curr == 0)
            {
                return;
            }

            --curr;
        }

        void prev_n(nat_t p)
        {
            if (p > curr)
            {
                curr = 0;
                return;
            }

            curr -= p;
        }

        void move_to(nat_t p)
        {
            curr = std::min(p, array_ptr->size());
        }

        void reset_first()
        {
            curr = 0;
        }

        void reset_last()
        {
            curr = array_ptr->size() - 1;
        }
    };

    /** `Derived` here is the actual leaf Iterator class (FixedArray<T>::
        Iterator, DynArray<T>::Iterator, ...), not TArrayIterator itself:
        RandomAccessIterator's operator+/operator-/operator[] construct and
        return a value of the CRTP type they were given, and if that type
        were TArrayIterator (this class) instead of the true leaf class,
        every one of those operators would silently return a
        TArrayIterator sliced from whatever leaf iterator was passed in —
        a different type than the original iterator despite one being
        derived from the other, which is exactly the kind of type
        mismatch `<algorithm>` functions like std::sort (which need every
        intermediate iterator expression to agree on one exact type) will
        not compile against. */
    template <class ArrayType, typename T, class Derived>
    class TArrayIterator : public ArrayIterator<Derived, ArrayType, T, false>
    {
        using Base = ArrayIterator<Derived, ArrayType, T, false>;
        using Base::Base;

    public:
        T& get_current()
        {
            if (!Base::has_current())
            {
                throw std::overflow_error("There is not current element");
            }

            return (*Base::array_ptr)[Base::curr];
        }

        const T& get_current() const
        {
            if (!Base::has_current())
            {
                throw std::overflow_error("There is not current element");
            }

            return (*Base::array_ptr)[Base::curr];
        }
    };

    template <typename T>
    class FixedArray
    {
        nat_t cap;
        T* array_ptr;

        void init(const T&);

        void copy(const FixedArray&);

    public:
        using ItemType = T;
        using KeyType = T;
        using DataType = T;
        using ValueType = T;
        using SizeType = nat_t;

        /** Returns the index of `item` within this array, assuming `item` is
            a reference to one of its elements (e.g. one previously returned by
            at()/operator[]/append()/insert()). Relies on typed pointer
            arithmetic (`&item - array_ptr`), which the language already
            expresses in units of `T`, so it is correct for any `T` regardless
            of `sizeof(T)`. */
        nat_t item_to_pos(T& item)
        {
            return static_cast<nat_t>(&item - array_ptr);
        }

        FixedArray() : cap(0), array_ptr(nullptr)
        {
            // empty
        }

        FixedArray(nat_t c) : cap(c), array_ptr(new T[cap])
        {
            // empty
        }

        FixedArray(nat_t c, const T& init_value) : FixedArray(c)
        {
            init(init_value);
        }

        FixedArray(const FixedArray& a) : FixedArray(a.cap)
        {
            copy(a);
        }

        FixedArray(FixedArray&& a) : FixedArray()
        {
            swap(a);
        }

        FixedArray(const std::initializer_list<T>&);

        ~FixedArray()
        {
            delete[] array_ptr;
        }

        FixedArray& operator=(const FixedArray& a)
        {
            if (this == &a)
            {
                return *this;
            }

            delete[] array_ptr;
            cap = a.cap;
            array_ptr = new T[cap];
            copy(a);
            return *this;
        }

        FixedArray& operator=(FixedArray&& a)
        {
            swap(a);
            return *this;
        }

        void swap(FixedArray& a)
        {
            std::swap(cap, a.cap);
            std::swap(array_ptr, a.array_ptr);
        }

        void resize(nat_t);

        nat_t get_capacity() const
        {
            return cap;
        }

        nat_t size() const
        {
            return get_capacity();
        }

        T& at(nat_t i)
        {
            if (i >= cap)
            {
                throw std::out_of_range("Index out of range");
            }

            return array_ptr[i];
        }

        const T& at(nat_t i) const
        {
            if (i >= cap)
            {
                throw std::out_of_range("Index out of range");
            }

            return array_ptr[i];
        }

        T& operator[](nat_t i)
        {
            return at(i);
        }

        const T& operator[](nat_t i) const
        {
            return at(i);
        }

        class Iterator : public TArrayIterator<FixedArray, T, Iterator>
        {
            using Base = TArrayIterator<FixedArray, T, Iterator>;
            using Base::Base;
        };

        Iterator begin()
        {
            return Iterator(*this);
        }

        Iterator begin() const
        {
            return Iterator(*this);
        }

        Iterator end()
        {
            return Iterator(*this, cap);
        }

        Iterator end() const
        {
            return Iterator(*this, cap);
        }
    };

    template <typename T>
    void FixedArray<T>::init(const T& init_value)
    {
        for (nat_t i = 0; i < cap; ++i)
        {
            array_ptr[i] = init_value;
        }
    }

    template <typename T>
    void FixedArray<T>::copy(const FixedArray& a)
    {
        // `if constexpr` (rather than a runtime `if`) so the memcpy branch is
        // only ever compiled for actually-POD T: with a plain runtime `if`,
        // both branches are type-checked for every T regardless of which one
        // executes, and GCC's -Wclass-memaccess correctly flags memcpy-ing a
        // non-POD class (e.g. one with a user-provided copy constructor) even
        // though that branch is dead code for such a T.
        if constexpr (std::is_pod<T>::value)
        {
            memcpy(array_ptr, a.array_ptr, sizeof(T) * cap);
        }
        else
        {
            for (nat_t i = 0; i < cap; ++i)
            {
                array_ptr[i] = a.array_ptr[i];
            }
        }
    }

    template <typename T>
    void FixedArray<T>::resize(nat_t c)
    {
        if (c == cap)
        {
            return;
        }

        T* new_array_ptr = new T[c];

        nat_t sz = std::min(c, cap);

        for (nat_t i = 0; i < sz; ++i)
        {
            new_array_ptr[i] = array_ptr[i];
        }

        delete[] array_ptr;
        cap = c;
        array_ptr = new_array_ptr;
    }

    template <typename T>
    FixedArray<T>::FixedArray(const std::initializer_list<T>& l)
        : FixedArray(l.size())
    {
        nat_t i = 0;

        for (const T& item : l)
        {
            array_ptr[i++] = item;
        }
    }

    template <typename T>
    class DynArray : private FixedArray<T>,
                     public ContainerAlgorithms<DynArray<T>, T>
    {
        using BaseArray = FixedArray<T>;

    public:
        using ItemType = T;
        using KeyType = T;
        using DataType = T;
        using ValueType = T;
        using SizeType = nat_t;

    private:
        static constexpr nat_t MIN_SIZE = 32;
        static constexpr real_t RESIZE_FACTOR = 0.4;

        nat_t num_items;

        void copy_array(const DynArray&);

        /** Grows capacity geometrically (by RESIZE_FACTOR) once the array is
            full. The naive `cap * (1 + RESIZE_FACTOR)` truncates back to `cap`
            itself for small capacities (e.g. cap in {0, 1, 2}), which would
            stall growth forever and make the very next append()/insert()
            throw std::out_of_range. Taking the max with `cap + 1` guarantees
            capacity always increases by at least one slot, while still
            growing geometrically (amortized O(1) append) once cap is large
            enough for the factor to matter. */
        void resize_up()
        {
            nat_t cap = BaseArray::get_capacity();

            if (num_items < cap)
            {
                return;
            }

            nat_t new_cap =
                std::max<nat_t>(cap + 1, nat_t(cap * (1 + RESIZE_FACTOR)));

            BaseArray::resize(new_cap);
        }

        void resize_down()
        {
            if (num_items > BaseArray::get_capacity() * RESIZE_FACTOR or
                BaseArray::get_capacity() == MIN_SIZE)
            {
                return;
            }

            assert(BaseArray::get_capacity() * (1 - RESIZE_FACTOR) > num_items);

            nat_t new_cap = std::max<real_t>(
                BaseArray::get_capacity() * (1 - RESIZE_FACTOR), MIN_SIZE);

            BaseArray::resize(new_cap);
        }

        void open_breach(nat_t);

        void close_breach(nat_t);

    public:
        DynArray(nat_t cap) : BaseArray(cap), num_items(0)
        {
            // empty
        }

        /** Constructs an array holding `n` elements, each initialized to
            `init_val`. `n` is a count of elements, not a capacity reservation:
            the array is left in exactly the state it would be in had it grown
            organically via append() until it held `n` items, including the
            one-step-ahead capacity growth append() relies on (see
            resize_up()). Without that immediate resize_up() call, the array
            would be exactly full with no spare capacity, and the very next
            append()/insert() would throw std::out_of_range instead of
            growing. */
        DynArray(nat_t n, const T& init_val)
            : BaseArray(n, init_val), num_items(n)
        {
            resize_up();
        }

        DynArray() : DynArray(MIN_SIZE)
        {
            // empty
        }

        DynArray(const DynArray& a)
            : BaseArray(a.get_capacity()), num_items(a.num_items)
        {
            copy_array(a);
        }

        DynArray(DynArray&& a) : BaseArray(), num_items(0)
        {
            swap(a);
        }

        DynArray(const std::initializer_list<T>&);

        void swap(DynArray& a)
        {
            BaseArray::swap(a);
            std::swap(num_items, a.num_items);
        }

        nat_t get_capacity() const
        {
            return BaseArray::get_capacity();
        }

        nat_t size() const
        {
            return num_items;
        }

        bool is_empty() const
        {
            return num_items == 0;
        }

        void clear()
        {
            num_items = 0;

            if (BaseArray::get_capacity() != MIN_SIZE)
            {
                DynArray new_array(MIN_SIZE);
                BaseArray::swap(new_array);
            }
        }

        T& get_first()
        {
            if (num_items == 0)
            {
                throw std::underflow_error("Array is empty");
            }

            return BaseArray::at(0);
        }

        const T& get_first() const
        {
            if (num_items == 0)
            {
                throw std::underflow_error("Array is empty");
            }

            return BaseArray::at(0);
        }

        T& get_last()
        {
            if (num_items == 0)
            {
                throw std::overflow_error("Array is empty");
            }

            return BaseArray::at(num_items - 1);
        }

        const T& get_last() const
        {
            if (num_items == 0)
            {
                throw std::overflow_error("Array is empty");
            }

            return BaseArray::at(num_items - 1);
        }

        T& insert(nat_t pos, const T& item)
        {
            if (pos > num_items)
            {
                throw std::out_of_range("Index is out of range");
            }

            // resize_up() must run *before* open_breach(): open_breach() itself
            // writes at index `num_items` (shifting the last element into the
            // slot right after it) whenever pos < num_items, which requires
            // that slot to already exist. Growing only after the write (the
            // original order) left a window where the array's capacity could
            // be exactly `num_items`, e.g. right after DynArray(n, init_val) or
            // DynArray(0) followed immediately by an insert/append, and
            // open_breach()/the write below would index one past the end.
            resize_up();

            open_breach(pos);

            BaseArray::at(pos) = item;

            ++num_items;

            return BaseArray::at(pos);
        }

        T& insert(nat_t pos, T&& item)
        {
            if (pos > num_items)
            {
                throw std::out_of_range("Index is out of range");
            }

            resize_up(); // see the const-ref overload above for why this must
                         // come before open_breach()

            open_breach(pos);

            BaseArray::at(pos) = std::move(item);

            ++num_items;

            return BaseArray::at(pos);
        }

        T& insert(const T& item)
        {
            return insert(0, item);
        }

        T& insert(T&& item)
        {
            return insert(0, std::forward<T>(item));
        }

        T& append(const T& item)
        {
            // resize_up() must run *before* the write below, not after: if
            // capacity is currently exactly num_items (e.g. right after
            // DynArray(n, init_val) or DynArray(0)), at(num_items) is already
            // one past the end, so growing after attempting the write is too
            // late.
            resize_up();
            BaseArray::at(num_items++) = item;
            return BaseArray::at(num_items - 1);
        }

        T& append(T&& item)
        {
            resize_up(); // see the const-ref overload above
            BaseArray::at(num_items++) = std::move(item);
            return BaseArray::at(num_items - 1);
        }

        T remove_pos(nat_t pos)
        {
            if (pos >= num_items)
            {
                throw std::out_of_range("Index is out of range");
            }

            T ret_val = std::move(BaseArray::at(pos));
            BaseArray::at(pos) = std::move(BaseArray::at(--num_items));
            return ret_val;
        }

        T remove(T& item)
        {
            nat_t i = BaseArray::item_to_pos(item);

            if (i >= num_items)
            {
                throw std::logic_error("Item does not belong to array");
            }

            return remove_pos(i);
        }

        T remove_pos_closing_breach(nat_t pos)
        {
            if (pos >= num_items)
            {
                throw std::out_of_range("Index is out of range");
            }

            T ret_val = std::move(BaseArray::at(pos));

            --num_items;

            close_breach(pos);

            resize_down();

            return ret_val;
            ;
        }

        T remove_closing_breach(T& item)
        {
            nat_t i = BaseArray::item_to_pos(item);

            if (i >= num_items)
            {
                throw std::logic_error("Item does not belong to array");
            }

            return remove_pos_closing_breach(i);
        }

        T remove_first()
        {
            if (num_items == 0)
            {
                throw std::underflow_error("Array is empty");
            }

            return remove_pos(0);
        }

        T remove_last()
        {
            T ret_val = std::move(BaseArray::at(--num_items));
            resize_down();
            return ret_val;
        }

        DynArray& operator=(const DynArray& a)
        {
            if (this == &a)
            {
                return *this;
            }

            (BaseArray&)* this = a;
            num_items = a.num_items;
            return *this;
        }

        DynArray& operator=(DynArray&& a)
        {
            swap(a);
            return *this;
        }

        T& at(nat_t i)
        {
            if (i >= num_items)
            {
                throw std::out_of_range("Index is out of range");
            }

            return BaseArray::at(i);
        }

        const T& at(nat_t i) const
        {
            if (i >= num_items)
            {
                throw std::out_of_range("Index is out of range");
            }

            return BaseArray::at(i);
        }

        T& operator[](nat_t i)
        {
            return at(i);
        }

        const T& operator[](nat_t i) const
        {
            return at(i);
        }

        class Iterator : public TArrayIterator<DynArray, T, Iterator>
        {
            using Base = TArrayIterator<DynArray, T, Iterator>;
            using Base::Base;

        public:
            T del()
            {
                if (!Base::has_current())
                {
                    throw std::logic_error("There is not current element");
                }

                return Base::array_ptr->remove_pos_closing_breach(Base::curr);
            }
        };

        Iterator begin()
        {
            return Iterator(*this);
        }

        Iterator begin() const
        {
            return Iterator(*this);
        }

        Iterator end()
        {
            return Iterator(*this, num_items);
        }

        Iterator end() const
        {
            return Iterator(*this, num_items);
        }
    };

    template <typename T>
    DynArray<T>::DynArray(const std::initializer_list<T>& l)
        : DynArray(l.size() + 1)
    {
        for (const T& item : l)
        {
            append(item);
        }
    }

    template <typename T>
    void DynArray<T>::copy_array(const DynArray& a)
    {
        for (nat_t i = 0; i < num_items; ++i)
        {
            BaseArray::at(i) = a.at(i);
        }
    }

    template <typename T>
    void DynArray<T>::open_breach(nat_t p)
    {
        for (nat_t i = num_items; i > p; --i)
        {
            BaseArray::at(i) = std::move(BaseArray::at(i - 1));
        }
    }

    template <typename T>
    void DynArray<T>::close_breach(nat_t p)
    {
        for (nat_t i = p; i < num_items; ++i)
        {
            BaseArray::at(i) = std::move(BaseArray::at(i + 1));
        }
    }

    template <typename T, nat_t N = 2>
    class MultiDimArray
    {
        static_assert(N >= 2, "N must be greater than 1");

        class Slice
        {
            nat_t sz;               // size of linear array
            FixedArray<nat_t> exts; // extents
            FixedArray<nat_t> strs; // strides

            void init();

            void swap(Slice& another)
            {
                std::swap(sz, another.sz);
                std::swap(exts, another.exts);
                std::swap(strs, another.strs);
            }

        public:
            Slice() : sz(1), exts(N), strs(N)
            {
                init();
            };

            Slice(const Slice& slice)
                : sz(slice.sz), exts(slice.exts), strs(slice.strs)
            {
                // empty
            }

            Slice(Slice&& slice) : Slice()
            {
                swap(slice);
            }

            Slice(const std::initializer_list<nat_t>& l)
                : sz(1), exts(l), strs(l.size())
            {
                init();
            }

            template <typename... Dims>
            Slice(Dims... dims)
                : sz(1), exts({nat_t(dims)...}), strs(sizeof...(Dims))
            {
                static_assert(sizeof...(Dims) == N, "");
                static_assert(AllAreConvertible<nat_t, Dims...>::value, "");
                init();
            }

            const FixedArray<nat_t>& extents() const
            {
                return exts;
            }

            const FixedArray<nat_t>& strides() const
            {
                return strs;
            }

            nat_t size() const
            {
                return sz;
            }

            template <typename... Dims>
            nat_t operator()(Dims...) const;

            Slice& operator=(const Slice& slice)
            {
                if (&slice == this)
                {
                    return *this;
                }

                sz = slice.sz;
                exts = slice.exts;
                strs = slice.strs;

                return *this;
            }

            Slice& operator=(Slice&& slice)
            {
                swap(slice);
                return *this;
            }
        };

    public:
        using ItemType = T;
        using KeyType = T;
        using DataType = T;
        using ValueType = T;
        using SizeType = nat_t;

    private:
        Slice slice;
        FixedArray<T> array;

        void swap(MultiDimArray& another)
        {
            std::swap(slice, another.slice);
            std::swap(array, another.array);
        }

    public:
        MultiDimArray() = default;

        template <typename... Dims>
        MultiDimArray(Dims... dims) : slice(dims...), array(slice.size())
        {
            // empty
        }

        MultiDimArray(const MultiDimArray& mda)
            : slice(mda.slice), array(mda.array)
        {
            // empty
        }

        MultiDimArray(MultiDimArray&& mda) : MultiDimArray()
        {
            swap(mda);
        }

        nat_t size(nat_t i) const
        {
            return slice.extents()[i];
        }

        template <typename... Dims>
        T& get(Dims... dims)
        {
            nat_t idx = slice(dims...);
            if (idx >= slice.size())
            {
                throw std::overflow_error("Invalid position");
            }
            return array[idx];
        }

        template <typename... Dims>
        const T& get(Dims... dims) const
        {
            nat_t idx = slice(dims...);
            if (idx >= slice.size())
            {
                throw std::overflow_error("Invalid position");
            }
            return array[idx];
        }

        template <typename... Dims>
        T& at(Dims... dims)
        {
            return get(dims...);
        }

        template <typename... Dims>
        const T& at(Dims... dims) const
        {
            return get(dims...);
        }

        template <typename... Dims>
        T& operator()(Dims... dims)
        {
            return get(dims...);
        }

        template <typename... Dims>
        const T& operator()(Dims... dims) const
        {
            return get(dims...);
        }

        MultiDimArray& operator=(const MultiDimArray& mda)
        {
            if (&mda == this)
            {
                return *this;
            }

            slice = mda.slice;
            array = mda.array;

            return *this;
        }

        MultiDimArray& operator=(MultiDimArray&& mda)
        {
            swap(mda);
            return *this;
        }

        class Iterator : public FixedArray<T>::Iterator
        {
            using Base = typename FixedArray<T>::Iterator;

        public:
            Iterator() : Base()
            {
                // empty
            }

            Iterator(const MultiDimArray& a) : Base(a.array)
            {
                // empty
            }

            Iterator(const MultiDimArray& a, nat_t c) : Base(a.array, c)
            {
                // empty
            }

            Iterator(const Iterator& itor) : Base(itor)
            {
                // empty
            }

            Iterator(Iterator&& itor) : Iterator()
            {
                Base::swap(itor);
            }
        };

        Iterator begin()
        {
            return Iterator(*this);
        }

        Iterator begin() const
        {
            return Iterator(*this);
        }

        Iterator end()
        {
            return Iterator(*this, slice.size());
        }

        Iterator end() const
        {
            return Iterator(*this, slice.size());
        }
    };

    template <typename T, nat_t N>
    void MultiDimArray<T, N>::Slice::init()
    {
        for (nat_t i = 0; i < N; ++i)
        {
            sz *= exts[i];
        }

        strs[N - 1] = 1;

        for (nat_t i = N - 1; i > 0; --i)
        {
            strs[i - 1] = strs[i] * exts[i];
        }
    }

    template <typename T, nat_t N>
    template <typename... Dims>
    nat_t MultiDimArray<T, N>::Slice::operator()(Dims... dims) const
    {
        static_assert(sizeof...(Dims) == N, "");
        static_assert(AllAreConvertible<nat_t, Dims...>::value, "");
        FixedArray<nat_t> arr{nat_t(dims)...};

        nat_t ret = 0;

        for (size_t i = 0; i < N; ++i)
        {
            ret += arr[i] * strs[i];
        }

        return ret;
    }
} // end namespace Designar
