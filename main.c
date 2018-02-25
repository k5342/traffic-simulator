#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#if __has_include(<GLUT/glut.h>)
	#include <GLUT/glut.h>
#endif

#if __has_include(<GL/glut.h>)
	#include <GL/glut.h>
#endif

#define KEY_ESC 27

#define DOT 0.025
float light[] = {1.0, 1.0, 1.0, 1.0};
int frame = 0;

float roadLength;
float roadWidth;
float carLength;
float carWidth;

typedef enum {
	Red,
	Yellow,
	LightBlue,
	Gray,
	Black,
	White
} Color;

typedef enum {
	UNKNOWN = -1,
	NEXT = 0,
	RIGHT = 1,
	PREV = 2,
	LEFT = 3,
} Direction;

typedef struct road Road;
typedef struct car Car;
typedef struct traffic_light TrafficLight;

struct car {
	float speed_max;
	float speed_current;
	bool positive;
	float position;
	Road *road;
	Car *next;
	Car *prev;
	bool isStop;
	Direction next_road;
	Direction prev_road;
	int frame;
	float angle;
};

typedef enum {
	Normal,
	Intersection
} Roadtype;

struct road {
	Road *next;
	Road *prev;
	Road *right;
	Road *left;
	Roadtype type;
	Car *in_range_car_p;
	Car *in_range_car_n;
	Car *in_range_car_u;
	Car *in_range_car_d;
	TrafficLight *traffic_light;
	unsigned int drawcode;
	bool is_appended;
	Direction direction;
};

typedef enum {
	PrimaryBlue,
	PrimaryYellow,
	PrimaryRed,
	SecondaryBlue,
	SecondaryYellow,
	SecondaryRed
} TrafficLightState;

struct traffic_light {
	TrafficLight *next;
	TrafficLight *prev;
	int blue_time_primary;
	int blue_time_second;

	TrafficLightState state;
	int time_remain;
	bool isRed;
	bool isYellow;
	bool isBlue;
	
	bool isReds;
	bool isYellows;
	bool isBlues;
};

Road *root_road, *road_last;
TrafficLight *traffic_light_root;
Car *cars_head = NULL;

void initLightning(){
	float diffuse[] = {0.7, 0.7, 0.7, 1.0};
	float specular[] = {0.6, 0.6, 0.6, 1.0};
	float ambient[] = {1.0, 1.0, 1.0, 1.0};
	
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
}

void solidCuboid(float x, float y, float z){

	glPushMatrix();
		x /= 2;
		y /= 2;
		z /= 2;
		glTranslatef(x, y, z);
		glBegin(GL_QUADS);
			glNormal3f(0.0f, 1.0f, 0.0f);
			glVertex3f(  x,   y,   z);
			glVertex3f(  x,   y,  -z);
			glVertex3f( -x,   y,  -z);
			glVertex3f( -x,   y,   z);
		
			glNormal3f(0.0f, 0.0f, 1.0f);
			glVertex3f(  x,   y,   z);
			glVertex3f( -x,   y,   z);
			glVertex3f( -x,  -y,   z);
			glVertex3f(  x,  -y,   z);
		
			glNormal3f(1.0f, 0.0f, 0.0f);
			glVertex3f(  x,   y,   z);
			glVertex3f(  x,  -y,   z);
			glVertex3f(  x,  -y,  -z);
			glVertex3f(  x,   y,  -z);
		
			glNormal3f(0.0f, -1.0f, 0.0f);
			glVertex3f(  x,  -y,  -z);
			glVertex3f(  x,  -y,   z);
			glVertex3f( -x,  -y,   z);
			glVertex3f( -x,  -y,  -z);
		
			glNormal3f(0.0f, 0.0f, -1.0f);
			glVertex3f(  x,   y,  -z);
			glVertex3f(  x,  -y,  -z);
			glVertex3f( -x,  -y,  -z);
			glVertex3f( -x,   y,  -z);
		
			glNormal3f(-1.0f, 0.0f, 0.0f);
			glVertex3f( -x,   y,  -z);
			glVertex3f( -x,  -y,  -z);
			glVertex3f( -x,  -y,   z);
			glVertex3f( -x,   y,   z);
		glEnd();
	glPopMatrix();
}

