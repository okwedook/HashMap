// Copyright [2020] <Mikhail Pogodin>

/*
* Hashmap implementation using a linked-list.
* All the operations are working in expected amortize O(1) time.
*
* Rehashing (modulo some number depending on the size) is used
* to avoid collision.
* All the values with the same rehashed hash are stored continuously
* in the linked list.
*
* Uses O(n) amortize time and O(n) memory
* (n is the maximum amount of elements during working process).
* All iterator functions work in real-time O(1).
*
* All the iterators are invalidated during
* insertion or deletion.
*/

#include <list>
#include <stdexcept>
#include <vector>
#include <utility>
#include <cassert>

template<class Key, class Value, class Hash = std::hash<Key>>
struct HashMap {
 public:
    typedef std::pair<const Key, Value> KeyValuePair;
    typedef std::list<KeyValuePair> List;
    typedef typename List::iterator iterator;
    typedef typename List::const_iterator const_iterator;

 private:
    static constexpr float LOAD_FACTOR = 2;
    static constexpr float RESIZE_FACTOR = 3;
    size_t hashmod_ = 1;
    List list_ = List();
    size_t size_ = 0;
    std::vector<iterator> first_occ_;
    std::vector<bool> used_;
    Hash hasher_;

    // Returns hash modulo table size.
    size_t geth(const Key &k) const {
        return hasher_(k) % hashmod_;
    }

    // Changes the size of the table when too many elements.
    void tryresize() {
        if (size_ > hashmod_ * LOAD_FACTOR) {
            rehash(hashmod_ * RESIZE_FACTOR);
        }
    }

    // Adds an element to the table, assumes key doesn't exist.
    void add(const KeyValuePair &p) {
        ++size_;
        int h = geth(p.first);
        if (!used_[h]) {
            used_[h] = true;
            list_.push_back(p);
            first_occ_[h] = --list_.end();
        } else {
            first_occ_[h] = list_.insert(first_occ_[h], p);
            tryresize();
        }
    }

    // Adds all elements between two iterators.
    template<class It>
    void addit(It begin, It end) {
        while (begin != end) add(*begin++);
    }

    // Safely changes table size, be careful with overflow.
    void rehash(size_t nmd) {
        if (nmd == 0) nmd = 1;
        std::vector<KeyValuePair> cop(begin(), end());
        clear();
        hashmod_ = nmd;
        first_occ_ = std::vector<iterator>(hashmod_);
        used_ = std::vector<bool>(hashmod_);
        for (auto i : cop) add(i);
    }

 public:
    HashMap(Hash _ = Hash()) : hasher_(_) { rehash(hashmod_); }

    template<class It>
    HashMap(It begin, It end, Hash _ = Hash()) : hasher_(_) {
        rehash(hashmod_);
        addit(begin, end);
    }

    HashMap(std::initializer_list<KeyValuePair> lst, Hash _ = Hash())
        : hasher_(_) {
        rehash(hashmod_);
        addit(lst.begin(), lst.end());
    }

    HashMap& operator=(const HashMap &h) {
        std::vector<KeyValuePair> v(h.begin(), h.end());
        clear();
        hasher_ = h.hasher_;
        rehash(hashmod_);
        for (const auto &i : v) add(i);
        return *this;
    }

    iterator begin() { return list_.begin(); }
    iterator end() { return list_.end(); }
    const_iterator begin() const { return list_.begin(); }
    const_iterator end() const { return list_.end(); }

    // Returns an iterator to the element with Key k if exists, end() otherwise.
    const_iterator find(const Key &k) const {
        size_t h = geth(k);
        if (!used_[h]) return end();
        auto c = first_occ_[h];
        while (c != end() && geth(c->first) == h) {
            if (c->first == k) return c;
            ++c;
        }
        return end();
    }

    iterator find(const Key &k) {
        size_t h = geth(k);
        if (!used_[h]) return end();
        auto c = first_occ_[h];
        while (c != end() && geth(c->first) == h) {
            if (c->first == k) return c;
            ++c;
        }
        return end();
    }

    // Returns if an element with Key k exists.
    bool count(const Key &k) const {
        return find(k) != end();
    }

    bool empty() const { return list_.empty(); }
    size_t size() const { return size_; }

    // Adds an element if it doesn't exist.
    void insert(const KeyValuePair &p) {
        if (!count(p.first)) add(p);
    }

    // Erases an element with the given iterator.
    void erase(iterator er) {
        if (first_occ_[geth(er->first)] == er) {
            auto next = er;
            ++next;
            if (next == end() || geth(next->first) != geth(er->first))
                used_[geth(er->first)] = false;
            else
                first_occ_[geth(er->first)] = next;
        }
        list_.erase(er);
    }

    // Erases the element with Key k.
    void erase(const Key &k) {
        if (count(k)) {
            --size_;
            erase(find(k));
        }
    }

    // Returns the value associated to k if exists, creates default otherwise.
    Value& operator[](const Key &k) {
        if (!count(k)) add(KeyValuePair(k, Value()));
        return find(k)->second;
    }

    void clear() {
        used_ = std::vector<bool>(hashmod_);
        list_.clear();
        size_ = 0;
    }

    Hash hash_function() const {
        return hasher_;
    }

    // const operator[]
    const Value& at(const Key &k) const {
        if (!count(k)) throw std::out_of_range("none");
        return find(k)->second;
    }
};
