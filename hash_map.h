// Copyright [2020] <Mikhail Pogodin>

#include <list>
#include <stdexcept>
#include <vector>
#include <utility>
#include <cassert>

template<class Key, class Value, class Hash = std::hash<Key>>
struct HashMap {
 public:
    typedef std::pair<const Key, Value> pair;
    typedef std::list<pair> List;
    typedef typename List::iterator iterator;
    typedef typename List::const_iterator const_iterator;

 private:
    static constexpr float LOADFACTOR = 2;
    static constexpr float RESIZEFACTOR = 3;
    size_t md_ = 1;
    List lst_ = List();
    size_t cnt_ = 0;
    std::vector<iterator> it_;
    std::vector<bool> used_;
    Hash hasher_;
    // returns hash modulo table size
    size_t geth(const Key &k) const {
        return hasher_(k) % md_;
    }
    // adds an element to the table, assumes key doesn't exist
    void add(const pair &p) {
        ++cnt_;
        int h = geth(p.first);
        if (!used_[h]) {
            used_[h] = true;
            lst_.push_back(p);
            it_[h] = --lst_.end();
        } else {
            it_[h] = lst_.insert(it_[h], p);
            if (size() > md_ * LOADFACTOR) rehash(md_ * RESIZEFACTOR);
        }
    }
    template<class It>
    void addit(It begin, It end) {
        while (begin != end)
            add(*begin++);
    }
    // safely changes table size, be careful with overflow
    void rehash(size_t nmd) {
        if (nmd == 0) nmd = 1;
        std::vector<pair> cop(begin(), end());
        clear();
        md_ = nmd;
        it_ = std::vector<iterator>(md_);
        used_ = std::vector<bool>(md_);
        for (auto i : cop) add(i);
    }

 public:
    HashMap(Hash _ = Hash()) : hasher_(_) { rehash(md_); }
    template<class It>
    HashMap(It begin, It end, Hash _ = Hash()) : hasher_(_) {
        rehash(md_);
        addit(begin, end);
    }
    HashMap(std::initializer_list<pair> lst, Hash _ = Hash()) : hasher_(_) {
        rehash(md_);
        addit(lst.begin(), lst.end());
    }
    HashMap& operator=(const HashMap &h) {
        std::vector<pair> v(h.begin(), h.end());
        clear();
        hasher_ = h.hasher_;
        rehash(md_);
        for (const auto &i : v)
            add(i);
        return *this;
    }
    iterator begin() { return lst_.begin(); }
    iterator end() { return lst_.end(); }
    const_iterator begin() const { return lst_.begin(); }
    const_iterator end() const { return lst_.end(); }
    // returns an iterator to the element with Key k if exists, end() otherwise
    const_iterator find(const Key &k) const {
        size_t h = geth(k);
        if (!used_[h]) return end();
        auto c = it_[h];
        while (c != end() && geth(c->first) == h) {
            if (c->first == k) return c;
            ++c;
        }
        return end();
    }
    iterator find(const Key &k) {
        size_t h = geth(k);
        if (!used_[h]) return end();
        auto c = it_[h];
        while (c != end() && geth(c->first) == h) {
            if (c->first == k) return c;
            ++c;
        }
        return end();
    }
    // returns if an element with Key k exists
    bool count(const Key &k) const {
        return find(k) != end();
    }
    bool empty() const { return lst_.empty(); }
    size_t size() const { return cnt_; }
    // adds an element if it doesn't exist
    void insert(const pair &p) {
        if (!count(p.first))
            add(p);
    }
    void erase(iterator er) {
        if (it_[geth(er->first)] == er) {
            auto next = er;
            ++next;
            if (next == end() || geth(next->first) != geth(er->first))
                used_[geth(er->first)] = false;
            else
                it_[geth(er->first)] = next;
        }
        lst_.erase(er);
        if (size() < md_ / LOADFACTOR) rehash(md_ / RESIZEFACTOR);
    }
    // erases the element with Key k
    void erase(const Key &k) {
        if (count(k)) {
            --cnt_;
            erase(find(k));
        }
    }
    // returns the value associated to k if exists, creates default otherwise
    Value& operator[](const Key &k) {
        if (!count(k))
            add(pair(k, Value()));
        return find(k)->second;
    }
    void clear() {
        used_ = std::vector<bool>(md_);
        lst_.clear();
        cnt_ = 0;
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
