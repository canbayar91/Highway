#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include "Angel.h"

namespace Angel {

	typedef vec3 color3;

	const int LANE_COUNT = 13;
	const int PAVEMENT_COUNT = 6;
	const int SCORE_LIMIT = 100;

	const GLint WIDTH = 500;
	const GLint HEIGHT = 600;

	const GLint LANE_HEIGHT = 25;
	const GLint OBJECT_HEIGHT = 18;
	const GLint CENTER = 13;
	const GLint PADDING = 4;

	const GLint SEGMENT_COUNT = 60;
	const GLint RADIUS = 9;

	const GLfloat PI = 3.1416;
	const GLfloat ANGLE_INCREMENT = PI / SEGMENT_COUNT;

	enum Direction {
		LEFT,
		RIGHT,
		UP,
		DOWN
	};

	enum VehicleType {
		CAR,
		TRUCK
	};

	struct Coordinate {
		GLint x;
		GLint y;
	};

	struct Vehicle {
		VehicleType type;
		Direction direction;
		Coordinate coordinate;
		GLint speed;
		color3 color;
	};

	struct Coin {
		Coordinate coordinate;
		bool visible = false;
		int counter = 0;
	};

	struct Player {
		Coordinate vertices[3];
		Direction direction = UP;
		bool visible = true;
	};

	struct Explosion {
		Coordinate coordinate;
		GLint radius;
		color3 color = { 1.0f, 0.0f, 0.0f };
		bool visible = false;
	};
}

#endif // _DEFINITIONS_H_