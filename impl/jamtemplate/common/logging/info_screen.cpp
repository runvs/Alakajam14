#include "info_screen.hpp"
#include "audio/sound.hpp"
#include "game_interface.hpp"
#include "imgui.h"

namespace jt {
void InfoScreen::doCreate()
{
    m_frameTimesVector.resize(m_frameTimes.size());
    m_GameObjectAliveCountVector.resize(m_GameObjectAliveCount.size());
}
void InfoScreen::doUpdate(float const elapsed)
{
#ifdef JT_ENABLE_DEBUG
    if (getGame()->input().keyboard()->justPressed(jt::KeyCode::End)) {
        m_showInfo = !m_showInfo;
    }

    m_frameTimes.push(elapsed);
    auto const pushIndex = m_frameTimes.getPushIndex();
    for (auto index = pushIndex; index != pushIndex + m_frameTimes.size(); ++index) {
        m_frameTimesVector[index - pushIndex] = m_frameTimes[index];
    }

    m_GameObjectAliveCount.push(static_cast<float>(getNumberOfAliveGameObjects()));
    auto const pushIndex2 = m_GameObjectAliveCount.getPushIndex();
    for (auto index = pushIndex2; index != pushIndex2 + m_GameObjectAliveCount.size(); ++index) {
        m_GameObjectAliveCountVector[index - pushIndex2] = m_GameObjectAliveCount[index];
    }
#endif
}
void InfoScreen::doDraw() const
{
    if (!m_showInfo) {
        return;
    }
    ImGui::Begin("Debug Info");
    if (!ImGui::CollapsingHeader("Textures")) {
        std::string textures = "# Textures stored: "
            + std::to_string(getGame()->gfx().textureManager().getNumberOfTextures());
        ImGui::Text(textures.c_str());
    }
    if (!ImGui::CollapsingHeader("Performance")) {

        ImGui::PlotLines("Frame Time [s]", m_frameTimesVector.data(),
            static_cast<int>(m_frameTimesVector.size()), 0, nullptr, 0, FLT_MAX, ImVec2 { 0, 100 });
    }
    if (!ImGui::CollapsingHeader("GameState")) {
        auto const state = getGame()->stateManager().getCurrentState();
        ImGui::Text(state->getName().c_str());
        std::string const gameStateAge
            = "Age: " + jt::MathHelper::floatToStringWithXDigits(state->getAge(), 2) + " s";
        ImGui::Text(gameStateAge.c_str());

        std::string const gameObjectsInThisStateText
            = "# GameObjects (in state): " + std::to_string(state->getNumberOfObjects());
        ImGui::Text(gameObjectsInThisStateText.c_str());

        std::string const totalGameObjectsText
            = "# GameObjects (total): " + std::to_string(getNumberOfAliveGameObjects());
        ImGui::Text(totalGameObjectsText.c_str());

        std::string const createdGameObjectsText
            = "# GameObjects (created): " + std::to_string(getNumberOfCreatedGameObjects());
        ImGui::Text(createdGameObjectsText.c_str());

        ImGui::PlotLines("AliveGameObjects [#] = %s", m_GameObjectAliveCountVector.data(),
            static_cast<int>(m_GameObjectAliveCountVector.size()), 0, nullptr, 0, FLT_MAX,
            ImVec2 { 0, 100 });
    }
    if (!ImGui::CollapsingHeader("Sound")) {
        std::string const createdSoundsText
            = "# Sounds (created): " + std::to_string(jt::Sound::createdObjects());
        std::string const aliveSoundsText
            = "# Sounds (alive): " + std::to_string(jt::Sound::aliveObjects());

        ImGui::Text(createdSoundsText.c_str());
        ImGui::Text(aliveSoundsText.c_str());
    }
    ImGui::End();
}
} // namespace jt
