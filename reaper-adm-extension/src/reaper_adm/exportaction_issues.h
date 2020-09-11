#pragma once

#include <string>

class AdmAuthoringError : public std::logic_error {
public:
    AdmAuthoringError(std::string message) : std::logic_error{ message } {}
};