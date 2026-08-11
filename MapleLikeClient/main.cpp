#include <SFML/Graphics.hpp>
#include <optional>

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), "MapleLike RPG");

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear();

        sf::CircleShape circle(50.f);
        circle.setPosition({ 100.f, 100.f });

        window.draw(circle);

        window.display();
    }

    return 0;
}