float emission[] = {0.0, 0.0, 0.0, 1.0};
void setWhite(){
	float diffuse[] = {0.95, 0.95, 0.95, 1.0};
	float specular[] = {0.8, 0.8, 0.8, 1.0};
	float ambient[] = {0.1, 0.1, 0.1, 1.0};
	float shininess = 128.0;
	
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, emission);
}

void setGray(){
	float diffuse[] = {0.2, 0.2, 0.2, 1.0};
	float specular[] = {0.8, 0.8, 0.8, 1.0};
	float ambient[] = {0.1, 0.1, 0.1, 1.0};
	float shininess = 128.0;
	
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, emission);
}

void setBlack(){
	float diffuse[] = {0.05, 0.05, 0.05, 1.0};
	float specular[] = {0.8, 0.8, 0.8, 1.0};
	float ambient[] = {0.1, 0.1, 0.1, 1.0};
	float shininess = 128.0;
	
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, emission);
}

void setRed(){
	float diffuse[] = {0.9, 0.1, 0.0, 1.0};
	float specular[] = {0.8, 0.8, 0.8, 1.0};
	float ambient[] = {0.1, 0.1, 0.1, 1.0};
	float shininess = 128.0;
	
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, emission);
}

void setYellow(){
	float diffuse[] = {0.9, 0.9, 0.0, 1.0};
	float specular[] = {0.8, 0.8, 0.8, 1.0};
	float ambient[] = {0.1, 0.1, 0.1, 1.0};
	float shininess = 128.0;
	
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, emission);
}

void setLightBlue(){
	float diffuse[] = {0.0, 0.5, 1.0, 1.0};
	float specular[] = {0.8, 0.8, 0.8, 1.0};
	float ambient[] = {0.1, 0.1, 0.1, 1.0};
	float shininess = 128.0;
	
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, emission);
}

// hontai sita
float bodyx = 0.33;
float bodyy = DOT * 5;
float bodyz = 0.165;


Road *nextRoad(Road *current, Direction from, Direction to){
	switch (((from + 2) % 4 + to) % 4){
		case NEXT:
			return current -> next;
			break;
		case RIGHT:
			return current -> right;
			break;
		case PREV:
			return current -> prev;
			break;
		case LEFT:
			return current -> left;
			break;
	}
}

