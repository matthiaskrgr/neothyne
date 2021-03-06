#ifndef U_MISC_HDR
#define U_MISC_HDR
#include <stdarg.h>  // va_start, va_end, va_list
#include <string.h>  // strcpy
#include <stdint.h> // uint32_t
#include <stdio.h>

#include "u_string.h" // u::string
#include "u_vector.h" // u::vector
#include "u_memory.h" // u::unique_ptr

namespace u {
namespace detail {
    int c99vsnprintf(char *str, size_t maxSize, const char *format, va_list ap);
    int c99vsscanf(const char *s, const char *format, va_list ap);
}

template <typename T>
inline T endianSwap(T value) {
    union {
        int i;
        char data[sizeof(int)];
    } x = { 1 };
    if (x.data[0] != 1) {
        union {
            T value;
            unsigned char data[sizeof(T)];
        } src = { value }, dst;
        for (size_t i = 0; i < sizeof value; i++)
            dst.data[i] = src.data[sizeof value - i - 1];
        return dst.value;
    }
    return value;
}

template <typename T>
inline void endianSwap(T *data, size_t length) {
    for (T *end = &data[length]; data < end; data++)
        *data = endianSwap(*data);
}

inline int sscanf(const u::string &thing, const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    int value = detail::c99vsscanf(thing.c_str(), fmt, va);
    va_end(va);
    return value;
}

inline u::vector<u::string> split(const char *str, char ch = ' ') {
    u::vector<u::string> splitted;
    bool quoted = false;
    splitted.push_back("");
    for (const char *s = str; *s; s++) {
        if (*s == '\\' && (s[1] == '\\' || s[1] == '\"')) {
            if (s[1] == '\\') splitted[splitted.size() - 1] += "\\";
            if (s[1] == '\"') splitted[splitted.size() - 1] += "\"";
            s += 2;
            continue;
        }
        if (*s == '\"') {
            quoted = !quoted;
            continue;
        }
        if (*s == ch && !quoted)
            splitted.push_back("");
        else
            splitted[splitted.size() - 1] += *s;
    }
    return splitted;
}

inline u::vector<u::string> split(const u::string &str, char ch = ' ') {
    return u::split(str.c_str(), ch);
}

inline int atoi(const u::string &str) {
    return ::atoi(str.c_str());
}

inline float atof(const u::string &str) {
    return ::atof(str.c_str());
}

// argument normalization for format
template <typename T>
inline constexpr typename u::enable_if<u::is_integral<T>::value, long>::type
formatNormalize(T argument) { return argument; }

template <typename T>
inline constexpr typename u::enable_if<u::is_floating_point<T>::value, double>::type
formatNormalize(T argument) { return argument; }

template <typename T>
inline constexpr typename u::enable_if<u::is_pointer<T>::value, T>::type
formatNormalize(T argument) { return argument; }

inline const char *formatNormalize(const u::string &argument) {
    return argument.c_str();
}

static inline u::string formatProcess(const char *fmt, va_list va) {
    const int len = detail::c99vsnprintf(nullptr, 0, fmt, va);
    u::string data;
    data.resize(len);
    detail::c99vsnprintf(&data[0], len + 1, fmt, va);
    return move(data);
}

static inline u::string formatProcess(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    u::string value = u::formatProcess(fmt, va);
    va_end(va);
    return u::move(value);
}

template <typename... Ts>
inline u::string format(const char *fmt, const Ts&... ts) {
    return formatProcess(fmt, formatNormalize(ts)...);
}

inline u::string format(const char *fmt, va_list va) {
    return formatProcess(fmt, va);
}

static inline u::string sizeMetric(size_t size) {
    static const char *kSizes[] = { "B", "kB", "MB", "GB" };
    size_t r = 0;
    size_t i = 0;
    for (; size >= 1024 && i < sizeof kSizes / sizeof *kSizes; i++) {
        r = size % 1024;
        size /= 1024;
    }
    U_ASSERT(i != sizeof kSizes / sizeof *kSizes);
    return u::format("%.2f %s", float(size) + float(r) / 1024.0f, kSizes[i]);
}

const char *CPUDesc();
const char *RAMDesc();

template <typename... Ts>
inline void fprint(FILE *fp, const char *fmt, const Ts&... ts) {
    fprintf(fp, "%s", format(fmt, ts...).c_str());
}

void *moveMemory(void *dest, const void *src, size_t n);

// Random number generation facilities
uint32_t randu(); //[0, UINT32_MAX]
float randf(); // [0, 1]

unsigned char log2(uint32_t v);

uint32_t crc32(const unsigned char *buffer, size_t length);

// We don't use <ctype.h> as it's locale dependent and the engine operates on
// the assumption that everything is the C locale.
static inline bool isspace(int ch) {
    return ch == ' ' || (unsigned)ch-'\t' < 5;
}

template <typename T>
static inline constexpr T fourCC(const char (&four)[5]) {
    return ((four[3] << 24) & 0xFF000000) | ((four[2] << 16) & 0x00FF0000) |
           ((four[1] << 8) & 0x0000FF00) | ((four[0] << 0) & 0x000000FF);
}

}
#endif
