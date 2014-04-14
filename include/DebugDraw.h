#ifndef DEBUG_RENDERER_H
#define DEBUG_RENDERER_H

#include <Box2D/Common/b2Draw.h>

namespace sf
{
	class RenderWindow;
}

class b2World;

class DebugDraw : public b2Draw
{

public:

	DebugDraw(sf::RenderWindow* win, b2World* world = nullptr);

	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)override;
	void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) override;
	void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) override;
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
	void DrawTransform(const b2Transform& xf) override;

private:

	void drawPolyImpl(const b2Vec2* vertices, int32 count, const b2Color& color, bool filled = false);

	sf::RenderWindow* window = nullptr;
};

#endif
