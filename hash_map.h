// Copyright [2020] <Mikhail Pogodin>

#include <list>
#include <stdexcept>
#include <vector>
#include <utility>

template<class Key, class Value, class Hash = std::hash<Key>>
struct HashMap {
 public:
    typedef std::pair<const Key, Value> pair;
    typedef std::list<pair> List;
    typedef typename List::iterator iterator;
    typedef typename List::const_iterator const_iterator;

 private:
    size_t md = 1;
    List lst = List();
    size_t cnt = 0;
    std::vector<iterator> it;
    std::vector<bool> used;
    Hash hasher;
    size_t geth(const Key &k) const {
        return hasher(k) % md;
    }
    void add(const pair &p) {
        ++cnt;
        int h = geth(p.first);
        if (!used[h]) {
            used[h] = true;
            lst.push_back(p);
            it[h] = --lst.end();
        } else {
            it[h] = lst.insert(it[h], p);
            if (size() > md * 2) rehash(md * 2.23);
        }
    }
    template<class It>
    void addit(It begin, It end) {
        while (begin != end)
            add(*begin++);
    }
    void rehash(size_t nmd) {
        if (nmd == 0) nmd = 1;
        std::vector<pair> cop(begin(), end());
        clear();
        md = nmd;
        it = std::vector<iterator>(md);
        used = std::vector<bool>(md);
        for (auto i : cop) add(i);
    }

 public:
    HashMap(Hash _ = Hash()) : hasher(_) { rehash(md); }
    template<class It>
    HashMap(It begin, It end, Hash _ = Hash()) : hasher(_) {
        rehash(md);
        addit(begin, end);
    }
    HashMap(std::initializer_list<pair> lst, Hash _ = Hash()) : hasher(_) {
        rehash(md);
        addit(lst.begin(), lst.end());
    }
    HashMap& operator=(const HashMap &h) {
        std::vector<pair> v(h.begin(), h.end());
        clear();
        hasher = h.hasher;
        rehash(md);
        for (const auto &i : v)
            add(i);
        return *this;
    }
    iterator begin() { return lst.begin(); }
    iterator end() { return lst.end(); }
    const_iterator begin() const { return lst.begin(); }
    const_iterator end() const { return lst.end(); }
    const_iterator find(const Key &k) const {
        size_t h = geth(k);
        if (!used[h]) return end();
        auto c = it[h];
        while (c != end() && geth(c->first) == h) {
            if (c->first == k) return c;
            ++c;
        }
        return end();
    }
    iterator find(const Key &k) {
        size_t h = geth(k);
        if (!used[h]) return end();
        auto c = it[h];
        while (c != end() && geth(c->first) == h) {
            if (c->first == k) return c;
            ++c;
        }
        return end();
    }
    bool count(const Key &k) const {
        return find(k) != end();
    }
    bool empty() const { return lst.empty(); }
    size_t size() const { return cnt; }
    void insert(const pair &p) {
        if (!count(p.first))
            add(p);
    }
    void erase(iterator er) {
        if (it[geth(er->first)] == er) {
            auto next = er;
            ++next;
            if (next == end() || geth(next->first) != geth(er->first))
                used[geth(er->first)] = false;
            else
                it[geth(er->first)] = next;
        }
        lst.erase(er);
        if (size() < md / 2) rehash(md / 2.23);
    }
    void erase(const Key &k) {
        if (count(k)) {
            --cnt;
            erase(find(k));
        }
    }
    Value& operator[](const Key &p) {
        if (!count(p))
            add(pair(p, Value()));
        return find(p)->second;
    }
    void clear() {
        used = std::vector<bool>(md);
        lst.clear();
        cnt = 0;
    }
    Hash hash_function() const {
        return hasher;
    }
    const Value& at(const Key &k) const {
        if (!count(k)) throw std::out_of_range("none");
        return find(k)->second;
    }
};
