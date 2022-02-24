//
// Created by Richard Bailey on 24/02/2022.
//

#ifndef EAR_PRODUCTION_SUITE_WEAK_PTR_HPP
#define EAR_PRODUCTION_SUITE_WEAK_PTR_HPP
namespace ear::plugin {

template <typename ElementT, typename F>
void removeDeadPointersAfter(std::vector<std::weak_ptr<ElementT>>& container,
                             F&& fn) {
  container.erase(std::remove_if(container.begin(), container.end(),
                                 [&fn](std::weak_ptr<ElementT> const& weak) {
                                   auto shared = weak.lock();
                                   if (shared) {
                                     fn(shared);
                                   }
                                   return !shared;
                                 }),
                  container.end());
}

}

#endif  // EAR_PRODUCTION_SUITE_WEAK_PTR_HPP
