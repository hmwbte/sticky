#pragma once
#include <type_traits>
#include <stdexcept>

#ifdef _HMW_DEBUG
  #include <stdio.h>
  #include <typeinfo>
  #define _HMW_DEBUG_FUNC(...) printf (__VA_ARGS__)
#else
  #define _HMW_DEBUG_FUNC(...)
#endif

namespace hmw {
  namespace sticky {

    class object;

    template <typename T>
    concept valid_for_pointer = std::is_base_of_v<object, T>;

    template <typename T> requires valid_for_pointer<T> class pointer;

    /**
     * Implementation details of sticky's tag system.  The code behind
     * this stuff is subject to change, which is why its hidden.
     */
    namespace _Impl {
      typedef struct tag tag;

      tag*         createTag        ()                  noexcept;
      void         setTagPointer    (tag* t, object* o) noexcept;
      object*      getTagPointer    (tag* t)            noexcept;
      unsigned int getTagGeneration (tag* t)            noexcept;
      unsigned int getTagRevision   (tag* t)            noexcept;
      void         releaseTag       (tag* t)            noexcept;

    }; // class _Impl

    /**
     * Handles tag creation and management.
     *
     * This class MUST be the polymorphic base of EVERY class you want
     * to use sticky::pointer with.
     */
    class object {

      template <typename T> requires valid_for_pointer<T> friend class pointer;

      private:
        _Impl::tag* tag;

        _Impl::tag* createOrGetTag () {
          if (tag == nullptr) {
            tag = _Impl::createTag ();
            _Impl::setTagPointer (tag, this);
            _HMW_DEBUG_FUNC ("[hmw::sticky::object::createOrGetTag] Spawned tag for sticky object\n");
          }
          return tag;
        };

      protected:

        // Default constructor; init tag & do nothing
        object () noexcept
         : tag(nullptr) {
          _HMW_DEBUG_FUNC ("[hmw::sticky::object::object] Created sticky object\n");
        };

        // Copy constructor; also do nothing since this is a separate object
        object (const object& obj) noexcept
         : tag(nullptr) {
          _HMW_DEBUG_FUNC ("[hmw::sticky::object::object] Copy-created sticky object\n");
        };

        // Copy assignment: ditto
        object& operator= (const object& obj) noexcept {
          _HMW_DEBUG_FUNC ("[hmw::sticky::object::operator=] Copy-assigned sticky object\n");
          return *this;
        };

        // Move constructor; used to transfer tags.  The moved object's tag is nullified,
        // since that object is typically deleted.
        object (object&& obj) noexcept
         : tag(obj.tag) {
          _HMW_DEBUG_FUNC ("[hmw::sticky::object::object] Move-constructed sticky object\n");
          obj.tag = nullptr;
          _Impl::setTagPointer (tag, this);
        };

        // Move assignment: ditto
        object& operator= (object&& obj) noexcept {
          // Don't allow move assign to self
          _HMW_DEBUG_FUNC ("[hmw::sticky::object::operator=] Move-assigned sticky object\n");
          if (obj.tag == tag) {
            _HMW_DEBUG_FUNC ("[hmw::sticky::object.operator=] Move-assigned self!\n");
            return *this;
          }
          tag = obj.tag;
          obj.tag = nullptr;
          _Impl::setTagPointer (tag, this);
          return *this;
        };

        // Destructor; virtual, because this class MUST be polymorphic
        virtual ~object () {
          _HMW_DEBUG_FUNC ("[hmw::sticky::object::~object] Deleted sticky object\n");
          _Impl::releaseTag (tag);
          tag = nullptr;
        };

    }; // class object


    /**
     * A "sticky" pointer to a sticky::object or subclass.  Because everything
     * sticky-related is virtual/polymorphic, the pointer type does not have
     * to be the exact type of the pointed-to object; it may be a base type.
     */
    template <typename T> requires valid_for_pointer<T>
    class pointer {

      template <typename U> requires valid_for_pointer<U>
      friend class pointer;

      private:
        _Impl::tag* tag;
        unsigned int generation;
        unsigned int revision;
        T* cache;

        // Refresh the cached pointer
        T* refresh () noexcept {
          object* ptr;

          // Fail if no tag is set
          if (tag == nullptr) {
            _HMW_DEBUG_FUNC ("[hmw::sticky::pointer::refresh] failed to refresh cached pointer; tag is null.\n");
            return cache = nullptr;
          }

          // Fail if tag generation doesn't match (tag was recycled, so object is gone)
          if (_Impl::getTagGeneration(tag) != generation) {
            _HMW_DEBUG_FUNC ("[hmw::sticky::pointer::refresh] failed to refresh cached pointer; tag is defunct.\n");
            return cache = nullptr;
          }

          // Tag is valid; make sure it points to a valid object, and update our
          // revision tracker to make the cache valid again.
          ptr = _Impl::getTagPointer(tag);
          revision = _Impl::getTagRevision(tag);

          if (ptr == nullptr) {
            _HMW_DEBUG_FUNC ("[hmw::sticky::pointer::refresh] failed to refresh cached pointer; tag points to null.\n");
            return cache = nullptr;
          }

          _HMW_DEBUG_FUNC ("[hmw::sticky::pointer::refresh] dyncasting object pointer to %s\n", typeid(T).name());

          // Cast to destination type
          return cache = dynamic_cast<T*>(ptr);
        };

        // Get the object pointer, cast to type T
        T* get () noexcept {
          if (tag == nullptr || generation != _Impl::getTagGeneration(tag)) {
            _HMW_DEBUG_FUNC ("[hmw::sticky::pointer::get] failed to get pointer; tag is null.\n");
            return nullptr;
          }
          if (revision != _Impl::getTagRevision(tag)) refresh ();
          return cache;
        };

      public:

        // Default constructor: null everything!
        pointer () noexcept
         : tag(nullptr), generation(~0U), revision(~0U), cache(nullptr) {};

        // Construct pointer from object: get/create the tag, and populate the generation. Revision is unset, so that a refresh is required
        explicit pointer (object& obj) noexcept
         : tag(obj.createOrGetTag()), generation(_Impl::getTagGeneration(tag)), revision(~0U), cache(nullptr) {};

        // Construct pointer from another pointer: discard the pointer's type, only keep the tag pointer and generation.
        // Cache is nulled & revision is discarded since we can't guarantee the pointer is valid for a new type.
        template <typename _discard>
        explicit pointer (pointer<_discard>& ptr) noexcept
         : tag(ptr.tag), generation(ptr.generation), revision (~0U), cache(nullptr) {};

        // Dereference operators
        T* operator-> () {
          return get ();
        };
        T& operator& () {
          T* tmp = get ();
          if (tmp == nullptr) throw std::runtime_error ("[hmw::sticky::pointer::operator&] Failed to dereference sticky pointer: pointer is null!");
          return *tmp;
        };

        // Operator bool, the most cursed thing in all of C++ because apparently `bool b = pointer::operator bool();` is legal!
        operator bool () {
          return get() != nullptr;
        };

        // Comparison operators, for convenience
        template<typename _discard>
        bool operator== (const pointer<_discard>& that) const {
          return tag == that.tag;
        };

        bool operator== (const T* that) {
          return get() == that;
        };

        // Inequality operators are auto-generated
    };

  }; // namespace sticky

}; // namespace hmw