#pragma once
#include <string>
#include <cstring>
#ifdef _WIN32
  #define NOMINMAX
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #include <WDL/swell/swell-types.h>
#endif

namespace admplug {

class ReaperGUID
{
public:
    explicit ReaperGUID(GUID* guidPtr);
    explicit ReaperGUID(std::string guidStr);

    GUID* get();
    friend inline bool operator== (ReaperGUID const& lhs, ReaperGUID const & rhs) {
      return memcmp(&lhs.guid, &rhs.guid, sizeof(GUID)) == 0;
    }

private:
    void assignFrom(std::string guidStr);
    bool isValid(std::string guidStr);
    GUID guid;
};

inline bool operator!= (ReaperGUID const& lhs, ReaperGUID const& rhs) {
    return !(lhs == rhs);
}

}

