#include "Definitions.h"
#include <time.h>
#include <vector>
#include <mutex>
#include <string>
#include <iostream>

using namespace Angel;

Coordinate pavements[PAVEMENT_COUNT];
Coordinate lanes[LANE_COUNT];

std::vector<Vehicle> vehicles;
std::mutex mutex;

Coin coin;
Player player;
Explosion explosion;

int score = 0;
bool paused = false;
bool gameover = false;

// Forward definitions for callbacks
void timer(int id);
void keyboard(unsigned char key, int x, int y);
void movement(int key, int x, int y);
void mouse(int button, int state, int x, int y);

void create_background() {

	// Store pavement coordinates
	int pavement_locations[PAVEMENT_COUNT] = { 0, 4, 9, 14, 18, 23 };
	for (int i = 0; i < PAVEMENT_COUNT; i++) {
		pavements[i].x = 0;
		pavements[i].y = pavement_locations[i] * LANE_HEIGHT;
	}

	// Store lane coordinates
	int lane_locations[LANE_COUNT] = { 2, 3, 6, 7, 8, 11, 12, 13, 16, 17, 20, 21, 22 };
	for (int i = 0; i < LANE_COUNT; i++) {
		lanes[i].x = 0;
		lanes[i].y = lane_locations[i] * LANE_HEIGHT;
	}
}

void skip_pavement(int &lane) {

	// Skips the pavement location to put object into lane
	if (lane <= 2) {
		lane++;
	} else if (lane >= 3 && lane <= 6) {
		lane += 2;
	} else if (lane >= 7 && lane <= 10) {
		lane += 3;
	} else if (lane >= 11 && lane <= 13) {
		lane += 4;
	} else {
		lane += 5;
	}
}

bool check_vehicle_collision(Vehicle &vehicle) {

	// Decide on vehicle length according to its type
	int length = OBJECT_HEIGHT;
	if (vehicle.type == TRUCK) {
		length *= 2;
	}

	// Check if player and vehicle are on the same lane
	int vehicle_lane = vehicle.coordinate.y / LANE_HEIGHT;
	int player_lane = player.vertices[0].y / LANE_HEIGHT;
	if (vehicle_lane != player_lane) {
		return false;
	}

	// Decide on the leftmost and rightmost coordinates of the player
	int leftmost = player.vertices[0].x < player.vertices[1].x ? player.vertices[0].x : player.vertices[1].x;
	int rightmost = player.vertices[0].x > player.vertices[1].x ? player.vertices[0].x : player.vertices[1].x;
	leftmost = leftmost < player.vertices[2].x ? leftmost : player.vertices[2].x;
	rightmost = rightmost > player.vertices[2].x ? rightmost : player.vertices[2].x;

	// Check all the possibilities for the collision and return the result
	return (leftmost <= vehicle.coordinate.x + length && rightmost > vehicle.coordinate.x + length) ||
			(rightmost >= vehicle.coordinate.x && leftmost < vehicle.coordinate.x) ||
			(leftmost >= vehicle.coordinate.x && rightmost <= vehicle.coordinate.x + length);
}

bool check_coin_collision(Direction direction) {

	// Check if player and coin are on the same lane
	int coin_lane = coin.coordinate.y / LANE_HEIGHT;
	int player_lane = player.vertices[0].y / LANE_HEIGHT;
	if (coin_lane != player_lane) {
		return false;
	}

	// Check coin collision on x-coordinate and return the result
	return player.vertices[0].x / LANE_HEIGHT == coin.coordinate.x / LANE_HEIGHT;
}

void generate_coin() {

	// Choose a lane randomly
	int lane = rand() % 18;
	skip_pavement(lane);

	// Set the coordinates
	coin.coordinate.x = (rand() % 20) * LANE_HEIGHT + PADDING;
	coin.coordinate.y = PADDING + LANE_HEIGHT * lane;

	// Set coin visible
	coin.visible = true;

	// Check for a collision with the coin
	if (coin.visible && check_coin_collision(player.direction)) {
		score += 5;
		glutTimerFunc(50, timer, 4);
	}
}

