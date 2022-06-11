//
//  on_exit.hpp
//  AngryKit
//
//  Created by  Ivan Ushakov on 17.07.2021.
//

#pragma once

namespace angry
{

template<typename F>
struct OnExit
{
    F func;

    OnExit(F&& f) : func(std::forward<F>(f)) {}

    ~OnExit()
    {
        func();
    }
};

template<typename F> OnExit(F&& frv) -> OnExit<F>;

}
