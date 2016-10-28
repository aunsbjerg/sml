//
// Copyright (c) 2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_SML_FRONT_STATE_HPP
#define BOOST_SML_FRONT_STATE_HPP

namespace front {

struct initial_state {};
struct terminate_state {};
struct history_state {};

template <class...>
struct transition;

template <class, class>
struct transition_sa;

template <class, class>
struct transition_sg;

template <class, class>
struct transition_eg;

template <class>
struct state;
template <class>
class stringable {};
template <class TState>
struct stringable<state<TState>> {
  static constexpr bool value = concepts::stringable<TState>::value;
};

template <class S, bool = stringable<S>::value>
struct state_str {
  static auto c_str() { return S::type::c_str(); }
};
template <class S>
struct state_str<S, false> {
  static auto c_str() { return __PRETTY_FUNCTION__; }
};
template <>
struct state_str<state<terminate_state>> {
  static auto c_str() { return "terminate"; }
};
template <char... Chrs>
struct state_str<state<aux::string<Chrs...>>, false> : aux::string<Chrs...> {};
template <char... Chrs, class T>
struct state_str<state<aux::string<Chrs...>(T)>, false> : state_str<state<aux::string<Chrs...>>> {};
template <class TState>

struct state_impl : state_str<TState> {
  template <class T>
  auto operator<=(const T &t) const {
    return transition<TState, T>{static_cast<const TState &>(*this), t};
  }
  template <class T>
  auto operator+(const T &t) const {
    return transition<TState, T>{static_cast<const TState &>(*this), t};
  }
  template <class T, __BOOST_SML_REQUIRES(concepts::callable<bool, T>::value)>
  auto operator[](const T &t) const {
    return transition_sg<TState, aux::zero_wrapper<T>>{static_cast<const TState &>(*this), aux::zero_wrapper<T>{t}};
  }
  template <class T, __BOOST_SML_REQUIRES(concepts::callable<void, T>::value)>
  auto operator/(const T &t) const {
    return transition_sa<TState, aux::zero_wrapper<T>>{static_cast<const TState &>(*this), aux::zero_wrapper<T>{t}};
  }
};

template <class TState>
struct state : state_impl<state<TState>> {
  using type = TState;

  static constexpr auto initial = false;
  static constexpr auto history = false;

  auto operator*() const { return state<TState(initial_state)>{}; }
  auto operator()(const initial_state &) const { return state<TState(initial_state)>{}; }
  auto operator()(const history_state &) const { return state<TState(history_state)>{}; }
  template <class... Ts>
  auto operator()(const state<Ts> &...) const {
    return state<TState(Ts...)>{};
  }
  template <class T>
  auto operator=(const T &t) const {
    return transition<T, state>{t, *this};
  }
};

template <class TState>
struct state<TState(initial_state)> : state_impl<state<TState(initial_state)>> {
  using type = TState;

  static constexpr auto initial = true;
  static constexpr auto history = false;

  template <class T>
  auto operator=(const T &t) const {
    return transition<T, state>{t, *this};
  }
};

template <class TState>
struct state<TState(history_state)> : state_impl<state<TState(history_state)>> {
  using type = TState;

  static constexpr auto initial = true;
  static constexpr auto history = true;

  template <class T>
  auto operator=(const T &t) const {
    return transition<T, state>{t, *this};
  }
};

template <class T, class = void>
struct state_sm {
  using type = state<T>;
};

template <class T>
struct state_sm<T, aux::enable_if_t<concepts::configurable<T>::value>> {
  using type = state<back::sm<back::sm_policy<T>>>;
};

}  // front

#endif
