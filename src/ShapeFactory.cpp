#include "ShapeFactory.h"
#include <Box2D/Collision/Shapes/b2ChainShape.h>
#include <Box2D/Collision/Shapes/b2CircleShape.h>
#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include <algorithm>
#include <iterator>

std::unique_ptr<b2ChainShape> ShapeFactory::chain(const b2Vec2* points, int count, bool closed)
{
	
	copyScaled(points, count);

	auto shape = std::make_unique<b2ChainShape>();

	if (closed)
	{
		shape->CreateLoop(buffer.data(), buffer.size());
	}
	else
	{
		shape->CreateChain(buffer.data(), buffer.size());
	}

	return shape;
}

std::unique_ptr<b2PolygonShape> ShapeFactory::polygon(const b2Vec2* points, int count)
{
	auto shape = std::make_unique<b2PolygonShape>();

	copyScaled(points, count);

	shape->Set(buffer.data(), buffer.size());

	return shape;
}

std::unique_ptr<b2PolygonShape> ShapeFactory::polygon_rect(b2Vec2 position, float width, float height, float angle)
{
	auto shape = std::make_unique<b2PolygonShape>();

	shape->SetAsBox(scale * width * 0.5f, scale * height * 0.5f, { position.x * scale, position.y * scale }, angle);

	return shape;
}

std::unique_ptr<b2CircleShape> ShapeFactory::circle(b2Vec2 position, float radius)
{
	auto shape = std::make_unique<b2CircleShape>();
	shape->m_p = { position.x * scale, position.y * scale };
	shape->m_radius = radius * scale;

	return shape;
}

void ShapeFactory::copyScaled(const b2Vec2* points, int Count)
{
	buffer.clear();

	std::transform(points, points + Count, std::back_inserter(buffer), [=](const b2Vec2& vec)
	{
		return b2Vec2{ vec.x * scale, vec.y * scale };
	});
}
