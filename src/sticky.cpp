#include <vector>
#include <deque>
#include "../include/sticky.hpp"

typedef hmw::sticky::_Impl::tag _tag_t;
typedef hmw::sticky::object _obj_t;

struct hmw::sticky::_Impl::tag {
  _obj_t* ptr = nullptr;
  unsigned int generation = 0;
  unsigned int revision = 0;
};

static std::deque<_tag_t>* _tag_pool = nullptr;
static std::vector<_tag_t*>* _free_list = nullptr;

static void _tag_pool_factory () {
  static std::deque<_tag_t> q;
  static std::vector<_tag_t*> l;

  _tag_pool = &q;
  _free_list = &l;
};

_tag_t* hmw::sticky::_Impl::createTag () noexcept {
  _tag_t* new_tag;

  // Create the tag pool once, if needed.
  if (_tag_pool == nullptr) _tag_pool_factory ();

  // Cycle through expired tags in the free list
  // until there's a viable one; tags expire when they are reset past
  // 32-bit limit, which should be VERY infrequent.  Memory cost is
  // ideally negligible.
  try {

  do {
    if (_free_list->size() > 0) {
      new_tag = _free_list->back ();
      _free_list->pop_back ();
    } else {
      _tag_pool->emplace_back();
      new_tag = &_tag_pool->back();
    }
  } while (new_tag->generation == ~0U);

  // Return null if we somehow ran out of memory
  } catch (std::exception) {
    return nullptr;
  }

  // Scrub old pointers, just in case
  new_tag->ptr = nullptr;
  new_tag->revision = 0;

  return new_tag;
};
void hmw::sticky::_Impl::setTagPointer (_tag_t* t, _obj_t* o) noexcept {
  if (t == nullptr) return;
  t->ptr = o;
  t->revision ++;
};
hmw::sticky::object* hmw::sticky::_Impl::getTagPointer (_tag_t* t) noexcept {
  if (t == nullptr) return nullptr;
  return t->ptr;
};
unsigned int hmw::sticky::_Impl::getTagGeneration (_tag_t* t) noexcept {
  if (t == nullptr) return ~0U;
  return t->generation;
};
unsigned int hmw::sticky::_Impl::getTagRevision (_tag_t* t) noexcept {
  if (t == nullptr) return ~0U;
  return t->revision;
};
void hmw::sticky::_Impl::releaseTag (_tag_t* t) noexcept {
  if (t == nullptr) return;
  t->generation ++;
  try {
    _free_list->push_back(t);

  // Somehow the free list is really full and we ran out of memory.
  // Realistically this will never happen.
  } catch (std::exception) {

  }
};