#pragma once

#include <type_traits>
#include <iterator>
#include <cstddef>
#include <cassert>

template <typename T> class ListNode;
template <typename T> class List;

template <typename T, bool IsReverse, bool IsConst>
class ListIterator {
private:
    ListNode<T> *NodePtr = nullptr;

public:
    using value_type = T;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using difference_type = ptrdiff_t;

    ListIterator() = default;
    ListIterator(ListNode<T> *Node): NodePtr(Node) {}
    ListIterator(const ListNode<T> *Node)
        : NodePtr(const_cast<ListNode<T> *>(Node)) { }

    /// Get the underlying Node
    ListNode<T> *getNodePtr() { return NodePtr; } 

    /// Allowing construct a const iterator from a nonconst iterator.
    template <bool RHSIsConst>
    ListIterator(const ListIterator<T, IsReverse, RHSIsConst> &RHS,
                 std::enable_if_t<IsConst || !RHSIsConst, void *> = nullptr)
        : NodePtr(RHS.Node) {}

    /// Allowing assign a nonconst iterator to a const iterator.
    template <bool RHSIsConst>
    std::enable_if_t<IsConst || !RHSIsConst, ListIterator &>
    operator=(const ListIterator<T, IsReverse, RHSIsConst> &RHS) {
        NodePtr = RHS.NodePtr;
        return *this;
    }

    ListIterator &operator++() {
        NodePtr = IsReverse ? NodePtr->getPrevNode() : NodePtr->getNextNode();
        return *this;
    }

    ListIterator &operator--() {
        NodePtr = IsReverse ? NodePtr->getNextNode() : NodePtr->getPrevNode();
        return *this;
    }
    ListIterator operator++(int) {
        ListIterator tmp = *this;
        ++*this;
        return tmp; 
    }

    ListIterator operator--(int) {
        ListIterator tmp = *this;
        --*this;
        return tmp;
    }

    // Accessors.
    reference operator*() { return *NodePtr->getValuePtr(); }
    const_reference operator*() const { return operator*(); }
    pointer operator->() { return &operator*(); }

    friend bool operator==(const ListIterator &LHS, const ListIterator &RHS) {
        return LHS.NodePtr == RHS.NodePtr;
    }

    friend bool operator!=(const ListIterator &LHS, const ListIterator &RHS) {
        return LHS.NodePtr != RHS.NodePtr;
    }

    bool isNull() const { return *this == nullptr; }
};


template <typename T> class ListNode {
public:
    using value_type = T;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;

    // Sential list node should have public destructor.
    // workaround for no ListNodeImpl interface.
    friend class List<T>;

protected:
    ListNode *Prev = nullptr;
    ListNode *Next = nullptr;
    virtual ~ListNode() = default;

public:
    pointer getValuePtr() { return static_cast<pointer>(this); }
    const_pointer getValuePtr() const {
        return static_cast<const_pointer>(this);
    }

    pointer getNextNode() { return Next->getValuePtr(); }
    const_pointer getNextNode() const { return Next->getValuePtr(); }

    pointer getPrevNode() { return Prev->getValuePtr(); }
    const_pointer getPrevNode() const { return Prev->getValuePtr(); }

protected:
    /// Private insertion & removal interface
    void insertBefore(pointer Node) {
        if (Node) {
            if (Prev) {
                Prev->Next = Node;
            }
            Node->Next = this;
            Node->Prev = Prev;
            Prev = Node;
        }
    }

    void insertAfter(pointer Node) {
        if (Node) {
            if (Next) {
                Next->Prev = Node;
            }
            Node->Next = Next;
            Node->Prev = this;
            Next = Node;
        }
    }

    void removeFromList() {
        if (Prev) {
            Prev->Next = Next;
        }
        if (Next) {
            Next->Prev = Prev;
        }
        Prev = Next = nullptr;
    }

    void removeAndDispose() {
        removeFromList();
        delete this;
    }
};


template <typename T>
class List {
public:
    using value_type = T;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;

    using iterator = ListIterator<T, false, false>;
    using const_iterator = ListIterator<T, false, true>;
    using reverse_iterator = ListIterator<T, true, false>;
    using const_reverse_iterator = ListIterator<T, true, true>;

    using size_type = size_t;
    using difference_type = ptrdiff_t;

protected:
    ListNode<T> Sentinel;

public:
    List() = default;
    ~List() = default;

    List(const List &) = delete;
    List &operator=(const List &) = delete;

    iterator begin() { return ++iterator(&Sentinel); }
    const_iterator cbegin() const { return ++const_iterator(&Sentinel); }
    iterator end() { return iterator(&Sentinel); }
    const_iterator cend() const { return const_iterator(&Sentinel); }
    reverse_iterator rbegin() { return ++reverse_iterator(&Sentinel); }
    const_reverse_iterator crbegin() const {
        return ++const_reverse_iterator(&Sentinel);
    }
    reverse_iterator rend() { return reverse_iterator(&Sentinel); }
    const_reverse_iterator crend() const {
        return const_reverse_iterator(&Sentinel);
    }

    /// Calculate the size in linear time.
    [[nodiscard]] size_type size() const {
        return std::distance(begin(), end());
    }

    [[nodiscard]] bool empty() const {
        return Sentinel.getNextNode() == nullptr &&
               Sentinel.getPrevNode() == nullptr;
    }


    reference front() { return *begin(); }
    const_reference front() const { return *cbegin(); }
    reference back() { return *rbegin(); }
    const_reference back() const { return *crbegin(); }

    iterator insert(iterator pos, pointer New) {
        New->insertBefore(pos);
        return iterator(New);
    }

    iterator insertAfter(iterator pos, pointer New) {
        if (empty()) {
            return insert(begin(), New);
        } else {
            return insert(++pos, New);
        }
    }
    /// Remove a node from list.
    pointer remove(iterator &IT) {
        pointer Node = &*IT++;
        Node->removeFromList();
        return Node;
    }

    pointer remove(const iterator &IT) {
        iterator MutIT = IT;
        return remove(MutIT);
    }

    pointer remove(pointer IT) { return remove(iterator(IT)); }
    pointer remove(reference IT) { return remove(iterator(IT)); }

    /// Remove a node from list and delete it, return the iterator forwarded.
    iterator erase(iterator IT) {
        iterator Node = IT++;
        Node->removeAndDispose();
        return IT;
    }

    iterator erase(pointer IT) { return erase(iterator(IT)); }
    iterator erase(reference IT) { return erase(iterator(IT)); }

    /// Insert a node at front.
    void push_front(reference Node) { insert(begin(), Node); }
    /// Insert a node at back.
    void push_back(reference Node) { insert(end(), Node); }
    /// Remove the node at front and delete it.
    void pop_front() {
        assert(!empty() && "pop_front() on a empty list!");
        erase(begin());
    }
    /// Remove the node at back and delete it.
    void pop_back() {
        assert(!empty() && "pop_back() on a empty list!");
        iterator t = end();
        erase(--t);
    }

};