void generate_vehicle(Vehicle &vehicle) {

	// Generate a car or truck randomly
	int type = rand() % 2;
	if (type == 0) {
		vehicle.type = CAR;
	} else {
		vehicle.type = TRUCK;
	}

	// Choose a lane randomly
	int lane = rand() % 18;
	if ((lane >= 3 && lane <= 6) || (lane >= 11 && lane <= 13)) {
		vehicle.direction = LEFT;
		vehicle.coordinate.x = WIDTH;
	} else {
		vehicle.direction = RIGHT;
		vehicle.coordinate.x = -36;
	}

	// Set the speed according to the lane
	if (lane == 0 || lane == 3 || lane == 7 || lane == 11 || lane == 14) {
		vehicle.speed = 1;
	} else if (lane == 1 || lane == 4 || lane == 8 || lane == 12 || lane == 15) {
		vehicle.speed = 2;
	} else if (lane == 2 || lane == 5 || lane == 9 || lane == 13 || lane == 16) {
		vehicle.speed = 3;
	} else {
		vehicle.speed = 4;
	}

	// Set final y-coordinate location
	skip_pavement(lane);
	vehicle.coordinate.y = PADDING + LANE_HEIGHT * lane;

	// Generate a random vehicle color
	vehicle.color[0] = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX));
	vehicle.color[1] = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX));
	vehicle.color[2] = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX));
}

void move_player(Direction direction) {

	// Update player location according to the direction
	int x = player.vertices[0].x / LANE_HEIGHT;
	int y = player.vertices[0].y / LANE_HEIGHT;
	player.direction = direction;
	if (direction == LEFT) {
		player.vertices[0].x = LANE_HEIGHT * (x + 1) - OBJECT_HEIGHT - PADDING;
		player.vertices[0].y = LANE_HEIGHT * y + CENTER;
		player.vertices[1].x = player.vertices[0].x + OBJECT_HEIGHT;
		player.vertices[1].y = player.vertices[0].y - 5;
		player.vertices[2].x = player.vertices[0].x + OBJECT_HEIGHT;
		player.vertices[2].y = player.vertices[0].y + 5;
		player.vertices[0].x -= LANE_HEIGHT;
		player.vertices[1].x -= LANE_HEIGHT;
		player.vertices[2].x -= LANE_HEIGHT;
	} else if (direction == DOWN) {
		player.vertices[0].x = LANE_HEIGHT * x + CENTER;
		player.vertices[0].y = LANE_HEIGHT * (y + 1) - OBJECT_HEIGHT - PADDING;
		player.vertices[1].x = player.vertices[0].x - 5;
		player.vertices[1].y = player.vertices[0].y + OBJECT_HEIGHT;
		player.vertices[2].x = player.vertices[0].x + 5;
		player.vertices[2].y = player.vertices[0].y + OBJECT_HEIGHT;
		player.vertices[0].y -= LANE_HEIGHT;
		player.vertices[1].y -= LANE_HEIGHT;
		player.vertices[2].y -= LANE_HEIGHT;
	} else if (direction == RIGHT) {
		player.vertices[0].x = LANE_HEIGHT * x + OBJECT_HEIGHT + PADDING;
		player.vertices[0].y = LANE_HEIGHT * y + CENTER;
		player.vertices[1].x = player.vertices[0].x - OBJECT_HEIGHT;
		player.vertices[1].y = player.vertices[0].y - 5;
		player.vertices[2].x = player.vertices[0].x - OBJECT_HEIGHT;
		player.vertices[2].y = player.vertices[0].y + 5;
		player.vertices[0].x += LANE_HEIGHT;
		player.vertices[1].x += LANE_HEIGHT;
		player.vertices[2].x += LANE_HEIGHT;
	} else if (direction == UP) {
		player.vertices[0].x = LANE_HEIGHT * x + CENTER;
		player.vertices[0].y = LANE_HEIGHT * y + OBJECT_HEIGHT + PADDING;
		player.vertices[1].x = player.vertices[0].x - 5;
		player.vertices[1].y = player.vertices[0].y - OBJECT_HEIGHT;
		player.vertices[2].x = player.vertices[0].x + 5;
		player.vertices[2].y = player.vertices[0].y - OBJECT_HEIGHT;
		player.vertices[0].y += LANE_HEIGHT;
		player.vertices[1].y += LANE_HEIGHT;
		player.vertices[2].y += LANE_HEIGHT;
	}

	// Check for a collision with the coin
	if (coin.visible && check_coin_collision(player.direction)) {
		score += 5;
		glutTimerFunc(50, timer, 4);
	}
}