void putCar(Color color, Car *car){
	
	// hontai ue
	float ubodyx = bodyx * 0.7;
	float ubodyy = bodyy * 0.6;
	float ubodyz = bodyz * 0.8;
	
	// tire
	float tire2r = DOT * 3;
	float wheel2r = DOT;
	
	// 本体
	glPushMatrix();
		glPushMatrix();
			glTranslatef(carLength, 0.0, 0.0);
			glRotatef(car -> angle, 0.0, 1.0, 0.0);
			glTranslatef(-carLength, 0.0, 0.0);
			glTranslatef(0.0, 0.0, DOT);
			
			
			glPushMatrix();
					
				setLightBlue();
				
				glPushMatrix();
					
					// 下部
					glPushMatrix();
						glTranslatef(0.0, DOT, 0.0);
						solidCuboid(bodyx, bodyy - DOT, bodyz);
					glPopMatrix();
						
					// 上部
					glPushMatrix();
						glTranslatef((bodyx - ubodyx)/2, bodyy, (bodyz - ubodyz)/2);
						solidCuboid(ubodyx, ubodyy, ubodyz);
					glPopMatrix();
						
					// タイヤ
					glPushMatrix();
						glTranslatef((bodyx - ubodyx)/2 - tire2r*0.2, 0.0, -DOT/2);
						setBlack();
						solidCuboid(tire2r, tire2r, bodyz + DOT/2 * 2);
					
						// ホイール
						setWhite();
						glTranslatef((tire2r - wheel2r)/2, (tire2r - wheel2r)/2, -DOT/2);
						solidCuboid(wheel2r, wheel2r, bodyz + DOT/2 * 2 * 2);
					glPopMatrix();
						
					// タイヤ
					glPushMatrix();
						glTranslatef((bodyx - ubodyx)/2 + ubodyx - tire2r*0.8, 0.0, -DOT/2);
						setBlack();
						solidCuboid(tire2r, tire2r, bodyz + DOT/2 * 2);
					
						// ホイール
						setWhite();
						glTranslatef((tire2r - wheel2r)/2, (tire2r - wheel2r)/2, -DOT/2);
						solidCuboid(wheel2r, wheel2r, bodyz + DOT/2 * 2 * 2);
					glPopMatrix();
						
					// 前面ライト
					glPushMatrix();
						glTranslatef(bodyx, bodyy - DOT*2, 0.0);
						
						setYellow();
						if ((car -> frame / 28) % 2 == 0 && car -> next_road == LEFT){
							float ambient[] = {0.9, 0.9, 0.01, 1.0};
							glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
							
							float emission[] = {1.0, 1.0, 0.0, 1.0};
							glMaterialfv(GL_FRONT, GL_EMISSION, emission);
						}
						glTranslatef(0, 0.0, 0);
						solidCuboid(DOT/3, DOT, DOT/2);
						
						setWhite();
						glTranslatef(0, 0.0, DOT/2);
						solidCuboid(DOT/3, DOT, DOT);
						
						setBlack();
						glTranslatef(0, 0.0, DOT);
						solidCuboid(DOT/3, DOT, bodyz - (DOT*2 + DOT));
						
						setWhite();
						glTranslatef(0, 0.0, bodyz - (DOT*2 + DOT));
						solidCuboid(DOT/3, DOT, DOT);
						
						setYellow();
						if ((car -> frame / 28) % 2 == 0 && car -> next_road == RIGHT){
							float ambient[] = {0.9, 0.9, 0.01, 1.0};
							glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
							
							float emission[] = {1.0, 1.0, 0.0, 1.0};
							glMaterialfv(GL_FRONT, GL_EMISSION, emission);
						}
						glTranslatef(0, 0.0, DOT);
						solidCuboid(DOT/3, DOT, DOT/2);
					glPopMatrix();
				
					// 後面ライト
					glPushMatrix();
						glTranslatef(-DOT/3, bodyy - DOT*3, 0.0);
						
						setYellow();
						if ((car -> frame / 28) % 2 == 0 && car -> next_road == LEFT){
							float ambient[] = {0.9, 0.9, 0.01, 1.0};
							glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
							
							float emission[] = {1.0, 1.0, 0.0, 1.0};
							glMaterialfv(GL_FRONT, GL_EMISSION, emission);
						}
						glTranslatef(0, 0.0, 0);
						solidCuboid(DOT/3, DOT, DOT/2);
						
						setRed();
						if (car -> isStop){
							float ambient[] = {1.0, 0.01, 0.01, 1.0};
							glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
							
							float emission[] = {1.0, 0.0, 0.0, 1.0};
							glMaterialfv(GL_FRONT, GL_EMISSION, emission);
						}
						glTranslatef(0, 0.0, DOT/2);
						solidCuboid(DOT/3, DOT, DOT);
						
						glTranslatef(0, 0.0, bodyz - DOT*2);
						solidCuboid(DOT/3, DOT, DOT);
						
						setYellow();
						if ((car -> frame / 28) % 2 == 0 && car -> next_road == RIGHT){
							float ambient[] = {0.9, 0.9, 0.01, 1.0};
							glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
							
							float emission[] = {1.0, 1.0, 0.0, 1.0};
							glMaterialfv(GL_FRONT, GL_EMISSION, emission);
						}
						glTranslatef(0, 0.0, DOT);
						solidCuboid(DOT/3, DOT, DOT/2);
					glPopMatrix();
					
				glPopMatrix();
			glPopMatrix();
		glPopMatrix();
	glPopMatrix();
}

Road *createRoad(){
	Road *ptr = malloc(sizeof(Road));
	
	if (ptr == NULL){
		return NULL;
	}
	
	memset(ptr, 0, sizeof(Road));
	
	ptr -> type = Normal;
	ptr -> is_appended = false;
	
	return ptr;
}

Car *createCar(Road *r, float speed_max, bool positive){
	Car *ptr = malloc(sizeof(Car));
	
	if (ptr == NULL){
		return NULL;
	}
	
	memset(ptr, 0, sizeof(Car));
	
	ptr -> speed_current = 0;
	ptr -> speed_max = speed_max;
	ptr -> positive = positive;
	ptr -> position = 0;
	ptr -> road = r;
	ptr -> next_road = UNKNOWN;
	
	ptr -> next = NULL;
	ptr -> prev = cars_head;
	
	if (cars_head != NULL){
		cars_head -> next = ptr;
	}
	
	if (r != NULL){
		if (positive){
			r -> in_range_car_p = ptr;
		} else {
			r -> in_range_car_n = ptr;
		}
	}
	
	cars_head = ptr;
	
	return ptr;
}

