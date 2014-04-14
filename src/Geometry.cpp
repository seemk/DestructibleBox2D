#include "Geometry.h"
#include <boost/math/constants/constants.hpp>
#include <boost/geometry.hpp>

namespace bg = boost::geometry;

ring_t convertShape(b2Vec2 position, const b2ChainShape* source_shape)
{
	auto vertices = source_shape->m_vertices;
	auto vertexCount = source_shape->m_count;
	ring_t ring = { vertices, vertices + vertexCount };
	std::transform(ring.begin(), ring.end(), ring.begin(), [=](b2Vec2 v){ return v + position; });
	bg::correct(ring);

	return ring;
}

geometry_result_t subtract(const ring_t& source, const ring_t& subtrahend)
{
	geometry_result_t out;
	bg::difference(source, subtrahend, out);
	return out;
}

void simplify(geometry_result_t& rings)
{
    std::transform(rings.begin(), rings.end(), rings.begin(), [](const ring_t& r){
        ring_t simplified;
        bg::simplify(r, simplified, 0.05);
        // Discard self intersecting rings
        return bg::intersects(simplified) ? r : simplified;
    });
}

ring_t makeConvexRing(b2Vec2 position, float radius, int vertices)
{
	ring_t convexRing;
	const float theta = boost::math::constants::two_pi<float>() / static_cast<float>(vertices);

	float c = std::cos(theta);
	float s = std::sin(theta);

	float t = 0.0f;
	float y = 0.0f;
	float x = radius;
	for (float i = 0; i < vertices; i++)
	{
		float v_x = x + position.x;
		float v_y = y + position.y;
		bg::append(convexRing, b2Vec2(v_x, v_y));

		t = x;
		x = c * x - s * y;
		y = s * t + c * y;
	}

	return convexRing;

}
