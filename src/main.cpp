#include <SFML/Graphics.hpp>
#include <memory>
#include <Box2D/Box2D.h>
#include "DebugDraw.h"
#include "ShapeFactory.h"
#include "Geometry.h"
#include "Constants.h"
#include <unordered_set>
#include <iostream>

namespace bg = boost::geometry;

typedef std::unordered_set<b2Body*> QueryResult;
typedef std::pair<b2Body*, ring_t> match_t;

struct Shape
{
	enum Category
	{
		normal	     = (1 << 0),
		destructible = (1 << 1)
	};
};

namespace
{
	const float screenWidth = 1280.f;
	const float screenHeight = 720.f;
}

struct WorldQueryCallback : public b2QueryCallback
{
	WorldQueryCallback(b2Shape::Type filter, Shape::Category categoryFilter_)
	: shapeFilter(filter)
	, categoryFilter(categoryFilter_)
	{ }

	bool ReportFixture(b2Fixture* fixture) override
	{
		auto type = fixture->GetShape()->GetType();
		auto fixtureCategory = fixture->GetFilterData().categoryBits;
		if (type == shapeFilter && (categoryFilter & fixtureCategory))
		{
			foundBodies.insert(fixture->GetBody());
		}
		return true;
	}

	QueryResult foundBodies;
	b2Shape::Type shapeFilter;
	Shape::Category categoryFilter;

};

std::vector<std::unique_ptr<b2ChainShape>> convertGeometry(const ring_collection_t& rings)
{
	std::vector<std::unique_ptr<b2ChainShape>> shapes;
	for (const auto& r : rings)
	{
		std::unique_ptr<b2ChainShape> shape{ new b2ChainShape() };
		shape->CreateChain(r.data(), r.size());
		shapes.push_back(std::move(shape));
	}

	return shapes;
}

void step(b2World& world, float dt)
{
	const int maxSteps = 20;
	const float fixedDt = 1.0f / 60.f;
	const float minDt = fixedDt / 10.f;
	int stepsPerformed = 0;
	float frameTime = dt;

	while (frameTime > 0.0f && stepsPerformed < maxSteps)
	{
		float delta = (std::min)(frameTime, fixedDt);
		frameTime -= delta;
		if (frameTime < minDt)
		{
			delta += frameTime;
			frameTime = 0.0f;
		}
		const int velocityIterations = 8;
		const int positionIterations = 3;
		world.Step(delta, velocityIterations, positionIterations);

	}

	world.ClearForces();
}



void addStaticShapes(b2World& world)
{

	ShapeFactory factory { constants::RENDER_SCALE };
	// Add the nondestructible screen edges
	std::vector<b2Vec2> boundaryPoints =
	{ b2Vec2{ 0.0f, 0.0f }, b2Vec2{ 0.0f, screenHeight },
	b2Vec2{ screenWidth, screenHeight }, b2Vec2{ screenWidth, 0.0f } };
	auto boundaryShape = factory.chain(boundaryPoints.data(), boundaryPoints.size());

	b2BodyDef boundaryDef;
	b2Body* boundaryBody = world.CreateBody(&boundaryDef);
	auto boundaryFixture = boundaryBody->CreateFixture(boundaryShape.get(), 0.0f);
	auto filter = boundaryFixture->GetFilterData();
	filter.categoryBits = Shape::normal;
	boundaryFixture->SetFilterData(filter);

	// Add a destructible polygon
	std::vector<b2Vec2> polygonPoints =
	{ b2Vec2{ screenWidth * 0.1f, screenHeight * 0.4f },
	b2Vec2{ screenWidth * 0.1f, screenHeight * 0.95f },
	b2Vec2{ screenWidth * 0.9f, screenHeight * 0.95f },
	b2Vec2{ screenWidth * 0.9f, screenHeight * 0.7f },
	b2Vec2{ screenWidth * 0.4f, screenHeight * 0.4f } };

	auto polygonShape = factory.chain(polygonPoints.data(), polygonPoints.size());
	b2BodyDef polygonDef;
	b2Body* polygonBody = world.CreateBody(&polygonDef);
	auto polygonFixture = polygonBody->CreateFixture(polygonShape.get(), 0.0f);
	filter.categoryBits = Shape::destructible;
	polygonFixture->SetFilterData(filter);
}

void drawMouseIndicator(sf::Vector2f position, float radius, sf::RenderWindow& window)
{
	sf::CircleShape indicator;
	auto circlePos = sf::Vector2f{ position.x - radius, position.y - radius };
	indicator.setPosition(circlePos);
	indicator.setOutlineThickness(1.f);
	indicator.setFillColor(sf::Color{ 0, 0, 0, 0 });
	indicator.setRadius(radius);
	window.draw(indicator);
}

std::unordered_set<b2Body*> queryDestructibleBodies(b2Vec2 position, float radius, const b2World& world)
{
	WorldQueryCallback callback{ b2Shape::e_chain, Shape::destructible };
	b2AABB aabb;
	aabb.lowerBound = { position.x - radius, position.y - radius };
	aabb.upperBound = { position.x + radius, position.y + radius };

	world.QueryAABB(&callback, aabb);

	return callback.foundBodies;
}

