#ifndef ShapeFactory_H
#define ShapeFactory_H

#include <Box2D/Common/b2Math.h>
#include <vector>
#include <memory>

class b2ChainShape;
class b2PolygonShape;
class b2CircleShape;

class ShapeFactory
{
public:

	ShapeFactory() { }
	ShapeFactory(float pixelsToMeter) { scale = 1.0f / pixelsToMeter; }

	std::unique_ptr<b2ChainShape> chain(const b2Vec2* points, int count, bool closed = true);
	std::unique_ptr<b2PolygonShape> polygon(const b2Vec2* points, int count);
	std::unique_ptr<b2PolygonShape> polygon_rect(b2Vec2 position, float width, float height, float angle = 0.0f);
	std::unique_ptr<b2CircleShape> circle(b2Vec2 position, float radius);

private:

	void copyScaled(const b2Vec2* points, int count);

	std::vector<b2Vec2> buffer;
	float scale = 1.0f / 30.f;

};


#endif
