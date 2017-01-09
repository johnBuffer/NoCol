#include "DynamicBlur.h"

int log2(int i)
{
    int targetlevel = 0;
    while (i >>= 1) ++targetlevel;
    return targetlevel;
}

void applyBlur(sf::RenderTexture& texture, sf::Shader& shader)
{
    shader.setParameter("DIRECTION", 1.0);
    texture.draw(sf::Sprite(texture.getTexture()), &shader);
    shader.setParameter("DIRECTION", 0.0);
    texture.draw(sf::Sprite(texture.getTexture()), &shader);
}

DynamicBlur::DynamicBlur(unsigned int textureWidth, unsigned int textureHeight) :
    __downSizeFactor(8.0),
    __WIDTH(textureWidth),
    __HEIGHT(textureHeight)
{
    __blur.loadFromFile("dblur.frag", sf::Shader::Fragment);
    __blur.setParameter("WIDTH", __WIDTH);
    __blur.setParameter("HEIGHT", __HEIGHT);

    __blurTexture.create(__WIDTH, __HEIGHT);
    __lowBlurTexture.create(__WIDTH, __HEIGHT);
}

const sf::Texture& DynamicBlur::operator()(const sf::Texture& inputTexture)
{
    sf::Sprite downscaleSprite(inputTexture);
    downscaleSprite.setScale(0.5, 0.5);
    __blurTexture.draw(downscaleSprite);

    __blur.setParameter("SCALE", 2);
    for (int i(2); i--;)
        applyBlur(__blurTexture, __blur);

    __blurTexture.display();

    sf::Sprite downscaledSprite1(__blurTexture.getTexture());
    downscaledSprite1.setScale(2/float(__downSizeFactor), 2/float(__downSizeFactor));
    __lowBlurTexture.draw(downscaledSprite1);
    __lowBlurTexture.display();
    __blurTexture.draw(sf::Sprite(__lowBlurTexture.getTexture()));

    sf::Sprite borderSprite(__lowBlurTexture.getTexture());
    borderSprite.setPosition(__WIDTH/__downSizeFactor, 0);
    __blurTexture.draw(borderSprite);
    borderSprite.setPosition(0, __HEIGHT/__downSizeFactor);
    __blurTexture.draw(borderSprite);

    int i = __downSizeFactor*2;
    while (i >>= 1 > 0.5)
    {
        __blur.setParameter("SCALE", 1/float(i));
        for (int k(log2(i)); k--;)
            applyBlur(__blurTexture, __blur);

        if (i-1)
        {
            sf::Sprite upscale(__blurTexture.getTexture());
            upscale.scale(2, 2);
            __lowBlurTexture.draw(upscale);
            __blurTexture.draw(sf::Sprite(__lowBlurTexture.getTexture()));
        }
    }
    __blurTexture.display();

    return __blurTexture.getTexture();
}