void removeCar(Car *c){
	printf("removeCar: %p\n", c);
	if (c -> positive){
		c -> road -> in_range_car_p = NULL;
	} else {
		c -> road -> in_range_car_n = NULL;
	}
	
	if (c -> next != NULL){
		printf("c -> next -> prev: %p\n", c -> next -> prev);
		c -> next -> prev = c -> prev;
	}
	
	if (c -> prev != NULL){
		c -> prev -> next = c -> next;
	}
	
	if (cars_head == c){
		if (cars_head == NULL){
			cars_head = NULL;
		} else {
			cars_head = cars_head -> prev;
		}
	}
	
	free(c);
	printf("---removeCar\n");
}

void processCars(){
	Car *c = cars_head;
	
	printf("1\n");
	if (c == NULL){
		return;
	}
	
	printf("2\n");
	do {
		Road *next_road = NULL;
		Road *next_next_road = NULL;
		Road *opposite_road = NULL;
		Car *in_next_road = NULL;
		Car *in_next_next_road = NULL;
		Car *in_opposite_road = NULL;
		
		bool next_positive;
		bool opposite_positive;
		
		c -> frame += 1;
		
		if (c -> road != NULL){
			
			if (c -> road -> type == Intersection){
				next_road = nextRoad(c -> road, c -> prev_road, c -> next_road);
				next_positive = (next_road -> prev == c -> road);
				
				opposite_road = nextRoad(c -> road, c -> prev_road, NEXT);
				opposite_positive = (opposite_road -> prev == c -> road);
			} else {
				if (c -> positive){
					next_road = c -> road -> next;
				} else {
					next_road = c -> road -> prev;
				}
				next_positive = c -> positive;
			}
			
			if (next_road != NULL){
				if (next_positive){
					in_next_road = next_road -> in_range_car_p;
					next_next_road = next_road -> next;
					
					if (next_next_road != NULL){
						in_next_next_road = next_next_road -> in_range_car_p;
					}
				} else {
					in_next_road = next_road -> in_range_car_n;
					next_next_road = next_road -> prev;
					
					if (next_next_road != NULL){
						in_next_next_road = next_next_road -> in_range_car_n;
					}
				}
			}
			
			if (opposite_road != NULL){
				if (opposite_positive){
					in_opposite_road = opposite_road -> in_range_car_p;
				} else {
					in_opposite_road = opposite_road -> in_range_car_n;
				}
			}
		}
		
		printf("%d, %p, road:%p, isStop:%d, speed:%f, in_next_road:%p, next_road:%p, next:%d\n", c->positive, c, c -> road, c -> isStop, c->speed_current, in_next_road, next_road, c -> next_road);
		
		// check whether can go or not
		if (next_road != NULL){
			bool tmp_isStop = false;
			
			// traffic light is red
			if (next_road -> traffic_light != NULL){
				TrafficLightState s = next_road -> traffic_light -> state;
				
				if (c -> road -> is_appended){
					if (s != SecondaryBlue && s != SecondaryYellow){
						tmp_isStop = tmp_isStop || true;
					}
				} else {
					if (s != PrimaryBlue && s != PrimaryYellow){
						tmp_isStop = tmp_isStop || true;
					}
				}
			}
			
			// next lane is not available
			if (in_next_road != NULL){
				if ((in_next_road -> position/100)*roadLength - carLength
						+ ((100 - c -> position)/100)*roadLength < carLength){
					tmp_isStop = tmp_isStop || true;
				}
			}
			
			if (next_road -> type == Intersection){
				if (in_next_next_road != NULL){
					if (in_next_next_road -> isStop){
						tmp_isStop = tmp_isStop || true;
					}
				}
			}
			
			if (c -> next_road == RIGHT){
				if (in_opposite_road != NULL){
					if (in_opposite_road -> next_road != RIGHT){
						tmp_isStop = tmp_isStop || true;
					}
				}
			}
			
			c -> isStop = tmp_isStop;
		} else {
			c -> isStop = false;
		}
		
		// determine next route
		if (next_road != NULL){
			if (next_road -> type == Intersection){
				if (c -> next_road == UNKNOWN){
					float x = rand() % 10;
					
					if (x < 6){
						c -> next_road = NEXT;
					} else if (x < 8){
						c -> next_road = RIGHT;
					} else {
						c -> next_road = LEFT;
					}
				}
			}
		}
		
		// speed management
		if (c -> isStop){
			float ds = c -> speed_current / (100 - c -> position);
			if (c -> speed_current > 0){
				c -> speed_current -= ds;
			}
			if (c -> speed_current < 0){
				c -> speed_current = 0;
			}
		} else {
			if (in_next_road == NULL){
				if (c -> speed_current < c -> speed_max){
					c -> speed_current += 0.01;
				}
			} else {
				if (c -> speed_current < c -> speed_max){
					c -> speed_current += 0.01;
				}
			}
			
			if (c -> speed_current > c -> speed_max){
				c -> speed_current = c -> speed_max;
			}
		}
		
		// prepare position at next frame
		if (c -> position + c -> speed_current >= 100){
			if (next_road == NULL){
				removeCar(c);
			} else {
				if (!c -> isStop && in_next_road == NULL){
					if (next_road -> type == Intersection){
						c -> position = 0;
					} else {
						c -> position = (c -> position + c -> speed_current) - 100;
					}
					
					if (c -> road -> type == Intersection){
						c -> next_road = UNKNOWN;
					}
					
					// swap current road status to next state
					if (c -> positive){
						c -> road -> in_range_car_p = NULL;
					} else {
						c -> road -> in_range_car_n = NULL;
					}
					
					c -> prev_road = c -> road -> direction;
					c -> road = next_road;
					c -> positive = next_positive;
					c -> angle = 0.0;
					
					if (c -> positive){
						next_road -> in_range_car_p = c;
					} else {
						next_road -> in_range_car_n = c;
					}
				}
			}
		} else {
			if (c -> road -> type == Intersection){
				switch (c -> next_road){
					case LEFT:
						c -> angle = pow(c -> position / 100, 3) * 90;
						c -> position += c -> speed_current;
						break;
					case RIGHT:
						c -> angle = -pow(c -> position / 100, 3) * 90;
						c -> position += c -> speed_current;
						break;
					default:
						c -> position += c -> speed_current;
						break;
				}
			} else {
				c -> position += c -> speed_current;
			}
		}
	} while ((c = c -> prev) != NULL);
}

