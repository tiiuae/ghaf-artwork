/*
 * Copyright 2022-2024 TII (SSRC) and the Ghaf contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <GhafAudioControl/Backends/PulseAudio/GeneralDevice.hpp>
#include <GhafAudioControl/IAudioControlBackend.hpp>

#include <pulse/context.h>
#include <pulse/introspect.h>

namespace ghaf::AudioControl::Backend::PulseAudio
{

class SourceOutput final : public IAudioControlBackend::ISourceOutput
{
public:
    SourceOutput(const pa_source_output_info& info, pa_context& context);

    bool operator==(const IDevice& other) const override;

    Index getIndex() const override
    {
        return m_device.getIndex();
    }

    std::string getName() const override
    {
        return m_device.getName();
    }

    Type getType() const override
    {
        return Type::SourceOutput;
    }

    bool isEnabled() const override
    {
        return m_device.isEnabled();
    }

    bool isMuted() const override
    {
        return m_device.isMuted();
    }

    void setMuted(bool mute) override;

    Volume getVolume() const override
    {
        return m_device.getVolume();
    }

    void setVolume(Volume volume) override;

    std::string toString() const override;

    std::string getDescription() const
    {
        return m_device.getDescription();
    }

    uint32_t getCardIndex() const noexcept
    {
        return m_device.getCardIndex();
    }

    void update(const pa_source_output_info& info)
    {
        m_device.update(info);
        m_onUpdate();
    }

    void markDeleted();

    OnUpdateSignal onUpdate() const override
    {
        return m_onUpdate;
    }

    OnDeleteSignal onDelete() const override
    {
        return m_onDelete;
    }

private:
    void deleteCheck();

private:
    GeneralDeviceImpl m_device;

    OnUpdateSignal m_onUpdate;
    OnDeleteSignal m_onDelete;
};

} // namespace ghaf::AudioControl::Backend::PulseAudio
