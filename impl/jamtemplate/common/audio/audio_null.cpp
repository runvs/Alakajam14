#include "audio_null.hpp"
#include "sound_null.hpp"

namespace jt {
void AudioNull::update() { }
oalpp::SoundContextInterface& AudioNull::getContext() { return m_context; }
void AudioNull::addTemporarySound(std::weak_ptr<SoundInterface> snd) { }
void AudioNull::addPermanentSound(
    std::string const& identifier, std::shared_ptr<SoundInterface> snd)
{
}

std::shared_ptr<SoundInterface> AudioNull::getSoundFromSoundPool(std::string const& baseIdentifier,
    std::function<std::shared_ptr<SoundInterface>()> const& function, std::size_t count)
{
    return std::make_shared<jt::SoundNull>();
}

std::shared_ptr<SoundInterface> AudioNull::getPermanentSound(std::string const& identifier)
{
    return std::make_shared<jt::SoundNull>();
}

void AudioNull::removePermanentSound(std::string const& identifier) { }

} // namespace jt
