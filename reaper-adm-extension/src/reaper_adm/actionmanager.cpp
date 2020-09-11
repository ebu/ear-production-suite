#include "actionmanager.h"
#include "reaperapi.h"
#include <WDL/swell/swell.h>
#include "reaper_plugin.h"
#include "reaperhost.h"
#include <stdexcept>
#include <cassert>

using namespace admplug;

namespace admplug {
struct Accel {
    gaccel_register_t accel_reg;
};
}

using namespace admplug;

Action::~Action() = default;
Action::Action(std::string desc, std::string id) : cmdId{ -1 }, desc{ desc }, id{ id } {}

void Action::setEnabled(bool enabled) { 
    enable = enabled; 
}

int Action::isEnabled() const { 
    return enable; 
}

char const* Action::description() {
    return desc.c_str();
}

char const* Action::identifier() {
    return id.c_str();
}

SimpleAction::SimpleAction(std::string desc, std::string id, std::function<void (ReaperAPI &)> func) :
    Action(desc, id),
    func{ func } {}

void SimpleAction::operator()(ReaperAPI& api) { func(api); }

bool hookCommand2ProcEx(KbdSectionInfo *sec, int command, int val, int valhw, int relmode, HWND hwnd) {
    return ActionManager::getManager(nullptr).processCommandHook(command);
}

bool hookCommandProcEx(int command, int flag) {
    return ActionManager::getManager(nullptr).processCommandHook(command);
}

int toggleactioncallback(int commandId) {
    return ActionManager::getManager(nullptr).processToggleActionCallback(commandId);
}


ActionManager::ActionManager(std::shared_ptr<ReaperAPI> api) : api_ptr{api}
{
    if(!(this->api_ptr)) {
       throw std::logic_error("api is null, make sure when getManager() is first called, a valid API pointer is provided.");
    }
    auto ret = this->api_ptr->Register("hookcommand2", reinterpret_cast<void*>(hookCommand2ProcEx));
    if(!ret) {
        throw std::runtime_error("cannot register hook command 2");
    }
    ret = this->api_ptr->Register("hookcommand", reinterpret_cast<void*>(hookCommandProcEx));
    if (!ret) {
      throw std::runtime_error("cannot register hook command");
    }
}

bool ActionManager::processCommandHook(int command)
{
  auto iter = action_map.find(command);
  if (iter != std::end(action_map)) {
    auto& action = iter->second;
    (*action)(*api_ptr);
    return true;
  }
  return false;
}

int ActionManager::processToggleActionCallback(int command)
{
    auto iter = action_map.find(command);
    if(iter != std::end(action_map)) {
        auto& action = iter->second;
        return action->isEnabled();
    } else {
        return -1;
    }
}

std::shared_ptr<ActionIdentifier> ActionManager::addAction(std::shared_ptr<Action> action)
{
    gaccel_register_t accel;
    accel.accel = {0, 0, 0};
    accel.desc = action->description();
    auto commandId = api_ptr->Register("command_id", reinterpret_cast<void*>(const_cast<char*>(action->identifier())));
    accel.accel.cmd = static_cast<unsigned short>(commandId);
    api_ptr->Register("gaccel", reinterpret_cast<void*>(&(accel)));
    action_map.insert(std::make_pair(commandId, action));
    return std::make_shared<RegisteredAction>(action, commandId);
}

ActionManager &ActionManager::getManager(std::shared_ptr<ReaperAPI> api)
{
    static ActionManager actionMan {api};
    return actionMan;
}

ActionManager &ActionManager::getManager()
{
    return getManager(nullptr);
}

RegisteredAction::RegisteredAction(std::shared_ptr<Action> action, long id) : action{action}, id{id}
{
    assert(action);
}

int RegisteredAction::getId() const
{
    return static_cast<int>(id);
}

void RegisteredAction::setEnabled(bool enable)
{
    action->setEnabled(enable);
}

int RegisteredAction::isEnabled() const
{
    return action->isEnabled();
}




