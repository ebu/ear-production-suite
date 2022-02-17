//
// Created by rsjbailey on 17/02/2022.
//

#pragma once
#include <memory>
#include <stdexcept>
#include "commands.hpp"

namespace ear::plugin::communication {
class InputControlConnection;

class HandshakeState {
 public:
  virtual void process() const = 0;
};

class InitState : public HandshakeState, public boost::static_visitor<void> {
 public:
  explicit InitState(InputControlConnection* connection);
  void operator()(NewConnectionResponse response) const;

  template <typename T>
  void operator()(T response) const {
    throw std::runtime_error("Unexpected response when establishing control connection");
  }
  void process() const override;
 private:
  void processResponse() const;
  InputControlConnection* controlConnection;
};

class ConnectingState : public HandshakeState, public boost::static_visitor<void> {
 public:
  explicit ConnectingState(InputControlConnection* connection);
  void operator()(NewConnectionResponse response) const;
  void operator()(ConnectionDetailsResponse response) const;
  template <typename T>
  void operator()(T response) const {
    throw std::runtime_error("Unexpected response when establishing control connection");
  }

  void process() const override;
 private:
  void processResponse() const;
  InputControlConnection* controlConnection;
};

class ErrorState : public HandshakeState {
 public:
  explicit ErrorState(std::string what);
  void process() const override;
  std::string what_;
};

}
