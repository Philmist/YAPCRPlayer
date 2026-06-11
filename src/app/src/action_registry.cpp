#include "action_registry.h"

namespace yapcr::app {

void ActionRegistry::setKeyMap(const QMap<ActionId, QStringList>& keyMap)
{
    keyMap_  = keyMap;
    reverse_ = buildReverseMap(keyMap);
}

void ActionRegistry::on(ActionId id, std::function<void()> fn)
{
    handlers_.insert(id, std::move(fn));
}

bool ActionRegistry::trigger(ActionId id)
{
    const auto it = handlers_.find(id);
    if (it == handlers_.end()) { return false; }
    (*it)();
    return true;
}

bool ActionRegistry::dispatch(const KeyChord& c)
{
    const auto rit = reverse_.find(c);
    if (rit == reverse_.end()) { return false; }
    return trigger(rit.value());
}

QStringList ActionRegistry::keysFor(ActionId id) const
{
    return keyMap_.value(id);
}

} // namespace yapcr::app
