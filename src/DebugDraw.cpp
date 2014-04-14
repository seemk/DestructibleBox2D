#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <Box2D/Dynamics/b2Body.h>
#include <Box2D/Dynamics/b2World.h>
#include "Constants.h"
#include "DebugDraw.h"

using namespace constants;

namespace
{
	sf::Color convertColor(const b2Color& color)
	{
		return{ static_cast<sf::Uint8>(color.r * 255),
				static_cast<sf::Uint8>(color.g * 255),
				static_cast<sf::Uint8>(color.b * 255) };
	}
}


DebugDraw::DebugDraw(sf::RenderWindow* win, b2World* world)
	: window(win)
{
	if (world)
	{
		world->SetDebugDraw(this);
		SetFlags(b2Draw::e_shapeBit);
	}
}

void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	drawPolyImpl(vertices, vertexCount, color, false);
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	drawPolyImpl(vertices, vertexCount, color, true);
}

void DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
	float ScaledRadius = radius * RENDER_SCALE;
	sf::CircleShape circle(radius * RENDER_SCALE);
	circle.setPointCount(4);
	circle.setPosition({ center.x * RENDER_SCALE - ScaledRadius, center.y * RENDER_SCALE - ScaledRadius });

	window->draw(circle);
}

void DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
	float RENDER_SCALEd_radius = radius * RENDER_SCALE;
	sf::CircleShape circle(RENDER_SCALEd_radius, 32);
	circle.setFillColor(convertColor(color));

	circle.setPosition({ center.x * RENDER_SCALE - RENDER_SCALEd_radius,
						 center.y * RENDER_SCALE - RENDER_SCALEd_radius });

	window->draw(circle);
	//DrawSegment(center, center + radius*axis, b2Color{ 1.0f, 0.0f, 0.0f });
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	auto convertedColor = convertColor(color);
	sf::Vertex line[] =
	{
		{ { p1.x * RENDER_SCALE, p1.y * RENDER_SCALE }, convertedColor },
		{ { p2.x * RENDER_SCALE, p2.y * RENDER_SCALE }, convertedColor }
	};

	window->draw(line, 2, sf::Lines);
}

void DebugDraw::DrawTransform(const b2Transform& transform)
{
	b2Vec2 p1 = transform.p, p2;
	const float32 k_axisScale = 0.4f;

	p2 = p1 + k_axisScale * transform.q.GetXAxis();
	DrawSegment(p1, p2, b2Color(1, 0, 0));

	p2 = p1 + k_axisScale * transform.q.GetYAxis();
	DrawSegment(p1, p2, b2Color(0, 1, 0));
}

void DebugDraw::drawPolyImpl(const b2Vec2* vertices, int32 count, const b2Color& color, bool filled)
{
	sf::ConvexShape shape;
	shape.setPointCount(count);
	auto transformed_color = convertColor(color);

	if (!filled)
	{
		shape.setOutlineThickness(1.0f);
		shape.setOutlineColor(transformed_color);
		transformed_color.a = 0;
	}

	shape.setFillColor(transformed_color);

	for (int i = 0; i < count; ++i)
	{
		shape.setPoint(i, { vertices[i].x * RENDER_SCALE,
							vertices[i].y * RENDER_SCALE });
	}

	window->draw(shape);
}
