#ifndef U_STRING_HDR
#define U_STRING_HDR
#include <string.h>

#include "u_traits.h"

namespace u {


struct string {
    typedef char *iterator;
    typedef const char *const_iterator;

    static constexpr size_t kSmallStringSize = 512_z;
    static constexpr size_t npos = -1_z;

    // constructors
    string();
    string(const string &other);
    string(string &&other);
    string(const char *sz);
    string(const char *sz, size_t len);
    template <typename I>
    string(I first, I last);
    ~string();

    // iterators
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    string &operator=(const string &other);
    string &operator=(string &&other);

    const char* c_str() const;
    size_t size() const;
    bool empty() const;

    void reserve(size_t size);
    void resize(size_t size);
    void clear();

    string &append(char ch);
    string &append(const char *first, const char* last);
    string &append(const char *str, size_t len);
    string &append(const char *str);

    string &replace_all(const char *what, const char *with);

    char pop_back();
    char pop_front();

    char &operator[](size_t index);
    const char &operator[](size_t index) const;
    size_t find(char ch) const;

    void erase(size_t beg, size_t end);

    void swap(string& other);

    void reset();

private:
    static void swap_small_string(string &dst, string &src);
    static void move_small_string(string &dst, string &&src);

    char *m_first;
    char *m_last;
    char *m_capacity;
    char m_buffer[kSmallStringSize];
};

template <typename I>
inline string::string(I first, I last)
    : m_first(m_buffer)
    , m_last(m_buffer)
    , m_capacity(m_buffer + kSmallStringSize)
{
    const size_t len = last - first;
    reserve(len);
    append((const char *)first, len);
}

inline bool operator==(const string &lhs, const string &rhs) {
    if (&lhs == &rhs)
        return true;
    if (lhs.size() != rhs.size())
        return false;
    return !strcmp(lhs.c_str(), rhs.c_str());
}

inline bool operator!=(const string &lhs, const string &rhs) {
    if (&lhs == &rhs) return false;
    return strcmp(lhs.c_str(), rhs.c_str()) != 0;
}

inline bool operator<(const string &lhs, const string &rhs) {
    if (&lhs == &rhs) return false;
    return strcmp(lhs.c_str(), rhs.c_str()) < 0;
}

inline bool operator>(const string &lhs, const string &rhs) {
    if (&lhs == &rhs) return false;
    return strcmp(lhs.c_str(), rhs.c_str()) > 0;
}

inline string operator+(const string &lhs, const char *rhs) {
    return string(lhs).append(rhs, strlen(rhs));
}

inline string operator+(const char *lhs, const string &rhs) {
    return string(lhs).append(rhs.c_str(), rhs.size());
}

inline string operator+(const string &lhs, const string &rhs) {
    return string(lhs).append(rhs.c_str(), rhs.size());
}

inline string operator+(const string &lhs, const char ch) {
    return string(lhs).append(ch);
}

inline void operator+=(string &lhs, const char *rhs) {
    lhs.append(rhs, strlen(rhs));
}

inline void operator+=(string &lhs, const string &rhs) {
    operator+=(lhs, rhs.c_str());
}

inline void operator+=(string &lhs, char rhs) {
    lhs.append(&rhs, 1);
}

size_t hash(const string &str);

}

#endif
