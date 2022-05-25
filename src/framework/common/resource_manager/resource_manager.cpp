#include "resource_manager.hpp"

ResourceManager::ResID ResourceManager::registerFont(const std::string& filename, const std::string& asset_name)
{
    const ResID id = fonts.size();
    fonts[id] = std::make_unique<sf::Font>();
    fonts[id]->loadFromFile(filename);
    name_to_id_fonts[asset_name] = id;
    return id;
}

ResourceManager::ResID ResourceManager::registerTexture(const std::string&  filename, const std::string& asset_name)
{
    const ResID id = textures.size();
    textures[id] = std::make_unique<sf::Texture>();
    textures[id]->loadFromFile(filename);
    name_to_id_textures[asset_name] = id;
    return id;
}

ResourceManager::ResID ResourceManager::registerImage(const std::string &filename, const std::string& asset_name)
{
    const ResID id = images.size();
    images[id] = std::make_unique<sf::Image>();
    images[id]->loadFromFile(filename);
    name_to_id_images[asset_name] = id;
    return id;
}

sf::Font& ResourceManager::getFont(ResourceManager::ResID id)
{
    // No check for now
    return *fonts[id];
}

sf::Texture& ResourceManager::getTexture(ResourceManager::ResID id)
{
    // No check for now
    return *textures[id];
}

sf::Image &ResourceManager::getImage(ResourceManager::ResID id)
{
    return *images[id];
}

ResourceManager::ResID ResourceManager::getImageID(const std::string &name)
{
    return name_to_id_images[name];
}

sf::Font &ResourceManager::getFont(const std::string &name)
{
    return *fonts[name_to_id_fonts[name]];
}

sf::Texture &ResourceManager::getTexture(const std::string &name)
{
    return *textures[name_to_id_textures[name]];
}

sf::Image &ResourceManager::getImage(const std::string &name)
{
    return *images[name_to_id_images[name]];
}
