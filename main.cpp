#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <cmath>
#include <iostream>
#include <fstream>

#include "DynamicBlur.h"

int WIN_WIDTH = 512;
int WIN_HEIGHT = 512;

struct Ball
{
    double x, y, oldX, oldY;
    double vx, vy;
    double r;

    bool stable;
    int stableCount;

    Ball(double arg_x, double arg_y, double arg_r) : x(arg_x), y(arg_y), vx(5), vy(-1), r(arg_r)
    {
        stableCount = 0;
    }

    void save()
    {
        oldX = x;
        oldY = y;
    }
};

bool update(std::vector<Ball>& balls, double speed)
{
    bool stable = true;

    int nBalls = balls.size();

    for (int i(0); i<nBalls; i++)
	{
	    Ball& currentBall = balls[i];
		currentBall.vx += (WIN_WIDTH/2-currentBall.x)*0.001;
		currentBall.vy += (WIN_HEIGHT/2-currentBall.y)*0.001;

		for (int k=0; k<nBalls; k++)
		{
		    if (k == i)
                break;

		    Ball& collider = balls[k];
			double dx = currentBall.x-collider.x;
			double dy = currentBall.y-collider.y;
			double dist = sqrt(dx*dx+dy*dy);

			double minDist = currentBall.r+collider.r;

			if (dist < minDist && dist)
			{
			    stable = false;

			    currentBall.stable = false;
			    collider.stable = false;

				double rx = dx/dist;
				double ry = dy/dist;

				currentBall.x += (minDist-dist)*rx;
				currentBall.y += (minDist-dist)*ry;

				collider.x -= (minDist-dist)*rx;
				collider.y -= (minDist-dist)*ry;
			}
		}
	}

	for (int i(0); i<nBalls; i++)
    {
        if(balls[i].stable)
            balls[i].stableCount++;
        else
            balls[i].stableCount = 0;
    }

	return stable;
}

void updatePos(std::vector<Ball>& balls, double& speedDownFactor, double& speedDownCounter)
{
    for (Ball& currentBall : balls)
    {
        currentBall.x += currentBall.vx/speedDownFactor;
        currentBall.y += currentBall.vy/speedDownFactor;
    }

    speedDownCounter--;
}

