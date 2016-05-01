#include "a_filter.h"
#include "a_system.h"

#include "m_trig.h"

#include "u_misc.h"

namespace a {

///! filter
filter::~filter() {
    // Empty
}

///! filterInstance
filterInstance::~filterInstance() {
    // Empty
}
void filterInstance::setFilterParam(int, float) {
    // Empty
}
void filterInstance::fadeFilterParam(int, float, float, float, float) {
    // Rmpty
}
void filterInstance::oscFilterParam(int, float, float, float, float) {
    // Empty
}

///! echoFilter
echoFilterInstance::echoFilterInstance(echoFilter *parent)
    : m_parent(parent)
    , m_offset(0)
{
}

void echoFilterInstance::filter(float *buffer, size_t samples, bool stereo, float sampleRate, float) {
    const size_t channels = stereo ? 2 : 1;
    if (m_buffer.empty()) {
        const size_t length = m::ceil(m_parent->m_delay * sampleRate) * channels;
        m_buffer.resize(length);
    }
    const size_t bufferLength = m_buffer.size() / channels;
    const float decay = m_parent->m_decay;
    for (size_t i = 0; i < samples; i++) {
        for (size_t j = 0; j < channels; j++) {
            const size_t c = j * bufferLength;
            const size_t b = j * samples;
            float sample = buffer[i + b] + m_buffer[m_offset + c] * decay;
            m_buffer[m_offset + c] = sample;
            buffer[i + b] = sample;
        }
        m_offset = (m_offset + 1) % bufferLength;
    }
}

echoFilterInstance::~echoFilterInstance() {
    // Empty
}

echoFilter::echoFilter()
    : m_delay(1.0f)
    , m_decay(0.5f)
{
}

void echoFilter::setParams(float delay, float decay) {
    m_delay = delay;
    m_decay = decay;
}

filterInstance *echoFilter::create() {
    return new echoFilterInstance(this);
}

///! BQRFilterInstance
BQRFilterInstance::BQRFilterInstance(BQRFilter *parent)
    : m_parent(parent)
    , m_filterType(m_parent->m_filterType)
    , m_sampleRate(m_parent->m_sampleRate)
    , m_frequency(m_parent->m_frequency)
    , m_resonance(m_parent->m_resonance)
    , m_active(false)
{
    calcParams();
}

void BQRFilterInstance::filter(float *buffer, size_t samples, bool stereo, float time, float) {
    if (!m_active) return;

    if (m_frequencyFader.m_active > 0) {
        m_dirty = true;
        m_frequency = m_frequencyFader(time);
    }

    if (m_resonanceFader.m_active > 0) {
        m_dirty = true;
        m_resonance = m_resonanceFader(time);
    }

    if (m_sampleRateFader.m_active > 0) {
        m_dirty = true;
        m_sampleRate = m_sampleRateFader(time);
    }

    if (m_dirty)
        calcParams();

    size_t channels = stereo ? 2 : 1;
    for (size_t s = 0, c = 0; s < channels; s++) {
        for (size_t i = 0; i < samples; i += 2, c++) {
            // filter the inputs
            float x = buffer[c];
            m_y2[s] = (m_a.x * x) + (m_a.y * m_x1[s]) + (m_a.z * m_x2[s])
                                  - (m_b.x * m_y1[s]) - (m_b.y * m_y2[s]);
            buffer[c++] = m_y2[s];
            // permute filter ops to reduce movement
            m_x2[s] = buffer[c];
            m_y1[s] = (m_a.y * x) + (m_a.x * m_x2[s]) + (m_a.z * m_x1[s])
                                  - (m_b.x * m_y2[s]) - (m_b.y * m_y1[s]);
            buffer[c] = m_y1[s];
            // move it a little
            m_x1[s] = m_x2[s];
            m_x2[s] = x;
        }
        // apply a very small impulse to prevent underflow
        m_y1[s] += 1.0e-26f;
    }
}


void BQRFilterInstance::setFilterParam(int attrib, float value) {
    switch (attrib) {
    case BQRFilter::kFrequency:
        m_dirty = true;
        m_frequencyFader.m_active = 0;
        m_frequency = value;
        break;
    case BQRFilter::kSampleRate:
        m_dirty = true;
        m_sampleRateFader.m_active = 0;
        m_sampleRate = value;
        break;
    case BQRFilter::kResonance:
        m_dirty = true;
        m_resonanceFader.m_active = 0;
        m_resonance = value;
        break;
    }
}

void BQRFilterInstance::fadeFilterParam(int attrib, float from, float to, float time, float startTime) {
    if (from == to || time <= 0.0f)
        return;
    switch (attrib) {
    case BQRFilter::kFrequency:
        m_frequencyFader.set(fader::kLERP, from, to, time, startTime);
        break;
    case BQRFilter::kSampleRate:
        m_sampleRateFader.set(fader::kLERP, from, to, time, startTime);
        break;
    case BQRFilter::kResonance:
        m_sampleRateFader.set(fader::kLERP, from, to, time, startTime);
        break;
    }
}

void BQRFilterInstance::oscFilterParam(int attrib, float from, float to, float time, float startTime) {
    if (from == to || time <= 0.0f)
        return;
    switch (attrib) {
    case BQRFilter::kFrequency:
        m_frequencyFader.set(fader::kLFO, from, to, time, startTime);
        break;
    case BQRFilter::kSampleRate:
        m_sampleRateFader.set(fader::kLFO, from, to, time, startTime);
        break;
    case BQRFilter::kResonance:
        m_resonanceFader.set(fader::kLFO, from, to, time, startTime);
        break;
    }
}

BQRFilterInstance::~BQRFilterInstance() {
    // Empty
}

void BQRFilterInstance::calcParams() {
    const m::vec2 omega = m::sincos((2.0f * m::kPi * m_frequency) / m_sampleRate);
    const float alpha = omega.x / (2.0f * m_resonance);
    const float scalar = 1.0f / (1.0f + alpha);
    switch (m_filterType) {
    case BQRFilter::kNone:
        m_active = false;
        break;
    case BQRFilter::kLowPass:
        m_a.x = 0.5f * (1.0f - omega.y) * scalar;
        m_a.y = (1.0f - omega.y) * scalar;
        m_a.z = m_a.x;
        m_b.x = -2.0f * omega.y * scalar;
        m_b.y = (1.0f - alpha) * scalar;
        m_active = true;
        break;
    case BQRFilter::kHighPass:
        m_a.x = 0.5f * (1.0f + omega.y) * scalar;
        m_a.y = -(1.0f + omega.y) * scalar;
        m_a.z = m_a.x;
        m_b.x = -2.0f * omega.y * scalar;
        m_b.y = (1.0f - alpha) * scalar;
        m_active = true;
        break;
    case BQRFilter::kBandPass:
        m_a.x = alpha * scalar;
        m_a.y = 0.0f;
        m_a.z = -m_a.x;
        m_b.x = -2.0f * omega.y * scalar;
        m_b.y = (1.0f - alpha) * scalar;
        m_active = true;
        break;
    }
}

BQRFilter::BQRFilter()
{
    // Empty
}

void BQRFilter::setParams(int type, float sampleRate, float frequency, float resonance) {
    m_filterType = type;
    m_sampleRate = sampleRate;
    m_frequency = frequency;
    m_resonance = resonance;
}

BQRFilter::~BQRFilter() {
    // Empty
}

BQRFilterInstance *BQRFilter::create() {
    return new BQRFilterInstance(this);
}

///! DCRemovalFilterInstance
DCRemovalFilterInstance::DCRemovalFilterInstance(DCRemovalFilter *parent)
    : m_offset(0)
    , m_parent(parent)
{
}

void DCRemovalFilterInstance::filter(float *buffer, size_t samples, bool stereo, float sampleRate, float) {
    const size_t channels = stereo ? 2 : 1;
    if (m_buffer.empty()) {
        m_buffer.resize(size_t(m::ceil(m_parent->m_length * sampleRate)) * channels);
        m_totals.resize(channels);
    }
    size_t bufferLength = m_buffer.size() / channels;
    for (size_t i = 0; i < samples; i++) {
        for (size_t j = 0; j < channels; j++) {
            const int c = j * bufferLength;
            const int b = j * samples;
            float n = buffer[i + b];
            m_totals[j] -= m_buffer[m_offset + c];
            m_totals[j] += n;
            m_buffer[m_offset + c] = n;
            n -= m_totals[j] / bufferLength;
            buffer[i + b] += (n - buffer[i + b]) * m_parent->m_length;
        }
        m_offset = (m_offset + 1) % bufferLength;
    }
}

DCRemovalFilterInstance::~DCRemovalFilterInstance() {
    // Empty
}

DCRemovalFilter::DCRemovalFilter()
    : m_length(0.1f)
{
}

void DCRemovalFilter::setParams(float length) {
    m_length = length;
}

filterInstance *DCRemovalFilter::create() {
    return new DCRemovalFilterInstance(this);
}

}
