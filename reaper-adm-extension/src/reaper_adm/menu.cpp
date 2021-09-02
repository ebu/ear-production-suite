#include "menu.h"
#include <algorithm>
#include <cassert>
#include <WDL/swell/swell.h>
#include "reaperapi.h"
#include <boost/algorithm/string.hpp>
#include <optional>

using namespace admplug;
using namespace admplug::detail;

namespace {
    std::string bufferToString(char const *buffer) { assert(buffer); return buffer; }

#ifdef WIN32
    using GetMenuItemInfoT = MENUITEMINFOW;
    auto GetMenuItemInfoFn = GetMenuItemInfoW;

    std::string bufferToString(wchar_t const* wstr) {
        int count = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
        std::vector<char> buffer(count, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &buffer[0], count, NULL, NULL);
        return bufferToString(buffer.data());
    }
#else
    using GetMenuItemInfoT = MENUITEMINFO;
    auto GetMenuItemInfoFn = GetMenuItemInfo;
#endif

    using CharT = std::remove_pointer_t<decltype(std::declval<GetMenuItemInfoT>().dwTypeData)>;
    using BufferT = std::vector<CharT>;

    enum class ReaperMenuState {
        Initialising = 0,
        AboutToShow = 1
    };

    MENUITEMINFO getStringMenuInfoStruct(char* text, std::size_t size = 0) {
        MENUITEMINFO info{};
        info.cbSize = sizeof(MENUITEMINFO);
        info.fType = MFT_STRING;
        info.fMask = MIIM_TYPE | MIIM_SUBMENU;
        info.dwTypeData = text;
        info.cch = static_cast<int>(size);
        return info;
    }

    std::optional<std::string> getMenuString(HMENU menu, int position) {
      GetMenuItemInfoT info{};
      info.cbSize = sizeof(GetMenuItemInfoT);
      info.fType = MFT_STRING;
      info.fMask = MIIM_TYPE;
      // nullptr here means query string size & store in info.cch
      info.dwTypeData = nullptr;
      auto success = GetMenuItemInfoFn(menu, position, true, &info);
      if (success) {
        // query doesn't seem to work with WDL shim, 255 bytes should be enough
        // for a short menu string. Truncates if too short.
        if(info.cch == 0) {
          info.cch = 255;
        }
        // returned size does not include null terminator so increment
        BufferT buffer(++info.cch, '\0');
        info.dwTypeData = &buffer[0];
        // get actual string data
        success = GetMenuItemInfoFn(menu, position, true, &info);
        if(success) {
          return bufferToString(buffer.data());
        }
      }
      return {};
    }
    bool supportsLocalisation(ReaperAPI const &api) {
      return std::stof(api.GetAppVersion()) >= 6.11f;
    }

    std::string localise(std::string const &name, std::string const &section,
                         ReaperAPI const &api) {
      return api.LocalizeString(name.c_str(), section.c_str(), 0);
    }

    void checkDefaultItemIndex(int defaultIndex, int actualIndex) {
      assert(defaultIndex == actualIndex);
    }

    void checkDefaultItemIndex(std::optional<int> defaultIndex, int actualIndex) {
      if(defaultIndex) {
        checkDefaultItemIndex(*defaultIndex, actualIndex);
      }
    }

    int findPositionOfItemWithText(HMENU menu, std::string textOrig) {
      boost::erase_all(textOrig, "&");
      auto itemCount = GetMenuItemCount(menu);
      for (int itemPosition = 0; itemPosition != itemCount; ++itemPosition) {
        if (auto menuText = getMenuString(menu, itemPosition)) {
          boost::erase_all(*menuText,
                           "&"); // Win returns amp preceding kbd shortcut
                                 // char - strip it for comparison
          if (*menuText == textOrig) {
            return itemPosition;
          }
        }
      }
      return -1;
    }

    int constrain(int val, int max) {
      return std::max<int>(std::min<int>(val, max), 0);
    }

    int findInsertionPosition(HMENU menu, std::string itemName, int offset, std::optional<int> fallbackPosition) {
      auto itemCount = GetMenuItemCount(menu);
      auto index = findPositionOfItemWithText(menu, itemName);
      if (index >= 0) {
        checkDefaultItemIndex(fallbackPosition, index);
        return constrain(itemCount, index + offset);
      }
      return fallbackPosition ? constrain(*fallbackPosition + offset, itemCount)
                              : itemCount;
    }

    HMENU findHmenuOfItemWithText(HMENU menu, std::string text) {
      auto position = findPositionOfItemWithText(menu, text);
      if (position >= 0) {
        return GetSubMenu(menu, position);
      }
      return nullptr;
    }

    HMENU findHmenuOfItemWithPosition(HMENU menu, int itemPosition) {
        auto itemCount = GetMenuItemCount(menu);
        if (itemPosition < (itemCount - 1)) {
            return GetSubMenu(menu, itemPosition);
        }
        return nullptr;
    }
}

