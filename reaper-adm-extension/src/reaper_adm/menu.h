#pragma once
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include "actionmanager.h"

namespace admplug {

class MenuInserter {
public:
    virtual ~MenuInserter() = default;
    virtual int getIndex(HMENU) const = 0;
};

class MenuItem {
public:
    virtual ~MenuItem() = default;
    virtual void addTo(HMENU menu, int position) = 0;
    virtual void update(HMENU menu) = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual std::unique_ptr<MenuItem> clone() const = 0;
    std::function<void(MenuItem& menuItem)> updateCallback;
};

namespace detail{
class Insertable {
public:
    Insertable(std::shared_ptr<MenuItem> item, std::shared_ptr<MenuInserter> inserter);
    Insertable(Insertable const& other);
    Insertable& operator=(Insertable const& other);
    void addTo(HMENU menu);
    void update(HMENU menu);
private:
    std::shared_ptr<MenuItem> item;
    std::shared_ptr<MenuInserter> inserter;
};
}

class MenuContainer {
public:
    virtual ~MenuContainer() = default;
    virtual void insert(std::unique_ptr<MenuItem> item, std::shared_ptr<MenuInserter> inserter) = 0;
};

class TopLevelMenu : public MenuContainer {
public:
    virtual void init(std::string menuId, HMENU menu) = 0;
    virtual void update(std::string menuId, HMENU menu) = 0;
};

class MenuAction : public MenuItem {
public:
    MenuAction(std::string menuText,
               std::shared_ptr<ActionIdentifier> action);
    void addTo(HMENU menu, int position) override;
    void update(HMENU menu) override;
    void setEnabled(bool enabled) override;
    std::unique_ptr<MenuItem> clone() const override;
private:
    std::shared_ptr<ActionIdentifier> action;
    std::vector<char> text;
    bool isInserted;
};

class SubMenu : public MenuContainer, public MenuItem {
public:
    explicit SubMenu(std::string menuText);
    void addTo(HMENU menu, int position) override;
    void insert(std::unique_ptr<MenuItem> item, std::shared_ptr<MenuInserter> inserter) override;
    void update(HMENU menu) override;
    void setEnabled(bool enabled) override;
    std::unique_ptr<MenuItem> clone() const override;
private:
    std::vector<char> text;
    std::vector<detail::Insertable> items;
    bool isInserted;
    bool isEnabled {true};
};

class ReaperMenu : public TopLevelMenu {
public:
    explicit ReaperMenu(std::string menuId);
    void init(std::string menuId, HMENU menu) override;
    void update(std::string menuId, HMENU menu) override;
    void insert(std::unique_ptr<MenuItem> item, std::shared_ptr<MenuInserter> inserter) override;
private:
    std::string id;
    std::vector<detail::Insertable> items;
};

class RawMenu : public TopLevelMenu {
public:
    explicit RawMenu(HMENU menuHandle);
    void init(std::string menuId, HMENU menu) override;
    void update(std::string menuId, HMENU menu) override;
    void insert(std::unique_ptr<MenuItem> item, std::shared_ptr<MenuInserter> inserter) override;
    std::shared_ptr<admplug::RawMenu> getMenuByText(std::string menuText,
                                                    std::string section,
                                                    int fallbackPosition,
                                                    ReaperAPI const &api);
    int positionOfItemWithText(std::string text) const;
    void init();
private:
    HMENU hMenu;
    std::vector<detail::Insertable> items;
};


class StartOffset : public MenuInserter {
public:
    explicit StartOffset(std::size_t offset);
    int getIndex(HMENU) const override;
private:
    std::size_t offset;
};

class EndOffset : public MenuInserter {
public:
    explicit EndOffset(std::size_t offset);
    int getIndex(HMENU) const override;
private:
    std::size_t offset;
};

class AfterNamedItem : public MenuInserter {
public:
    explicit AfterNamedItem(std::string itemName);
    AfterNamedItem(std::string itemName, std::string section,
                   int fallbackPosition, ReaperAPI const &api);
    int getIndex(HMENU) const override;
private:
    std::string itemName;
    std::optional<int> fallbackPosition;
};

class BeforeNamedItem : public MenuInserter {
public:
    explicit BeforeNamedItem(std::string itemName);
  BeforeNamedItem(std::string itemName, std::string section,
                  int fallbackPosition, ReaperAPI const &api);
    int getIndex(HMENU) const override;
private:
    std::string itemName;
    std::optional<int> fallbackPosition;
};

enum class MenuID {
    MAIN_MENU,
    MEDIA_ITEM_CONTEXT
};

class MenuManager {
public:

    ~MenuManager() = default;
    MenuManager(MenuManager const&) = delete;
    MenuManager(MenuManager&&) = delete;
    MenuManager& operator=(MenuManager const&) = delete;

    std::shared_ptr<TopLevelMenu> getReaperMenu(MenuID menuId);

    void processMenuHook(std::string menuId, HMENU menu, int flag);
    static MenuManager& getManager(ReaperAPI const* api, reaper_plugin_info_t *rec);

private:
    MenuManager(ReaperAPI const* api, reaper_plugin_info_t *rec);
    std::vector<std::shared_ptr<TopLevelMenu>> menus;
    std::map<MenuID, std::string> menuIdentifiers {{
            {MenuID::MEDIA_ITEM_CONTEXT, "Media item context"}
                                                       }};
    HMENU mainMenu;
};
}