template<typename It>
std::vector<match_t> matchBodiesToRings(It begin, It end)
{
	std::vector<match_t> batch;

	std::transform(begin, end, std::back_inserter(batch), [](b2Body* body)
	{
		auto f = body->GetFixtureList();
		auto shape = static_cast<b2ChainShape*>(f->GetShape());
		return std::make_pair(body, convertShape(body->GetWorldCenter(), shape));
	});

	return batch;
}

void processRemoval(b2Vec2 removalPosition, float removalRadius, b2World& world, bool simplifyGeometry)
{
	auto foundBodies = queryDestructibleBodies(removalPosition, removalRadius, world);
	auto batch = matchBodiesToRings(foundBodies.begin(), foundBodies.end());

	// Partition the shapes by area, so that elements to be processed are at the beginning
	auto borderIt = std::partition(batch.begin(), batch.end(), [](const match_t& m) {
		const double areaEpsilon = 0.02;
		return bg::area(m.second) > areaEpsilon;
	});

	// Remove small shapes
	std::for_each(borderIt, batch.end(), [&](const match_t& m) {
		world.DestroyBody(m.first);
	});

	// Subtract the input polygon from each shape returned from the query
	ring_t diff = makeConvexRing(removalPosition, removalRadius, 16);
	boost::geometry::correct(diff);

	typedef std::pair<std::unique_ptr<b2ChainShape>, b2Filter> shape_property_t;
	std::vector<shape_property_t> resultShapes;
	std::for_each(batch.begin(), borderIt, [&](const match_t& m) {
		auto subtractionResult = subtract(m.second, diff);
		// Simplify the results
		if (simplifyGeometry)
		{
			simplify(subtractionResult);
		}
		
		// Convert the rings to b2ChainShapes and add to result shapes
		auto converted = convertGeometry(subtractionResult);

		auto moveBegin = std::make_move_iterator(converted.begin());
		auto moveEnd = std::make_move_iterator(converted.end());
		std::transform(moveBegin, moveEnd, std::back_inserter(resultShapes),
			[&](std::unique_ptr<b2ChainShape> converted) {
			auto filter = m.first->GetFixtureList()->GetFilterData();
			return std::make_pair(std::move(converted), filter);
		});
		
		if (!subtractionResult.empty())
		{
			world.DestroyBody(m.first);
		}
	});

	for (auto&& s : resultShapes)
	{
		b2BodyDef bd;
		b2Body* body = world.CreateBody(&bd);
		auto fixture = body->CreateFixture(s.first.get(), 0.0f);
		fixture->SetFilterData(s.second);
	}
}

int main()
{
	sf::Font font;
	sf::Text overlayText;
	const std::string fontFile = "am.ttf";
	if (font.loadFromFile(fontFile))
	{
		overlayText.setFont(font);
	}
	else
	{
		std::cerr << "Could not find " << fontFile << "\n";
		return 1;
	}
	overlayText.setCharacterSize(10);
	overlayText.setString("Hold left mouse button to modify\nRight mouse button to add objects");

	sf::VideoMode videoMode{ static_cast<unsigned int>(screenWidth),
							 static_cast<unsigned int>(screenHeight) };

	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	sf::RenderWindow window(videoMode, "Box2D modifiable geometry", sf::Style::Default, settings);

	std::unique_ptr<b2World> physicsWorld{ new b2World{ b2Vec2{ 0.0f, 18.0f } } };
	physicsWorld->SetAutoClearForces(false);
	physicsWorld->SetContactListener(nullptr);
	DebugDraw debugDraw(&window, physicsWorld.get());

	addStaticShapes(*physicsWorld);

	sf::Clock clock;

	bool simplifyGeometry = true;
	while (window.isOpen())
	{
		float elapsed = clock.restart().asSeconds();
		sf::Event event;

		auto mousePos = sf::Mouse::getPosition(window);
		auto worldPos = window.mapPixelToCoords(mousePos);
		const float removalRadius = 25.f;
		b2Vec2 position = { worldPos.x, worldPos.y };

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
			if (event.type == sf::Event::MouseButtonPressed && 
				event.mouseButton.button == sf::Mouse::Button::Right)
			{
				ShapeFactory factory{ constants::RENDER_SCALE };
				auto ballShape = factory.circle(position, removalRadius / 2.f);
				b2BodyDef ballDef;
				b2Body* ballBody = physicsWorld->CreateBody(&ballDef);
				auto ballFixture = ballBody->CreateFixture(ballShape.get(), 0.0f);
				ballBody->SetType(b2BodyType::b2_dynamicBody);
				auto filter = ballFixture->GetFilterData();
				filter.categoryBits = Shape::normal;
				filter.maskBits = Shape::normal | Shape::destructible;
				ballFixture->SetFilterData(filter);
			}
			if (event.type == sf::Event::KeyReleased &&
				event.key.code == sf::Keyboard::S)
			{
				simplifyGeometry = !simplifyGeometry;
			}
		}

		window.clear();

		step(*physicsWorld, elapsed);

		b2Vec2 scaledPos = position;
		scaledPos *= (1.f / constants::RENDER_SCALE);
		float scaledRadius = removalRadius / constants::RENDER_SCALE;

		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
		{
			processRemoval(scaledPos, scaledRadius, *physicsWorld, simplifyGeometry);
			drawMouseIndicator(worldPos, removalRadius, window);
		}

		physicsWorld->DrawDebugData();
		window.draw(overlayText);
		window.display();
	}
	return 0;
}