Insertable::Insertable(std::shared_ptr<MenuItem> item, std::shared_ptr<MenuInserter> inserter) :
    item{item},
    inserter{inserter}
{}

Insertable::Insertable(const Insertable &other) : item{other.item->clone()}, inserter{other.inserter}
{

}

Insertable &Insertable::operator=(const Insertable &other)
{
    item = other.item->clone();
    inserter = other.inserter;
    return *this;
}

void Insertable::addTo(HMENU menu) {
    auto index = inserter->getIndex(menu);
    item->addTo(menu, index);
}

void Insertable::update(HMENU menu)
{
    item->update(menu);
}

MenuAction::MenuAction(std::string menuText, std::shared_ptr<ActionIdentifier> action) :
    action{action},
    text{menuText.begin(), menuText.end()},
    isInserted{false}
{
    text.push_back('\0');
    if(action == nullptr) {
        throw(std::runtime_error("Null action cannot be added to a menu"));
    }
}

void MenuAction::addTo(HMENU menu, int position) {
    auto id = action->getId();
    if(!isInserted && id >= 0) {
        auto info = getStringMenuInfoStruct(&text[0], text.size());
        info.fMask |= MIIM_ID;
        info.wID = static_cast<unsigned int>(action->getId());
        InsertMenuItem(menu, position, true, &info);
    }
}

void MenuAction::update(HMENU menu)
{
    if (updateCallback) updateCallback(*this);
    MENUITEMINFO info{};
    info.cbSize = sizeof(MENUITEMINFO);
    info.fMask = MIIM_ID | MIIM_STATE;
    if(action->isEnabled()) {
        info.wID = static_cast<unsigned int>(action->getId());
        info.fState = MFS_ENABLED;
        auto pos = findPositionOfItemWithText(menu, text.data());
        SetMenuItemInfo(menu, pos, true, &info);
    } else {
        info.wID = 0;
        info.fState = MFS_GRAYED;
        SetMenuItemInfo(menu, action->getId(), false, &info);
    }
}

void MenuAction::setEnabled(bool enabled)
{
    action->setEnabled(enabled);
}

std::unique_ptr<MenuItem> MenuAction::clone() const
{
    return std::make_unique<MenuAction>(*this);
}

ReaperMenu::ReaperMenu(std::string menuId) : id{menuId}
{

}

void ReaperMenu::init(std::string menuId, HMENU menu) {
    if(id == menuId) {
        for(auto& item : items) {
            item.addTo(menu);
        }
    }
}

void ReaperMenu::update(std::string menuId, HMENU menu)
{
    if(id == menuId) {
        for(auto& item : items) {
            item.update(menu);
        }
    }
}

void ReaperMenu::insert(std::unique_ptr<MenuItem> item, std::shared_ptr<MenuInserter> inserter)
{
    items.emplace_back(std::move(item), inserter);
}

RawMenu::RawMenu(HMENU menuHandle) : hMenu{menuHandle}
{
}

void RawMenu::init(std::string, HMENU)
{
    init();
}

void RawMenu::init()
{
    for(auto& item : items) {
        item.addTo(hMenu);
    }
}

std::shared_ptr<RawMenu> RawMenu::getMenuByText(std::string menuText,
                                                std::string section,
                                                int fallbackPosition,
                                                ReaperAPI const &api) {
  if (std::stof(api.GetAppVersion()) >= 6.11f) {
    auto localised = localise(menuText, section, api);
    if(!localised.empty()) {
      menuText = localised;
}
  }
  auto submenu = findHmenuOfItemWithText(hMenu, menuText);
  if(submenu && fallbackPosition >= 0) {
    checkDefaultItemIndex(fallbackPosition, positionOfItemWithText(menuText));
  }
  if(!submenu) {
    submenu = findHmenuOfItemWithPosition(hMenu, fallbackPosition);
  }
  if(!submenu) return nullptr;
  return std::make_shared<RawMenu>(submenu);
}

void RawMenu::update(std::string, HMENU)
{
    for(auto& item : items) {
        item.update(hMenu);
    }
}

void RawMenu::insert(std::unique_ptr<MenuItem> item, std::shared_ptr<MenuInserter> inserter)
{
    items.emplace_back(std::move(item), inserter);
}

int RawMenu::positionOfItemWithText(std::string text) const {
  return findPositionOfItemWithText(hMenu, std::move(text));
}

SubMenu::SubMenu(std::string menuText) :
    text{menuText.begin(), menuText.end()},
    isInserted{false}
{
    text.push_back('\0');
}

void SubMenu::addTo(HMENU menu, int position)
{
    if (!isInserted) {
        auto menuInfo = getStringMenuInfoStruct(&text[0]);
        auto submenu = CreatePopupMenu();
        menuInfo.hSubMenu = submenu;
        menuInfo.fMask |= MIIM_SUBMENU;
        InsertMenuItem(menu, position, true, &menuInfo);
        for (auto& item : items) {
            item.addTo(submenu);
        }
        isInserted = true;
    }
}