void single_step() {

	// Update vehicle coordinates
	if (!gameover) {
		for (unsigned int i = 0; i < vehicles.size(); i++) {
			if (vehicles[i].direction == RIGHT) {
				vehicles[i].coordinate.x += vehicles[i].speed;
				if (vehicles[i].coordinate.x >= WIDTH) {
					mutex.lock();
					vehicles.erase(vehicles.begin() + i);
					mutex.unlock();
				}
			} else {
				vehicles[i].coordinate.x -= vehicles[i].speed;
				if (vehicles[i].coordinate.x <= -36) {
					mutex.lock();
					vehicles.erase(vehicles.begin() + i);
					mutex.unlock();
				}
			}
		}
	}
}

void show_text(int x, int y, std::string text, void *font) {

	// Set the position of first letter and print out
	glRasterPos2i(x, y);
	for (unsigned int i = 0; i < text.size(); i++) {
		glutBitmapCharacter(font, text[i]);
	}
}

void initialize() {

	// Create the background
	create_background();

	// Seed the random number generator
	srand(time(0));

	// Reset player location
	player.visible = true;
	player.direction = UP;
	player.vertices[0].x = LANE_HEIGHT * 9 + CENTER;
	player.vertices[0].y = OBJECT_HEIGHT + PADDING;
	player.vertices[1].x = player.vertices[0].x - 5;
	player.vertices[1].y = player.vertices[0].y - OBJECT_HEIGHT;
	player.vertices[2].x = player.vertices[0].x + 5;
	player.vertices[2].y = player.vertices[0].y - OBJECT_HEIGHT;

	// Reset the game variables
	score = 0;
	paused = false;
	gameover = false;

	// Reset the other game objects
	vehicles.clear();
	coin.visible = false;
	coin.counter = 0;
	explosion.visible = false;
	explosion.radius = RADIUS;
	explosion.color[1] = 0.0f;
}

