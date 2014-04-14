#ifndef GEOMETRY_H
#define GEOMETRY_H
#define BOOST_GEOMETRY_OVERLAY_NO_THROW
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <Box2D/Collision/Shapes/b2ChainShape.h>
#include <Box2D/Common/b2Math.h>
#include <vector>

BOOST_GEOMETRY_REGISTER_POINT_2D(b2Vec2, float, boost::geometry::cs::cartesian, x, y)

// Template parameters for boost::geometry::model::ring
// <typename Point, bool ClockWise, bool Closed>
typedef boost::geometry::model::ring<b2Vec2, false, true> ring_t;
typedef std::vector<ring_t> ring_collection_t;

ring_t convertShape(b2Vec2 position, const b2ChainShape* source_shape);


ring_collection_t subtract(const ring_t& source, const ring_t& subtrahend);
void simplify(ring_collection_t& rings);
ring_t makeConvexRing(b2Vec2 position, float radius, int vertices = 8);

#endif
