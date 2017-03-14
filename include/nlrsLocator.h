#pragma once

#include "nlrs_assert.h"

namespace nlrs
{

// there is an additional template parameter, so that we
// can generate multiple classes of the type T
template<typename T, unsigned N = 0>
class locator
{
public:
    static T* get()
    {
        NLRS_ASSERT(service_ != nullptr);
        return service_;
    }

    static void set(T* service)
    {
        service_ = service;
    }

private:
    static T* service_;
};

template<typename T, unsigned N>
T* locator<T, N>::service_ = nullptr;

}
