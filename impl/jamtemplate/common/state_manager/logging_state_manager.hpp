#ifndef GUARD_JAMTEMPLATE_LOGGING_STATE_MANAGER_HPP
#define GUARD_JAMTEMPLATE_LOGGING_STATE_MANAGER_HPP

#include "logging/logger_interface.hpp"
#include "state_manager_interface.hpp"
namespace jt {
class LoggingStateManager : public StateManagerInterface {
public:
    LoggingStateManager(StateManagerInterface& decoratee, LoggerInterface& logger);

    std::shared_ptr<GameState> getCurrentState() override;
    void setTransition(std::shared_ptr<StateManagerTransitionInterface> transition) override;
    void switchState(std::shared_ptr<GameState> newState) override;
    void update(std::weak_ptr<GameInterface> gameInstance, float elapsed) override;
    void draw(std::shared_ptr<RenderTarget> rt) override;

private:
    StateManagerInterface& m_decoratee;
    LoggerInterface& m_logger;
};

} // namespace jt

#endif // GUARD_JAMTEMPLATE_LOGGING_STATE_MANAGER_HPP
