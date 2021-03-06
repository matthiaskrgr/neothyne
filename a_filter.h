#ifndef A_FILTER_HDR
#define A_FILTER_HDR
#include "u_vector.h"
#include "m_vec.h"
#include "a_fader.h"

namespace a {

struct Source;

struct FilterInstance {
    virtual void filter(float *buffer, size_t samples, size_t channels, float sampleRate, float time) = 0;

    virtual void setFilterParam(int attrib, float value);
    virtual void fadeFilterParam(int attrib, float from, float to, float time, float startTime);
    virtual void oscFilterParam(int attrib, float from, float to, float time, float startTime);

    virtual ~FilterInstance();
};

struct Filter {
    virtual FilterInstance *create() = 0;
    virtual ~Filter();
};

struct EchoFilter;

struct EchoFilterInstance final : FilterInstance {
    virtual void filter(float *buffer, size_t samples, size_t channels, float sampleRate, float streamTime) final;
    virtual ~EchoFilterInstance() final;
    EchoFilterInstance(EchoFilter *parent);

private:
    u::vector<float> m_buffer;
    EchoFilter *m_parent;
    size_t m_offset;
};

struct EchoFilter final : Filter {
    virtual FilterInstance *create() final;
    EchoFilter();
    void setParams(float delay, float decay = 0.7f, float filter = 0.0f);

private:
    friend struct EchoFilterInstance;
    float m_delay;
    float m_decay;
    float m_filter;
};

struct BQRFilter;

struct BQRFilterInstance final : FilterInstance {
    BQRFilterInstance(BQRFilter *parent);

    virtual void filter(float *buffer, size_t samples, size_t channels, float sampleRate, float time) final;

    virtual void setFilterParam(int attrib, float value) final;
    virtual void fadeFilterParam(int attrib, float from, float to, float time, float startTime) final;
    virtual void oscFilterParam(int attrib, float from, float to, float time, float startTime) final;

    virtual ~BQRFilterInstance() final;

private:
    void calcParams();

    BQRFilter *m_parent;

    m::vec3 m_a;
    m::vec3 m_b;
    m::vec2 m_x1, m_x2;
    m::vec2 m_y1, m_y2;

    int m_filterType;

    float m_sampleRate;
    float m_frequency;
    float m_resonance;
    float m_wetSignal;

    Fader m_resonanceFader;
    Fader m_frequencyFader;
    Fader m_sampleRateFader;
    Fader m_wetSignalFader;

    bool m_active;
    bool m_dirty;
};

struct BQRFilter final : Filter {
    // type
    enum {
        kNone,
        kLowPass,
        kHighPass,
        kBandPass
    };

    // attribute
    enum {
        kSampleRate,
        kFrequency,
        kResonance,
        kWet
    };

    virtual BQRFilterInstance *create() final;
    BQRFilter();
    void setParams(int type, float sampleRate, float frequency, float resonance);
    virtual ~BQRFilter() final;

private:
    friend struct BQRFilterInstance;
    int m_filterType;
    float m_sampleRate;
    float m_frequency;
    float m_resonance;
};

// check docs/AUDIO.md for explanation of how this does what it does
struct DCRemovalFilter;

struct DCRemovalFilterInstance final : FilterInstance {
    virtual void filter(float *buffer, size_t samples, size_t channels, float sampleRate, float streamTime) final;

    virtual ~DCRemovalFilterInstance() final;

    DCRemovalFilterInstance(DCRemovalFilter *parent);
private:
    u::vector<float> m_buffer;
    u::vector<float> m_totals;
    size_t m_offset;
    DCRemovalFilter *m_parent;
};

struct DCRemovalFilter final : Filter {
    virtual FilterInstance *create() final;
    DCRemovalFilter();
    void setParams(float length = 0.1f);

private:
    friend struct DCRemovalFilterInstance;
    float m_length;
};

struct LoFiFilter;

}

#endif