void SubMenu::insert(std::unique_ptr<MenuItem> item, std::shared_ptr<MenuInserter> inserter)
{
    items.emplace_back(std::move(item), inserter);
}

void SubMenu::update(HMENU menu)
{
    auto pos = findPositionOfItemWithText(menu, text.data());
    if (pos >= 0) {
        if (updateCallback) updateCallback(*this);
        if(isEnabled) {
            EnableMenuItem(menu, pos, MF_BYPOSITION);
        } else {
            EnableMenuItem(menu, pos, MF_BYPOSITION | MFS_GRAYED);
        }
        MENUITEMINFO menuInfo{};
        menuInfo.cbSize = sizeof(MENUITEMINFO);
        menuInfo.fMask = MIIM_SUBMENU;
        auto success = GetMenuItemInfo(menu, pos, true, &menuInfo);
        if (success) {
            for (auto& item : items) {
                item.update(menuInfo.hSubMenu);
            }
        }
    }

}

void SubMenu::setEnabled(bool enabled)
{
    isEnabled = enabled;
}

std::unique_ptr<MenuItem> SubMenu::clone() const
{
    return std::make_unique<SubMenu>(*this);
}


void hookCustomMenuProcEx(const char* menuidstr, HMENU menu, int flag) {
  MenuManager::getManager(nullptr, nullptr).processMenuHook(menuidstr, menu, flag);
}

MenuManager::MenuManager(const ReaperAPI *api, reaper_plugin_info_t *rec)
{
    if (!api) {
        throw std::logic_error("api is null, make sure when getManager() is first called, a valid API pointer is provided.");
    }
    if (!rec) {
        throw std::logic_error("rec is null, make sure when getManager() is first called, a valid rec pointer is provided.");
    }
    auto ret = api->Register("hookcustommenu", reinterpret_cast<void*>(hookCustomMenuProcEx));
    if (!ret) {
        throw std::runtime_error("cannot register hookcustommenu");
    }
    auto main_hwnd = rec->hwnd_main;
    mainMenu = GetMenu(main_hwnd);
}

std::shared_ptr<TopLevelMenu> MenuManager::getReaperMenu(MenuID menuId)
{
    std::shared_ptr<TopLevelMenu> menu;
    if (menuId == MenuID::MAIN_MENU) {
        menu = std::make_shared<RawMenu>(mainMenu);
        menus.push_back(menu);
    }
    else {
        menu = std::make_shared<ReaperMenu>(menuIdentifiers[menuId]);
        menus.push_back(menu);
    }
    return menu;
}

void MenuManager::processMenuHook(std::string menuId, HMENU menu, int flag)
{
    if(ReaperMenuState(flag) == ReaperMenuState::Initialising) {
      for(auto& topMenu : menus) {
         topMenu->init(menuId, menu);
      }
    }
    if(ReaperMenuState(flag) == ReaperMenuState::AboutToShow) {
      for(auto& topMenu : menus) {
          topMenu->update(menuId, menu);
      }
    }
}

MenuManager& MenuManager::getManager(const ReaperAPI *api, reaper_plugin_info_t *rec) {
    static MenuManager manager(api, rec);
    return manager;
}

StartOffset::StartOffset(std::size_t offset) : offset{offset}
{
}

int StartOffset::getIndex(HMENU menu) const
{
    auto itemCount = GetMenuItemCount(menu);
    auto position = std::min<int>(itemCount, static_cast<int>(offset));
    return position;
}

EndOffset::EndOffset(std::size_t offset) : offset{offset}
{
}

int EndOffset::getIndex(HMENU menu) const
{
    return std::max<int>(GetMenuItemCount(menu) - static_cast<int>(offset), 0);
}

AfterNamedItem::AfterNamedItem(std::string itemName) : itemName{itemName}
{
}



AfterNamedItem::AfterNamedItem(std::string itemName, std::string section,
                               int fallbackPosition, ReaperAPI const &api)
    : itemName{supportsLocalisation(api) ? localise(itemName, section, api)
                                         : itemName},
      fallbackPosition{fallbackPosition} {}

int AfterNamedItem::getIndex(HMENU menu) const
{
    return findInsertionPosition(menu, itemName, 1, fallbackPosition);
}

BeforeNamedItem::BeforeNamedItem(std::string itemName) : itemName{itemName}
{

}

BeforeNamedItem::BeforeNamedItem(std::string itemName, std::string section,
                               int fallbackPosition, ReaperAPI const &api)
    : itemName{supportsLocalisation(api) ? localise(itemName, section, api)
                                         : itemName},
      fallbackPosition{fallbackPosition} {}

int BeforeNamedItem::getIndex(HMENU menu) const
{
  return findInsertionPosition(menu, itemName, 0, fallbackPosition);
}
