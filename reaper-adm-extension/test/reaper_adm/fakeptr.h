#ifndef FAKEPTR_H
#define FAKEPTR_H
#include <vector>

struct FakePtrFactory {
    template <typename T>
    T* get() {
        fakes.push_back(count++);
        return reinterpret_cast<T*>(&fakes.back());
    }
    int count{0};
    std::vector<int> fakes;
};

#endif // FAKEPTR_H
