#pragma once

/**
 * @brief Template class to transform any class into a singleton
 *
 * Just include this header and use a typedef to create the singleton:
 *
 *     #include "singleton.hpp"
 *     typedef Singleton<MyClass> MySingleton;
 *
 * Once created the singleton can be used by accessing `instance`:
 *
 *     MySingleton::instance().myMethod();
 *
 *
 */

template <class T>
class Singleton {
 public:
  Singleton() = delete;
  ~Singleton() = delete;
  Singleton(Singleton const&) = delete;
  Singleton& operator=(Singleton const&) = delete;

  static T& instance() {
    static T instance;
    return instance;
  }
};
