/*
 * Copyright 2022-2024 TII (SSRC) and the Ghaf contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <GhafAudioControl/Volume.hpp>

#include <functional>
#include <map>
#include <memory>
#include <optional>

#include <sigc++/signal.h>

namespace ghaf::AudioControl
{

using Index = uint64_t;

class IAudioControlBackend
{
public:
    enum class EventType
    {
        Add,
        Update,
        Delete
    };

    template<class T>
    class SignalMap final
    {
    public:
        using IndexT = Index;
        using PtrT = std::shared_ptr<T>;

    private:
        using MapType = std::map<Index, PtrT>;

    public:
        using Iter = MapType::iterator;
        using OnChangeSignal = sigc::signal<void(EventType, IndexT, PtrT)>;

        SignalMap() = default;

        void add(Index key, PtrT&& data)
        {
            auto result = m_map.emplace(key, std::forward<PtrT&&>(data));
            auto iter = result.first;

            m_onChange(EventType::Add, iter->first, iter->second);
        }

        [[nodiscard]] std::optional<Iter> findByKey(Index key)
        {
            if (auto iter = m_map.find(key); iter != m_map.end())
                return iter;

            return std::nullopt;
        }

        [[nodiscard]] std::vector<Iter> findByPredicate(const std::function<bool(const T&)>& predicate)
        {
            std::vector<Iter> iterators;

            for (Iter it = m_map.begin(); it != m_map.end(); ++it)
            {
                if (const PtrT ptr = it->second; predicate(*ptr))
                    iterators.push_back(it);
            }

            return iterators;
        }

        [[nodiscard]] std::vector<PtrT> getAllValues() const
        {
            std::vector<PtrT> iterators;

            for (auto it = m_map.cbegin(); it != m_map.cend(); ++it)
                iterators.push_back(it->second);

            return iterators;
        }

        void update(Iter iter, const std::function<void(T&)>& updateFunction)
        {
            PtrT ptr = iter->second;

            updateFunction(*ptr);
            m_onChange(EventType::Update, iter->first, ptr);
        }

        void remove(Iter iter, const std::function<void(T&)>& deleteFunction)
        {
            PtrT ptr = iter->second;
            deleteFunction(*ptr);

            const auto key = iter->first;
            std::ignore = m_map.erase(key);

            m_onChange(EventType::Delete, key, nullptr);
        }

        [[nodiscard]] OnChangeSignal onChange() const
        {
            return m_onChange;
        }

    private:
        MapType m_map;
        OnChangeSignal m_onChange;
    };

    class IDevice
    {
    public:
        using Ptr = std::shared_ptr<IDevice>;
        using IntexT = Index;

        enum class Type
        {
            Sink,
            Source,
            SinkInput,
            SourceOutput
        };

        using OnUpdateSignal = sigc::signal<void()>;
        using OnDeleteSignal = sigc::signal<void()>;

        virtual ~IDevice() = default;

        virtual bool operator==(const IDevice& other) const = 0;

        [[nodiscard]] virtual Index getIndex() const = 0;

        [[nodiscard]] virtual std::string getName() const = 0;
        [[nodiscard]] virtual Type getType() const = 0;

        [[nodiscard]] virtual bool isEnabled() const = 0;

        [[nodiscard]] virtual bool isMuted() const = 0;
        virtual void setMuted(bool mute) = 0;

        [[nodiscard]] virtual Volume getVolume() const = 0;
        virtual void setVolume(Volume volume) = 0;

        [[nodiscard]] virtual std::string toString() const = 0;

        [[nodiscard]] virtual OnUpdateSignal onUpdate() const = 0;
        [[nodiscard]] virtual OnDeleteSignal onDelete() const = 0;
    };

    class ISink : public IDevice
    {
    };

    class ISource : public IDevice
    {
    };

    class ISinkInput : public IDevice
    {
    };

    class ISourceOutput : public IDevice
    {
    };

    using Sinks = SignalMap<ISink>;
    using Sources = SignalMap<ISource>;
    using SinkInputs = SignalMap<ISinkInput>;
    using SourceOutputs = SignalMap<ISourceOutput>;

    using OnErrorSignal = sigc::signal<void(std::string)>;

    virtual ~IAudioControlBackend() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void setDeviceVolume(IDevice::IntexT index, IDevice::Type type, Volume volume) = 0;
    virtual void setDeviceMute(IDevice::IntexT index, IDevice::Type type, bool mute) = 0;

    virtual std::vector<IDevice::Ptr> getAllDevices() const = 0;

    [[nodiscard]] virtual Sinks::OnChangeSignal onSinksChanged() const = 0;
    [[nodiscard]] virtual Sources::OnChangeSignal onSourcesChanged() const = 0;
    [[nodiscard]] virtual SinkInputs::OnChangeSignal onSinkInputsChanged() const = 0;
    [[nodiscard]] virtual SourceOutputs::OnChangeSignal onSourceOutputsChanged() const = 0;

    [[nodiscard]] virtual OnErrorSignal onError() const = 0;
};

} // namespace ghaf::AudioControl