Road *addNormalRoad(Road *root, Road **addition, int x, Direction d){
	if (x < 1){
		printf("x must be greater than 1.\n");
		exit(1);
	}
	
	Road *prev;
	
	*addition = createRoad();
	(*addition) -> direction = d;
	(*addition) -> prev = root;
	prev = *addition;
	
	d = NEXT;
	x -= 1;
	
	int i = 0;
	for(i = 0; i < x; i++){
		Road *r = createRoad();
		
		r -> type = Normal;
		r -> prev = prev;
		r -> next = NULL;
		r -> direction = d;
		
		prev -> next = r;
		
		prev = r;
		d = NEXT;
	}
	
	return prev;
}

Road *addIntersection(Road *root, Road **addition, Direction d){
	Road *r = createRoad();
	r -> type = Intersection;
	r -> prev = root;
	r -> next = NULL;
	r -> direction = d;
	
	if (r -> prev != NULL){
		r -> prev -> direction = PREV;
	}
	
	*addition = r;
	
	return r;
}

TrafficLight *createTrafficLight(TrafficLight **root, int blue_time_primary, int blue_time_second){
	TrafficLight *ptr = malloc(sizeof(TrafficLight));
	
	while (*root != NULL && *(root = &((*root) -> next)) != NULL){
	}
	
	if (ptr == NULL){
		return NULL;
	}
	
	memset(ptr, 0, sizeof(TrafficLight));
	
	*root = ptr;
	
	ptr -> next = NULL;
	ptr -> prev = *root;
	ptr -> blue_time_primary = blue_time_primary;
	ptr -> blue_time_second = blue_time_second;
	
	return ptr;
}