void display() {

	// Clear the frame buffers
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw the pavements
	glColor3f(0.1f, 0.1f, 0.1f);
	for (int i = 0; i < PAVEMENT_COUNT; i++) {
		Coordinate pavement = pavements[i];
		glRecti(pavement.x, pavement.y - 1, pavement.x + WIDTH, pavement.y + LANE_HEIGHT + 1);
	}

	// Draw the lanes
	glColor3f(1.0f, 1.0f, 0.0f);
	for (int i = 0; i < LANE_COUNT; i++) {
		Coordinate lane = lanes[i];
		glBegin(GL_LINES);
		for (int i = 0; i <= WIDTH; i += 20) {
			glVertex2i(i, lane.y);
		}
		glEnd();
	}

	// Draw the coin if its life-cycle is not over
	if (coin.visible) {
		glColor3f(1.0f, 1.0f, 0.0f);
		glBegin(GL_TRIANGLE_FAN);
		for (GLfloat theta = 0.0f; theta < 2 * PI; theta += ANGLE_INCREMENT) {
			GLfloat x = RADIUS * cos(theta) + coin.coordinate.x + OBJECT_HEIGHT / 2;
			GLfloat y = RADIUS * sin(theta) + coin.coordinate.y + OBJECT_HEIGHT / 2;
			glVertex2f(x, y);
		}
		glEnd();
	}

	// Draw the player
	if (player.visible) {
		glColor3f(1.0f, 0.5f, 0.0f);
		glBegin(GL_TRIANGLES);
			glVertex2i(player.vertices[0].x, player.vertices[0].y);
			glVertex2i(player.vertices[1].x, player.vertices[1].y);
			glVertex2i(player.vertices[2].x, player.vertices[2].y);
		glEnd();
	}

	// Draw the vehicles
	for (unsigned int i = 0; i < vehicles.size(); i++) {
		Vehicle vehicle = vehicles[i];
		Coordinate coordinate = vehicle.coordinate;
		glColor3f(vehicle.color[0], vehicle.color[1], vehicle.color[2]);
		if (vehicle.type == CAR) {
			glRecti(coordinate.x, coordinate.y, coordinate.x + OBJECT_HEIGHT, coordinate.y + OBJECT_HEIGHT);
		} else {
			glRecti(coordinate.x, coordinate.y, coordinate.x + OBJECT_HEIGHT * 2, coordinate.y + OBJECT_HEIGHT);
		}

		// Check if any of the vehicles collided with the player
		if (check_vehicle_collision(vehicle) && !gameover) {
			player.visible = false;
			explosion.visible = true;
			explosion.coordinate.x = (player.vertices[0].x + player.vertices[1].x + player.vertices[2].x) / 3;
			explosion.coordinate.y = (player.vertices[0].y + player.vertices[1].y + player.vertices[2].y) / 3;
			glutTimerFunc(50, timer, 5);
			gameover = true;
		}
	}

	// Draw the explosion
	if (explosion.visible) {
		glColor3f(explosion.color[0], explosion.color[1], explosion.color[2]);
		glBegin(GL_TRIANGLE_FAN);
		for (GLfloat theta = 0.0f; theta < 2 * PI; theta += ANGLE_INCREMENT) {
			GLfloat x = explosion.radius * cos(theta) + explosion.coordinate.x;
			GLfloat y = explosion.radius * sin(theta) + explosion.coordinate.y;
			glVertex2f(x, y);
		}
		glEnd();
	}

	// Output the current score to the screen
	glColor3f(1.0f, 1.0f, 0.0f);
	std::string output = "Score: " + std::to_string(score);
	show_text(5, HEIGHT - 20, output, GLUT_BITMAP_HELVETICA_18);

	// Output game result if game is completed
	if (gameover) {

		// If score limit is reached, output the success message
		std::string message;
		int start_point;
		if (score >= SCORE_LIMIT) {
			message = "You win!";
			start_point = 210;
		} else {
			message = "Game Over!";
			start_point = 190;
		}

		show_text(start_point, 305, message, GLUT_BITMAP_TIMES_ROMAN_24);
		show_text(140, 280, "Press 'R' to reset the game", GLUT_BITMAP_HELVETICA_18);
	} 
	
	// Double buffering
	glutSwapBuffers();

	// Force to render as soon as possible
	glFlush();
}

