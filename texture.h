#ifndef TEXTURE_HDR
#define TEXTURE_HDR
#include <stdint.h>

#include "u_string.h"
#include "u_optional.h"
#include "u_vector.h"

enum {
    kTexFlagNormal     = 1 << 0,
    kTexFlagGrey       = 1 << 1,
    kTexFlagDisk       = 1 << 2,
    kTexFlagPremul     = 1 << 3,
    kTexFlagNoCompress = 1 << 4,
    kTexFlagCompressed = 1 << 5
};

enum TextureFormat {
    kTexFormatRGB,
    kTexFormatRGBA,
    kTexFormatBGR,
    kTexFormatBGRA,
    kTexFormatRG,
    kTexFormatLuminance,
    kTexFormatDXT1,
    kTexFormatDXT3,
    kTexFormatDXT5,
    kTexFormatBC4U, // Also ATI1
    kTexFormatBC4S,
    kTexFormatBC5U, // also ATI2
    kTexFormatBC5S
};

enum SaveFormat {
    kSaveBMP,
    kSaveTGA,
    kSavePNG,
    kSaveJPG
};

static constexpr const char *kSaveFormatExtensions[] = {
    "bmp", "tga", "png", "jpg"
};

enum FlipFlags {
    kFlipHorizontal = 1 << 0,
    kFlipVertical = 1 << 1
};

struct Texture {
    Texture()
        : m_width(0)
        , m_height(0)
        , m_bpp(0)
        , m_pitch(0)
        , m_mips(0)
        , m_flags(0)
        , m_format(kTexFormatLuminance)
    {
    }

    Texture(const unsigned char *const data, size_t length, size_t width,
        size_t height, bool normal, TextureFormat format, size_t mips = 1);

    Texture(const Texture &other);
    Texture(Texture &&other);

    Texture &operator=(Texture &&other);

    bool load(const u::string &file, float quality = 1.0f);
    bool from(const unsigned char *const data, size_t length, size_t width,
        size_t height, bool normal, TextureFormat format, size_t mips = 1);

    bool save(const u::string &file, SaveFormat save = kSaveBMP, float quality = 1.0f);

    void colorize(uint32_t color);

    template <size_t S>
    static void halve(unsigned char *src, size_t sw, size_t sh, size_t stride,
        unsigned char *dst);

    template <size_t S>
    static void shift(unsigned char *src, size_t sw, size_t sh, size_t stride,
        unsigned char *dst, size_t dw, size_t dh);

    template <size_t S>
    static void scale(unsigned char *src, size_t sw, size_t sh, size_t stride,
        unsigned char *dst, size_t dw, size_t dh);

    static void scale(unsigned char *src, size_t sw, size_t sh, size_t bpp,
        size_t pitch, unsigned char *dst, size_t dw, size_t dh);
    static void reorient(unsigned char *src, size_t sw, size_t sh, size_t bpp,
        size_t stride, unsigned char *dst, bool flipx, bool flipy, bool swapxy);

    void resize(size_t width, size_t height);

    // Flip a texture either horizontally or vertically
    void flip(int flags = kFlipHorizontal);

    // Draw a string into the texture
    void drawString(size_t &line, const char *string);

    template <TextureFormat F>
    void convert();

    size_t width() const;
    size_t height() const;
    TextureFormat format() const;
    const char *components() const;

    const unsigned char *data() const;

    size_t size() const;
    size_t bpp() const;
    size_t pitch() const;
    size_t mips() const;

    const u::string &hashString() const;

    int flags() const;

    void unload();

private:
    void normalize();

    void writeTGA(u::vector<unsigned char> &outData);
    void writeBMP(u::vector<unsigned char> &outData);
    void writePNG(u::vector<unsigned char> &outData);
    void writeJPG(u::vector<unsigned char> &outData);

    u::optional<u::string> find(const u::string &file);
    template <typename T>
    bool decode(const u::vector<unsigned char> &data, const char *fileName,
        float quality = 1.0f);

    u::string m_hashString;
    u::vector<unsigned char> m_data;
    size_t m_width;
    size_t m_height;
    size_t m_bpp;
    size_t m_pitch;
    size_t m_mips;
    int m_flags;
    TextureFormat m_format;
};

#endif
