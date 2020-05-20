#ifndef DYNAMICBLUR_H_INCLUDED
#define DYNAMICBLUR_H_INCLUDED

#include <SFML/Graphics.hpp>

class DynamicBlur
{
public:
    DynamicBlur(unsigned int texureWidth, unsigned int texureHeight);
    const sf::Texture& operator()(const sf::Texture&);

    void setFactor(unsigned int i) {__downSizeFactor = pow(2, i);}

private:
    float        __downSizeFactor;
    unsigned int __WIDTH, __HEIGHT;

    sf::RenderTexture __blurTexture;
    sf::RenderTexture __lowBlurTexture;

    sf::Shader __blur;
};

#endif // DYNAMICBLUR_H_INCLUDED
