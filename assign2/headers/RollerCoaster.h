#ifndef _ROLLER_COASTER_H_
#define _ROLLER_COASTER_H_

#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <jerror.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <iostream>
#include <sstream>
#include <pic.h>
#include <cmath>
#include <math.h>
#include <vector>

// The Gravity Constant
const double GRAVITY = 9.81;

// Max Height of Track
const double MAX_HEIGHT = 26.00;

// Delta Time
const double deltaTime = 0.004;

// Window Width/Height
const int WIN_WIDTH = 640;
const int WIN_HEIGHT = 480;

// Window Origin X/Y Pos
const int ORIGIN_X = 0;
const int ORIGIN_Y = 0;

// Maximum number of Screen Captures
const int MAX_SCREEN_CAPTURES = 958;

/**
 * RailSide - Enum determining which Side
 *            of the Rail to draw
 */
enum RailSide
{
    RAIL_LEFT, RAIL_RIGHT, RAIL_CENTER
};

/**
 * point - represents a point in OpenGL Space
 */
struct point
{
    double x;
    double y;
    double z;
};

/**
 * spline - contains how many control points, and an array of control points
 */
struct spline
{
    int numControlPoints;
    struct point *points;
};

// World Transform Enum
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
CONTROLSTATE g_ControlState = ROTATE;

class RollerCoaster
{
public:
private:

};

#endif // _ROLLER_COASTER_H_
