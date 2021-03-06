#ifndef ALAKAJAM14_GRAPHICS_COMPONENT_INTERFACE_HPP
#define ALAKAJAM14_GRAPHICS_COMPONENT_INTERFACE_HPP

#include "render_target.hpp"
#include "vector.hpp"

class GraphicsComponentInterface {
public:
    virtual ~GraphicsComponentInterface() = default;
    virtual void updateGraphics(float elapsed) = 0;
    virtual void setPosition(jt::Vector2f const& position) = 0;
    virtual void draw(std::shared_ptr<jt::RenderTarget> target) = 0;
    virtual void flash(float time, jt::Color const& color) = 0;
    virtual bool setAnimationIfNotSet(std::string const& newAnimationName) = 0;
    virtual void setPlayerAnimationLooping(bool isLooping) = 0;
    virtual void setUnderlayAnimation(std::string const& animationName) = 0;

    virtual std::string getCurrentAnimation() const = 0;
};

#endif // ALAKAJAM14_GRAPHICS_COMPONENT_INTERFACE_HPP
