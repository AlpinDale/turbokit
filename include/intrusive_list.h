#pragma once

#include <cstddef>
#include <iterator>

namespace turbokit {

template <typename ElementType> struct ListNode {
  ElementType *previous = nullptr;
  ElementType *successor = nullptr;
};

template <typename ElementType, ListNode<ElementType> ElementType::*node_ptr>
struct LinkedList {
private:
  ElementType sentinel;
  static ElementType *&get_next(ElementType *current) noexcept {
    return (current->*node_ptr).successor;
  }
  static ElementType *&get_previous(ElementType *current) noexcept {
    return (current->*node_ptr).previous;
  }

public:
  using value_type = ElementType;
  using reference = ElementType &;
  using const_reference = const ElementType &;
  using difference_type = std::ptrdiff_t;
  using size_type = std::size_t;

  struct iterator {
  private:
    ElementType *current_ptr = nullptr;

  public:
    iterator() = default;
    explicit iterator(ElementType *current_ptr) : current_ptr(current_ptr) {}

    using difference_type = std::ptrdiff_t;
    using value_type = ElementType;
    using pointer = ElementType *;
    using reference = ElementType &;
    using iterator_category = std::bidirectional_iterator_tag;

    ElementType &operator*() const noexcept { return *current_ptr; }
    ElementType *operator->() const noexcept { return current_ptr; }
    iterator &operator++() noexcept {
      current_ptr = get_next(current_ptr);
      return *this;
    }
    iterator operator++(int) noexcept {
      iterator result = (*this);
      current_ptr = get_next(current_ptr);
      return result;
    }
    iterator &operator--() noexcept {
      current_ptr = get_previous(current_ptr);
      return *this;
    }
    iterator operator--(int) noexcept {
      iterator result = (*this);
      current_ptr = get_previous(current_ptr);
      return result;
    }
    bool operator==(iterator other) const noexcept {
      return current_ptr == other.current_ptr;
    }
    bool operator!=(iterator other) const noexcept {
      return current_ptr != other.current_ptr;
    }
  };

  LinkedList() noexcept {
    get_previous(&sentinel) = &sentinel;
    get_next(&sentinel) = &sentinel;
  }
  LinkedList(const LinkedList &) = delete;
  LinkedList(LinkedList &&other) { *this = std::move(other); }
  LinkedList &operator=(const LinkedList &) = delete;
  LinkedList &operator=(LinkedList &&other) {
    if (other.empty()) {
      clear();
    } else {
      auto sentinel_prev = get_previous(&other.sentinel);
      auto sentinel_next = get_next(&other.sentinel);
      get_previous(sentinel_next) = &sentinel;
      get_next(sentinel_prev) = &sentinel;
      get_previous(&sentinel) = sentinel_prev;
      get_next(&sentinel) = sentinel_next;
      other.clear();
    }
    return *this;
  }

  iterator begin() noexcept { return iterator(get_next(&sentinel)); }
  iterator end() noexcept { return iterator(&sentinel); }
  size_t size() = delete;
  constexpr size_t max_size() = delete;
  bool empty() const noexcept {
    return get_next((ElementType *)&sentinel) == &sentinel;
  }

  void clear() noexcept {
    get_previous(&sentinel) = &sentinel;
    get_next(&sentinel) = &sentinel;
  }
  static iterator insert(iterator position, ElementType &element) noexcept {
    ElementType *next_element = &*position;
    ElementType *prev_element = get_previous(&*position);
    get_previous(next_element) = &element;
    get_next(prev_element) = &element;
    get_next(&element) = next_element;
    get_previous(&element) = prev_element;
    return position;
  }
  static iterator erase(iterator position) noexcept {
    ElementType *next_element = get_next(&*position);
    ElementType *prev_element = get_previous(&*position);
    get_previous(next_element) = prev_element;
    get_next(prev_element) = next_element;
    get_previous(&*position) = nullptr;
    get_next(&*position) = nullptr;
    return iterator(next_element);
  }
  static void erase(ElementType &element) noexcept {
    erase(iterator(&element));
  }
  iterator push_front(ElementType &element) noexcept {
    return insert(begin(), element);
  }
  iterator push_back(ElementType &element) noexcept {
    return insert(end(), element);
  }
  void pop_front() noexcept { erase(begin()); }
  void pop_back() noexcept { erase(iterator(get_previous(&sentinel))); }
  ElementType &front() noexcept { return *get_next(&sentinel); }
  ElementType &back() noexcept { return *get_previous(&sentinel); }
};

template <typename T> using IntrusiveListLink = ListNode<T>;

template <typename T, IntrusiveListLink<T> T::*link>
using IntrusiveList = LinkedList<T, link>;

} // namespace turbokit