void processTrafficLights(TrafficLight *root){
	if (root == NULL){
		return;
	}
	
	do {
		printf("%p : remain -> %d, state -> %d\n", root, root -> time_remain, root -> state);
		if (root -> time_remain - 1 <= 0){
			// this is called on state changing
			switch (root -> state){
				case PrimaryBlue:
					root -> isBlue = false;
					root -> isYellow = true;
					root -> state = PrimaryYellow;
					root -> time_remain = 100;
					break;
				case PrimaryYellow:
					root -> isYellow = false;
					root -> isRed = true;
					root -> state = PrimaryRed;
					root -> time_remain = 100;
					break;
				case PrimaryRed:
					root -> isReds = false;
					root -> isBlues = true;
					root -> state = SecondaryBlue;
					root -> time_remain = root -> blue_time_second;
					break;
				case SecondaryBlue:
					root -> isBlues = false;
					root -> isYellows = true;
					root -> state = SecondaryYellow;
					root -> time_remain = 100;
					break;
				case SecondaryYellow:
					root -> isYellows = false;
					root -> isReds = true;
					root -> state = SecondaryRed;
					root -> time_remain = 100;
					break;
				case SecondaryRed:
				default:
					root -> isRed = false;
					root -> isBlue = true;
					root -> state = PrimaryBlue;
					root -> time_remain = root -> blue_time_primary;
					break;
			}
		} else {
			root -> time_remain -= 1;
		}
	} while ((root = root -> next) != NULL);
}

void putTrafficLight(bool isRed, bool isYellow, bool isBlue){
	glPushMatrix();
		setWhite();
		
		solidCuboid(DOT, bodyy*2 + DOT*4 + DOT*3 + DOT/2, DOT);
	
		setWhite();
		glTranslatef(0.0, bodyy*2 + DOT*4, DOT);
		solidCuboid(DOT, DOT*3, DOT*7);
		
		float ambient[] = {0.01, 0.01, 0.01, 1.0};
		
		
		setLightBlue();
		glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
		
		if (isBlue){
			float ambient[] = {0.01, 0.5, 1.0, 1.0};
			glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
			
			float emission[] = {0.0, 0.5, 1.0, 1.0};
			glMaterialfv(GL_FRONT, GL_EMISSION, emission);
		}
		
		glTranslatef(-DOT/2, DOT, DOT);
		solidCuboid(DOT/2, DOT, DOT);
		
		
		setYellow();
		glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
		
		if (isYellow){
			float ambient[] = {0.9, 0.9, 0.01, 1.0};
			glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
			
			float emission[] = {1.0, 1.0, 0.0, 1.0};
			glMaterialfv(GL_FRONT, GL_EMISSION, emission);
		}
		
		glTranslatef(0.0, 0.0, DOT*2);
		solidCuboid(DOT/2, DOT, DOT);
		
		
		setRed();
		glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
		
		if (isRed){
			float ambient[] = {1.0, 0.01, 0.01, 1.0};
			glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
			
			float emission[] = {1.0, 0.0, 0.0, 1.0};
			glMaterialfv(GL_FRONT, GL_EMISSION, emission);
		}
			
		glTranslatef(0.0, 0.0, DOT*2);
		solidCuboid(DOT/2, DOT, DOT);
	glPopMatrix();
}

