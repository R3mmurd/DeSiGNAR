/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#pragma once

namespace Designar
{

    /** Generic Singleton.
     *
     *  This class was designed in order to be reused for any class which
     *  requires only one instance.
     *
     *  @param T Concrete singleton class.
     *
     *  Usage example:
     *
     *  \code{.cpp}
     *  class MySingletonClass : public Singleton<MySingletonClass>
     *  {
     *    friend class Singleton<MySingletonClass>;
     *
     *    // If you need default constructor, make it protected.
     *  protected:
     *    MySingletonClass() { }
     *
     *    // Any attributes or methods;
     *  };
     *
     *  MySingletonClass * ptr_instance = MySingletonClass::get_ptr_instance();
     *
     *  MySingletonClass & instance = MySingletonClass::get_instance();
     *  \endcode
     *
     *  @author: Alejandro J. Mujica
     */
    template <typename T>
    class Singleton
    {
    protected:
        Singleton()
        {
            // Empty
        }

        Singleton(const Singleton<T>&) = delete;

        Singleton& operator=(const Singleton<T>&) = delete;

    public:
        virtual ~Singleton()
        {
            // Empty
        }

        /** Get a reference to instance.
         *
         *  Lazily constructs the single `T` instance on first call and
         *  returns the same instance on every subsequent call.
         *
         *  Thread-safety: this relies on C++11 "magic statics" — the
         *  standard guarantees that initialization of a function-local
         *  `static` is thread-safe, so concurrent first calls from multiple
         *  threads cannot race and cannot observe a partially-constructed
         *  `T`. An earlier version implemented this lazily with a hand-rolled
         *  `if (instance == nullptr) instance = new T();` check on a
         *  `std::unique_ptr` member; that is a classic unsynchronized
         *  check-then-act race — two threads could both see `nullptr`, both
         *  construct a `T`, and the second assignment would silently replace
         *  (and leak) the first instance, leaving any raw pointer obtained
         *  from the first instance dangling. Delegating construction to a
         *  function-local static removes the race entirely instead of adding
         *  a mutex around it.
         *
         *  @return A reference to instance.
         */
        static T& get_instance()
        {
            static T instance;
            return instance;
        }

        /** Get a pointer to instance.
         *
         *  @return A pointer to instance.
         */
        static T* get_ptr_instance()
        {
            return &get_instance();
        }
    };

} // end namespace Designar