void timer(int id) {

	// Choose which timer is operation is being called
	if (id == 0) {

		// Generate a coin if game is not paused or completed
		if (!paused && !gameover) {
			generate_coin();
		}

		// Start the timer to erase the coin
		glutTimerFunc(5000, timer, 3);

	} else if (id == 1) {

		// Stop generating vehicles if game is paused or completed
		if (!paused && !gameover) {

			// Generate a vehicle
			Vehicle vehicle;
			generate_vehicle(vehicle);

			// Protect the vehicle list from concurrent updates
			mutex.lock();
			vehicles.push_back(vehicle);
			mutex.unlock();
		}

		// Start the timer again
		glutTimerFunc(1000, timer, 1);

	} else if (id == 2) {

		// Stop updating vehicle coordinates if game is paused or completed
		if (!paused && !gameover) {

			// Every vehicle take a single step
			single_step();
		}

		// Start the timer again
		glutTimerFunc(20, timer, 2);

	} else if (id == 3) {

		// Hide the coin if game is not paused or completed
		if (!paused && !gameover) {
			coin.visible = false;
		}

		// Start the timer to generate another coin
		glutTimerFunc(10000, timer, 0);

	} else if (id == 4) {

		// Start the timer again to play the animation until the counter reaches 5
		if (coin.counter == 5) {
			coin.counter = 0;
			coin.visible = false;
		} else {
			coin.counter++;
			coin.coordinate.y += 2;
			glutTimerFunc(50, timer, 4);
		}
	} else {

		// Start the timer again to play the animation until the explosion radius is big enough
		if (explosion.radius < RADIUS + 8) {
			explosion.radius++;
			explosion.color[1] += 1.0f / 8;
			glutTimerFunc(50, timer, 5);
		} else {
			explosion.visible = false;
		}
	}

	// Render the scene again
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {

	// Quit or restart game
	if (key == 'q' || key == 'Q' || key == '\x1b') {
		exit(EXIT_SUCCESS);
	} else if (gameover && (key == 'r' || key == 'R')) {
		initialize();
	}
}

void movement(int key, int x, int y) {

	// Stop player movement if game is paused or completed
	if (!paused && !gameover) {

		// Movement with arrow keys
		if (key == GLUT_KEY_LEFT && player.vertices[0].x > LANE_HEIGHT) {
			move_player(LEFT);
		} else if (key == GLUT_KEY_DOWN && player.vertices[0].y > LANE_HEIGHT) {
			move_player(DOWN);
			score++;
		} else if (key == GLUT_KEY_RIGHT && player.vertices[0].x < WIDTH - LANE_HEIGHT) {
			move_player(RIGHT);
		} else if (key == GLUT_KEY_UP && player.vertices[0].y < HEIGHT - LANE_HEIGHT) {
			move_player(UP);
			score++;
		}
	}

	// Complete the game if score limit is reached
	if (score >= SCORE_LIMIT) {
		gameover = true;
	}

	// Render the scene again
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {

	// Controls the game state
	if (state == GLUT_DOWN) {
		switch (button) {
		case GLUT_LEFT_BUTTON:
			paused = !paused;
			break;
		case GLUT_RIGHT_BUTTON:
			paused = true;
			single_step();
			int x = (player.vertices[0].x + player.vertices[1].x + player.vertices[2].x) / 3;
			int y = (player.vertices[0].y + player.vertices[1].y + player.vertices[2].y) / 3;
			std::cout << "Points: " << score << ", Player Position: " << x << "-" << y << " Vehicle Count: " << vehicles.size() << std::endl;
			break;
		}
	}
}

int main(int argc, char **argv) {

	// Initialize window using GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(0, 0);

	// The lines below are used to check the version in case freeglut is used
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// Create the window
	glutCreateWindow("Highway");

	// Initialize GLEW
	glewInit();

	// Set up orthographic viewing region
	gluOrtho2D(0.0f, WIDTH, 0.0f, HEIGHT);

	// Enable anti-aliasing
	glEnable(GL_LINE_SMOOTH);

	// White background
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	// Initialize the application
	initialize();

	// Create a display function callback
	glutDisplayFunc(display);

	// Create a time function callback
	glutTimerFunc(10000, timer, 0);
	glutTimerFunc(1000, timer, 1);
	glutTimerFunc(20, timer, 2);

	// Create a keyboard event listener
	glutKeyboardFunc(keyboard);

	// Create a special keyboard event listener for arrow keys
	glutSpecialFunc(movement);

	// Create a mouse event listener
	glutMouseFunc(mouse);

	// Keep the window open
	glutMainLoop();

	return 0;
}