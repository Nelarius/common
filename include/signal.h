#pragma once

#include "aliases.h"
#include "memory_arena.h"

#include <functional>
#include <utility>
#include "stl/unordered_map.h"

namespace nlrs
{

template<class... Args>
class signal
{
public:
    using handle = u32;

    signal()
        : current_id_(0u)
    {}
    signal(const signal<Args...>&) = delete;
    signal<Args...>& operator=(const signal<Args...>&) = delete;
    signal(signal<Args...>&&) = default;
    signal<Args...>& operator=(signal<Args...>&&) = default;
    ~signal() = default;

    usize num_connections() const { return slots_.size(); }

    handle connect(std::function<void(Args&&...)> slot)
    {
        slots_.insert(std::make_pair(++current_id_, slot));
        return current_id_;
    }

    template<class T>
    handle connect(T* instance, void(T::*func)(Args...))
    {
        return connect([instance, func](Args&&...args) -> void { (instance->*func)(std::forward<Args>(args)...); });
    }

    template<class T>
    handle connect(T* instance, void(T::*func)(Args...) const)
    {
        return connect([instance, func](Args&&...args) -> void { (instance->*func)(std::forward<Args>(args)...); });
    }

    void disconnect(handle id)
    {
        slots_.erase(id);
    }

    void disconnect_all()
    {
        slots_.clear();
    }

    void emit(Args&&... args)
    {
        for (auto elem : slots_)
        {
            elem.second(std::forward<Args>(args)...);
        }
    }

private:
    std::pmr::unordered_map<handle, std::function<void(Args...)>> slots_;
    u32 current_id_;
};

template<>
class signal<void>
{
public:
    using handle = u32;

    signal()
        : current_id_(0u)
    {}
    signal(const signal<void>&) = delete;
    signal<void>& operator=(const signal<void>&) = delete;
    signal(signal<void>&&) = default;
    signal<void>& operator=(signal<void>&&) = default;
    ~signal() = default;

    inline usize num_connections() const { return slots_.size(); }

    inline handle connect(std::function<void(void)> slot)
    {
        slots_.insert(std::make_pair(++current_id_, slot));
        return current_id_;
    }

    template<class T>
    handle connect(T* instance, void(T::*func)(void))
    {
        return connect(std::bind(func, instance));
    }

    template<class T>
    handle connect(T* instance, void(T::*func)(void) const)
    {
        return connect(std::bind(func, instance));
    }

    inline void disconnect(handle id)
    {
        slots_.erase(id);
    }

    inline void disconnect_all()
    {
        slots_.clear();
    }

    inline void emit()
    {
        for (auto elem : slots_)
        {
            elem.second();
        }
    }

private:
    std::pmr::unordered_map<handle, std::function<void(void)>> slots_;
    u32 current_id_;
};

}