int main()
{
    srand(time(0));
    sf::ContextSettings settings;
    settings.antialiasingLevel = 2;
    sf::RenderWindow window(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "TEST", sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);

    double speedDownFactor = 1;
    double speedDownCounter = 1;
    double waitingSpeedFactor = 1;
    double speedDownFactorGoal = 1;
    int iterations = 0;

    bool drawTraces = true;
    bool synccEnable = true;

    sf::Shader shader;
    if (!shader.loadFromFile("shader.frag", sf::Shader::Fragment))
    {
        std::cout << "Shader loading error..." << std::endl;
    }

    sf::Shader drawer;
    if (!drawer.loadFromFile("drawer.frag", sf::Shader::Fragment))
        std::cout << "Erreur" << std::endl;

    drawer.setParameter("WIDTH", WIN_WIDTH);
    drawer.setParameter("HEIGHT", WIN_HEIGHT);

    int nBalls = 0;
    int maxSize = 22;
    int minSize = 2;

    std::ifstream infile;
    infile.open("config");
    std::cout << "Loading config" << std::endl;
    infile >> nBalls;
    infile >> maxSize;
    infile >> minSize;

    std::vector<Ball> balls;
    for (int i(0); i<nBalls; i++)
        balls.push_back(Ball(rand()%WIN_WIDTH, rand()%WIN_HEIGHT, rand()%(maxSize-minSize)+minSize));

    sf::RenderTexture traces, blurTexture, renderer;
    blurTexture.create(WIN_WIDTH, WIN_HEIGHT);
    traces.create(WIN_WIDTH, WIN_HEIGHT);
    renderer.create(WIN_WIDTH, WIN_HEIGHT);

    traces.clear(sf::Color::Black);
    traces.display();
    shader.setParameter("WIDTH", WIN_WIDTH);
    shader.setParameter("HEIGHT", WIN_HEIGHT);

    DynamicBlur dblur(WIN_WIDTH, WIN_HEIGHT);

    while (window.isOpen())
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        sf::Event event;
        while (window.pollEvent(event))
        {
			switch (event.type)
			{
				case sf::Event::KeyPressed:
					if (event.key.code == sf::Keyboard::Escape) window.close();
					else if (event.key.code == sf::Keyboard::Space)
                    {
                        if (speedDownFactor == 1)
                        {
                            speedDownFactorGoal = 20;
                        }
                        else
                        {
                            speedDownFactorGoal = 1;
                        }
                    }
                    else if (event.key.code == sf::Keyboard::A)
                        drawTraces = !drawTraces;
                    else if (event.key.code == sf::Keyboard::E)
                    {
                        synccEnable = !synccEnable;
                        window.setVerticalSyncEnabled(synccEnable);
                    }
                    break;
                case sf::Event::MouseButtonPressed:
                    break;
                case sf::Event::MouseMoved:
                    break;
				default:
                    break;
			}
        }

        if (iterations%5 == 0 && waitingSpeedFactor != speedDownFactorGoal)
        {
            waitingSpeedFactor += speedDownFactorGoal > waitingSpeedFactor ? 1 : speedDownFactorGoal-waitingSpeedFactor;
        }

        bool stable = true;
        if (!speedDownCounter)
        {
            int nBalls = balls.size();
            for (int i(0); i<nBalls; i++)
            {
                balls[i].stable = true;
                balls[i].save();
            }

            stable = update(balls, 1);
            if (waitingSpeedFactor)
            {
                speedDownFactor = waitingSpeedFactor;
            }
            speedDownCounter = speedDownFactor;
        }

        updatePos(balls, speedDownFactor, speedDownCounter);

        renderer.clear(sf::Color::Black);

        if (speedDownCounter == speedDownFactor-1)
        {
            traces.display();
            shader.setParameter("texture", traces.getTexture());
            traces.draw(sf::Sprite(traces.getTexture()), &shader);
            traces.display();

            dblur.setFactor(1);
            blurTexture.draw(sf::Sprite(dblur(traces.getTexture())));
            blurTexture.display();
        }

        if (drawTraces)
        {
            renderer.draw(sf::Sprite(traces.getTexture()));
            renderer.draw(sf::Sprite(blurTexture.getTexture()), sf::BlendAdd);
        }

        for (Ball& b : balls)
        {
            int c = b.stableCount > 255 ? 255 : b.stableCount;
            sf::Color color = sf::Color(255-c, c, 0);
            double r = b.r*c/255.0;

            if (speedDownFactor > 1)
                r = b.r;

            sf::CircleShape ballRepresentation(r);
            ballRepresentation.setFillColor(color);
            ballRepresentation.setOrigin(r, r);
            ballRepresentation.setPosition(b.x, b.y);
            renderer.draw(ballRepresentation);

            sf::VertexArray trace(sf::Lines, 2);
            trace[0].position = sf::Vector2f(b.x, b.y);
            trace[1].position = sf::Vector2f(b.oldX, b.oldY);

            trace[0].color = color;
            trace[1].color = color;

            traces.draw(trace);

            if (!b.stableCount && speedDownCounter == speedDownFactor-1)
            {
                double r = b.r;
                ballRepresentation.setRadius(r);
                ballRepresentation.setOrigin(r, r);
                ballRepresentation.setFillColor(sf::Color::Red);
                traces.draw(ballRepresentation);

                r -= 4;

                ballRepresentation.setRadius(r);
                ballRepresentation.setOrigin(r, r);
                ballRepresentation.setFillColor(sf::Color::Black);
                traces.draw(ballRepresentation);

                r += 2;

                sf::VertexArray circle(sf::LinesStrip, 20);
                for (int i(0); i<20; i++)
                {
                    double pi = 3.14159;
                    double x = b.x + r*sin(2*pi/19.0*i);
                    double y = b.y + r*cos(2*pi/19.0*i);
                    circle[i].position = sf::Vector2f(x, y);
                }

                traces.draw(circle);
            }
        }

        sf::CircleShape stableIndicator(10);
        stableIndicator.setPosition(10, 10);
        stableIndicator.setFillColor(sf::Color::Red);
        if (stable)
            stableIndicator.setFillColor(sf::Color::Green);

        renderer.draw(stableIndicator);
        renderer.display();

        drawer.setParameter("MIN_HEIGHT", WIN_HEIGHT-mousePos.y);
        drawer.setParameter("ORIGINAL_TEXTURE", renderer.getTexture());
        dblur.setFactor(3);
        renderer.draw(sf::Sprite(dblur(renderer.getTexture())), &drawer);
        renderer.display();

        window.draw(sf::Sprite(renderer.getTexture()));
        window.display();

        iterations++;
    }

    return 0;
}
