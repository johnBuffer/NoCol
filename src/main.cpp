#include <iostream>
#include "object.hpp"
#include "framework/window_context_handler.hpp"

int32_t main()
{
    const sf::Vector2u window_size{1600, 900};
    WindowContextHandler app("NoCol", window_size, window_size);

    while (app.run()) {

    }

    return 0;
}


