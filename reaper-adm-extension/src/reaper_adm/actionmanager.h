#pragma once
#include <map>
#include <string>
#include <functional>
#include <memory>
#include "reaper_plugin.h"

typedef struct HWND__ *HWND;

namespace admplug {

class ReaperAPI;
class ActionManager;

class ActionIdentifier {
public:
    virtual ~ActionIdentifier() = default;
    virtual int getId() const = 0;
    virtual void setEnabled(bool enable) = 0;
    virtual int isEnabled() const = 0;
};

class Action {
public:
    Action(std::string desc, std::string id);
    virtual ~Action();
    virtual void operator() (ReaperAPI&) = 0;

    void setEnabled(bool enabled);
    int isEnabled() const;
    const char* description();
    const char* identifier();

private:
    std::string desc;
    std::string id;
    bool enable = true;
    long cmdId;

};

class SimpleAction : public Action {
public:
    SimpleAction(std::string desc,
        std::string id,
        std::function<void(ReaperAPI&)> func);
    ~SimpleAction() override = default;
    void operator ()(ReaperAPI & api) override;

private:
    std::function<void(ReaperAPI&)> func;
};

template<typename T>
class StatefulAction : public Action {
public:
    StatefulAction(std::string desc, std::string id, std::unique_ptr<T> state,
        std::function<void(ReaperAPI& api, T& state)> callback) :
        state{ std::move(state) },
        callback{ callback },
        Action(desc, id) {}
    ~StatefulAction() override = default;

    void operator() (ReaperAPI& api) override { callback(api, *state); }

private:
  std::unique_ptr<T> state;
  std::function<void(ReaperAPI& api, T& state)> callback;
};

class RegisteredAction : public ActionIdentifier {
public:
    RegisteredAction(std::shared_ptr<Action> action, long id);
    int getId() const override;
    void setEnabled(bool enable) override;
    int isEnabled() const override;
private:
    std::shared_ptr<Action> action;
    long id;
};

class ActionManager {
public:
    bool processCommandHook(int command);
    int processToggleActionCallback(int commandId);
    std::shared_ptr<ActionIdentifier> addAction(std::shared_ptr<Action> action);
    static ActionManager& getManager(std::shared_ptr<ReaperAPI> api);
    static ActionManager& getManager();
private:
    ActionManager(std::shared_ptr<ReaperAPI> api);
    std::map<int, std::shared_ptr<Action>> action_map;
    std::shared_ptr<ReaperAPI> api_ptr;
};

}
