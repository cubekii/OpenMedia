#pragma once

namespace openmedia {

template<class... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

template<class... Ts>
overloads(Ts...) -> overloads<Ts...>;

}