void __drawRoad(Road *r){
	glPushMatrix();
		// positive lane
		glPushMatrix();
			glTranslatef(0.0, -DOT, 0.0);
			setGray();
			solidCuboid(roadLength, DOT, roadWidth);
		glPopMatrix();
		
		// median strip 
		glPushMatrix();
			glTranslatef(0.0, -DOT, roadWidth);
			switch (r -> type){
				case Intersection:
					setGray();
					solidCuboid(roadLength, DOT, DOT);
					break;
				default:
					setWhite();
					solidCuboid(roadLength, DOT + DOT/6, DOT);
					break;
			}
		glPopMatrix();
		
		// negative lane
		glPushMatrix();
			setGray();
			glTranslatef(0.0, -DOT, roadWidth + DOT);
			solidCuboid(roadLength, DOT, roadWidth);
		glPopMatrix();
		
		if (r -> type == Intersection){
			// traffic lights
			glPushMatrix();
				
				if (r -> traffic_light != NULL){
					TrafficLight *t = r -> traffic_light;
					
					glPushMatrix();
						glTranslatef(roadLength, 0.0, -DOT);
						putTrafficLight(t -> isRed, t -> isYellow, t -> isBlue);
					glPopMatrix();
					
					glPushMatrix();
						glTranslatef(0.0, 0.0, roadWidth*2 + DOT + DOT);
						glRotatef(180, 0.0, 1.0, 0.0);
						putTrafficLight(t -> isRed, t -> isYellow, t -> isBlue);
					glPopMatrix();
				}
			glPopMatrix();
		}
		
		float offsetX = 0.0;
		float offsetZ = 0.0;
		float rsize = (roadWidth - carWidth)/2;
		
		if (r -> in_range_car_p != NULL){
			Car *c = r -> in_range_car_p;
			
			if (c -> road -> type == Intersection){
				switch (c -> next_road){
					case LEFT:
						offsetX = -carLength + DOT * (r -> in_range_car_p -> position / 100);
						offsetZ = -pow(r -> in_range_car_p -> position / 100, 3) * rsize;
						break;
					case RIGHT:
						offsetX = -carLength + roadLength * (r -> in_range_car_p -> position / 100);
						offsetZ = pow(r -> in_range_car_p -> position / 100, 3) * (roadWidth*2);
						break;
					default:
						offsetX = -carLength + roadLength * (r -> in_range_car_p -> position / 100);
						break;
				}
			} else {
				offsetX = -carLength + roadLength * (r -> in_range_car_p -> position / 100);
			}
			
			glPushMatrix();
				glTranslatef(offsetX, 0, rsize + offsetZ);
				putCar(LightBlue, r -> in_range_car_p);
			glPopMatrix();
		}
		
		if (r -> in_range_car_n != NULL){
			Car *c = r -> in_range_car_n;
			
			if (c -> road -> type == Intersection){
				switch (c -> next_road){
					case LEFT:
						offsetX = -(rsize + DOT) * r -> in_range_car_n -> position / 100;
						offsetZ = DOT * pow((100 - r -> in_range_car_n -> position) / 100, 3);
						break;
					case RIGHT:
						offsetX = (roadWidth*2 + DOT - rsize) * r -> in_range_car_n -> position / 100;
						offsetZ = -(rsize*2 + DOT*2 + roadWidth) * pow(r -> in_range_car_n -> position / 100, 3);
						break;
					default:
						offsetX = roadLength * r -> in_range_car_n -> position / 100;
						break;
				}
			} else {
				offsetX = roadLength * r -> in_range_car_n -> position / 100;
			}
			
			glPushMatrix();
				
				glTranslatef(carLength/2, 0.0, carWidth/2 + DOT/2);
				glRotatef(180, 0.0, 1.0, 0.0);
				glTranslatef(-carLength/2, 0.0, -carWidth/2 - DOT/2);
				glPushMatrix();
					glTranslatef(-carLength + offsetX, 0, -(roadWidth - carWidth)/2 - roadWidth - offsetZ);
					putCar(LightBlue, r -> in_range_car_n);
				glPopMatrix();
			glPopMatrix();
		}
	glPopMatrix();
}

void drawRoads(Road *root){
	
	if (root != root_road && root -> drawcode == root_road -> drawcode){
		return;
	}
	
	Road *r = root;
	
	float x;
	float roadAllWidth = roadWidth *2 + DOT;
	
	do {
		glPushMatrix();
			glTranslatef(x, 0.0, 0.0);
			__drawRoad(r);
		glPopMatrix();
		r -> drawcode = root -> drawcode;
		
		if (r -> right != NULL){
			glPushMatrix();
				glTranslatef(x + roadLength, 0.0, roadAllWidth);
				glRotatef(-90, 0.0, 1.0, 0.0);
				drawRoads(r -> right);
			glPopMatrix();
		}
		if (r -> left != NULL){
			glPushMatrix();
				glTranslatef(x, 0.0, 0.0);
				glRotatef(90, 0.0, 1.0, 0.0);
				drawRoads(r -> left);
			glPopMatrix();
		}
		
		r -> drawcode += 1;
		x += roadLength;
	} while ((r = r -> next) != NULL);
}

void display(void){
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	glPushMatrix();
		
		glEnable(GL_NORMALIZE);
		
		glLightfv(GL_LIGHT0, GL_POSITION, light);
		glEnable(GL_LIGHT0);
		
		glEnable(GL_LIGHTING);
		
		glPushMatrix();
			glPushMatrix();
			glRotatef(90, 0.0, 1.0, 0.0);
	//			glRotatef(frame/10 % 360, 0.0, 1.0, 0.0);
	//			glTranslatef(-6.5, 0, 0);
			glRotatef(5, 0.0, 0.0, 1.0);
			glTranslatef(-3.0, 0.0, 0.0);
				
				printf("drawRoads\n");
				drawRoads(root_road);
				printf("---drawRoads\n");
				
			glPopMatrix();
		glPopMatrix();
		
	glPopMatrix();
	
	glutSwapBuffers();
}

