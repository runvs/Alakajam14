#ifndef GUARD_JAMTEMPLATE_SOUND_WITH_EFFECT_HPP
#define GUARD_JAMTEMPLATE_SOUND_WITH_EFFECT_HPP

#include "oalpp/sound.hpp"
#include "oalpp/sound_data.hpp"
#include "sound_interface.hpp"

namespace jt {

class SoundWithEffect : public SoundInterface {
public:
    SoundWithEffect(std::string const& fileName, oalpp::effects::MonoEffectInterface& effect);
    void update() override;
    bool isPlaying() const override;

    void play() override;
    void stop() override;
    void pause() override;

    float getVolume() const override;
    void setVolume(float newVolume) override;

    void setLoop(bool doLoop) override;
    bool getLoop(void) override;

    float getDuration() const override;
    float getPosition() const override;

    float getBlend() const override;
    void setBlend(float blend) override;

private:
    oalpp::SoundData m_drySoundData;
    oalpp::Sound m_drySound;

    oalpp::SoundDataWithEffect m_wetSoundData;
    oalpp::Sound m_wetSound;

    float m_blend { 0.0f };
    float m_volume { 1.0f };
};

} // namespace jt

#endif // GUARD_JAMTEMPLATE_SOUND_WITH_EFFECT_HPP