void onKeydown(unsigned char key, int x, int y){
	if (key == KEY_ESC){
		exit(0);
	}
}

void idle(){
	printf("idle\n");
	frame++;
		printf("processCars\n");
		processCars();
		printf("---processCars\n");
		processTrafficLights(traffic_light_root);
		
	
	if (rand() % 1000 < 10){
		if (root_road -> in_range_car_p == NULL){
			printf("createCar\n");
			createCar(root_road, 1.9 + (rand() % 2) - 1.0, true);
			printf("---createCar\n");
		}
	}
	if (rand() % 1000 < 10){
		if (road_last -> in_range_car_n == NULL){
			printf("createCar\n");
			createCar(road_last, 1.9 + (rand() % 2) - 1.0, false);
			printf("---createCar\n");
		}
	}
	
	glutPostRedisplay();
	printf("---idle\n");
}

void init(char *progname){
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
	
	// set window attribute
	int width = 500, height = 500;
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(width, height);
	
	glutCreateWindow(progname);
	glClearColor(0.5, 0.8, 1.0, 1.0);
	
	glEnable(GL_MAP2_VERTEX_3);
	glMapGrid2f(30, 0, 1.0, 30, 0, 1.0);
	
	glViewport(0, 0, width, height);
	
	initLightning();
}

void onMousedown(int button, int state, int x, int y){
	if (state == GLUT_DOWN){
		switch(button){
			case GLUT_LEFT_BUTTON:
				exit(-1);
				break;
			default:
				break;
		}
	}
}

void onReshape(int w, int h){
	glViewport(0, 0, w, h);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0, (double)w/(double)h, 1.0, 100.0);
	
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0.4, 0.5, 3.5, 0, 0, 0, 0, 1.0, 0);
}

int main(int argc, char *argv[]){
	glutInit(&argc, argv);
	init(argv[0]);

	// register lister function
	glutDisplayFunc(display);
	glutKeyboardFunc(onKeydown);
	glutMouseFunc(onMousedown);
	glutIdleFunc(idle);
	glutReshapeFunc(onReshape);
	
	glShadeModel(GL_FLAT);
	glShadeModel(GL_SMOOTH);
	
	roadLength = bodyx + DOT*13;
	roadWidth = (roadLength - DOT)/2;
	carWidth = bodyz + DOT*2;
	carLength = bodyx + DOT;
	
	Road *r = addNormalRoad(root_road, &root_road, 2, NEXT);
	r = addNormalRoad(r, &r -> next, 2, NEXT);
	
	r = addIntersection(r, &r -> next, NEXT);
	addNormalRoad(r, &r -> right, 5, RIGHT);
	addNormalRoad(r, &r -> left, 5, LEFT);
	r -> left -> is_appended = true;
	r -> right -> is_appended = true;
	r -> traffic_light = createTrafficLight(&traffic_light_root, 900, 300);

	r = addNormalRoad(r, &r -> next, 2, NEXT);
	
	r = addIntersection(r, &r -> next, NEXT);
	addNormalRoad(r, &r -> right, 5, RIGHT);
	addNormalRoad(r, &r -> left, 5, LEFT);
	r -> left -> is_appended = true;
	r -> right -> is_appended = true;
	r -> traffic_light = createTrafficLight(&traffic_light_root, 600, 600);

	r = addNormalRoad(r, &r -> next, 2, NEXT);
	
	r = addIntersection(r, &r -> next, NEXT);
	addNormalRoad(r, &r -> right, 5, RIGHT);
	addNormalRoad(r, &r -> left, 5, LEFT);
	r -> left -> is_appended = true;
	r -> right -> is_appended = true;
	r -> traffic_light = createTrafficLight(&traffic_light_root, 300, 900);
	
	r = addNormalRoad(r, &r -> next, 2, NEXT);
	
	road_last = r;
	srand(time(NULL));

	glutMainLoop();
	
	return 0;
}
