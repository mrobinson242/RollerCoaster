/*                                            */
/*  CSCI 420 Computer Graphics                */
/*  Assignment 2: Simulating a Roller Coaster */
/*  Author: Matt Robinson                     */
/*  Student Id: 9801107811                    */
/*                                            */

// Headers
#include "RollerCoaster.h"

// Right Click Menu Id
int _menuId;

// Current Mouse Position
int _mousePos[2] = {0, 0};

//  Mouse Buttons
int _leftMouseButton = 0;    /* 1 if pressed, 0 if not */
int _middleMouseButton = 0;
int _rightMouseButton = 0;

// Enabled Indicators
bool _translateEnabled;
bool _captureMode;

// State of the World Transformations */
float _theta[3] = {0, 0, 0};
float _translate[3] = {0, 0, 0};
float _scale[3] = {1, 1, 1};

// Saved File Counter
int _savedFileCount = 0;

// Camera Index
int _cameraIndex = 0;

// Number of Textures
GLuint texture[6];
Pic* textureArray[6];

// The spline array
struct spline *gSplines;

// Total number of splines from file
int g_iNumOfSplines;

// Total number of generated spline points
int _numSplinePoints;

// Vectors for Spline Values
std::vector<point> _splinePoints;
std::vector<point> _tangentPoints;
std::vector<point> _normalPoints;
std::vector<point> _biNormalPoints;

// Initialize Display List Index
GLuint _listIndex;

/**
 * loadSplines - Loads the Splines in from a .sp file
 */
int loadSplines(char *argv)
{
    char *cName = (char *)malloc(128 * sizeof(char));
    FILE *fileList;
    FILE *fileSpline;
    int iType, i = 0, j, iLength;

    // load the track file
    fileList = fopen(argv, "r");
    if (fileList == NULL)
    {
        printf ("can't open file\n");
        exit(1);
    }

    /* stores the number of splines in a global variable */
    fscanf(fileList, "%d", &g_iNumOfSplines);

    gSplines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

    /* reads through the spline files */
    for (j = 0; j < g_iNumOfSplines; j++)
    {
        i = 0;
        fscanf(fileList, "%s", cName);
        fileSpline = fopen(cName, "r");

        if (fileSpline == NULL) {
            printf ("can't open file\n");
            exit(1);
        }

        /* gets length for spline file */
        fscanf(fileSpline, "%d %d", &iLength, &iType);

        /* allocate memory for all the points */
        gSplines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
        gSplines[j].numControlPoints = iLength;

        /* saves the data to the struct */
        while (fscanf(fileSpline, "%lf %lf %lf",
                &gSplines[j].points[i].x,
                &gSplines[j].points[i].y,
                &gSplines[j].points[i].z) != EOF)
        {
            i++;
        }
    }

    free(cName);

    return 0;
}

/**
 * texLoad - loads a Texture from a JPEG Image
 */
void texLoad(int i, char* fileName)
{
    // Create an Image
    Pic *img;

    // Read in Image from File
    img = jpeg_read(fileName, NULL);
    textureArray[i] = img;

    // Make Texture the Current Active Texture
    glBindTexture(GL_TEXTURE_2D, texture[i]);

    // Load Image Data into Texture Object
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->nx, img->ny, 0, GL_RGB, GL_UNSIGNED_BYTE, &img->pix[0]);
}

/**
 * saveScreenshot - Write a screenshot to the
 *                  specified filename
 */
void saveScreenshot (char *filename)
{
  int i, j;
  Pic *in = NULL;

  if (filename == NULL)
    return;

  /* Allocate a picture buffer */
  in = pic_alloc(640, 480, 3, NULL);

  // Log File Location
  printf("File to save to: %s\n", filename);

  for (i=479; i>=0; i--)
  {
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  }

  if (jpeg_write(filename, in))
  {
    // Log Successful File Saving
    printf("File saved Successfully\n");

    // Increment File Count
    _savedFileCount++;
  }
  else
  {
    // Log Error in File Saving
    printf("Error in Saving\n");
  }

  pic_free(in);
}

/**
 * captureScreenshot - Captures a ScreenShot of the Display
 */
void captureScreenshot()
{
    // Initialize StringStream
    std::stringstream ss;

    // Get Saved Image Count
    ss << _savedFileCount;
    std::string s = ss.str();

    // Clear StringStream
    ss.str(std::string());

    // Check if Length of String
    if(s.length() == 1)
    {
        ss << "00";
        ss << s;
    }
    else if(s.length() == 2)
    {
        ss << "0";
        ss << s;
    }
    else
    {
        ss << s;
    }

    // Add File Extension
    ss << ".jpg";

    // Create File String
    std::string fileString = ss.str();
    char* fileName = (char*) fileString.c_str();

    // Save Screenshot
    saveScreenshot(fileName);
}

/**
 * addVectors - Does Vector Addition
 *              on Two Vectors
 */
point addVectors(point v1, point v2)
{
    // Initialize Added Vector
    point returnVector;

    returnVector.x = v1.x + v2.x;
    returnVector.y = v1.y + v2.y;
    returnVector.z = v1.z + v2.z;

    return returnVector;
}

/**
 * negateVector - Negates the Values
 *                of a Vector
 */
point negateVector(point v)
{
   // Initialize Negated Vector
    point negatedVector;

    negatedVector.x = -v.x;
    negatedVector.y = -v.y;
    negatedVector.z = -v.z;

    return negatedVector;
}

/**
 * magnitude - Gets the Magnitude
 *             of a Vector
 */
double magnitude(point v)
{
    // Calculate Length (Magnitude) of the Vector
    double magnitude = sqrt(pow(v.x,2) + pow(v.y,2) + pow(v.z,2));

    return magnitude;
}

/**
 * normalize - Normalizes a Vector
 *             to get the Unit Vector
 */
point normalize(point v)
{
    // Create Unit Vector
    point unitVector;

    // Calculate Length (Magnitude) of the Vector
    double length = magnitude(v);

    unitVector.x = v.x/length;
    unitVector.y = v.y/length;
    unitVector.z = v.z/length;

    return unitVector;
}

/**
 * scalarMultiply - Does Scalar Multiplication
 *                  of a Vector v by Scalar s
 */
point scalarMultiply(double s, point v)
{
    // Initialize Scaled Vector
    point rVector;

    rVector.x = s * v.x;
    rVector.y = s * v.y;
    rVector.z = s * v.z;

    return rVector;
}

/**
 * subtractVectors - Does Vector Subtraction
 *                   on two Vectors
 */
point subtractVectors(point v1, point v2)
{
    // Initialize Subtracted Vector
    point returnVector;

    returnVector.x = v1.x - v2.x;
    returnVector.y = v1.y - v2.y;
    returnVector.z = v1.z - v2.z;

    return returnVector;
}

/**
 * drawRail - Draws the Rail Track of the Roller Coaster
 */
point* drawRail(point v0, point v1, point v2, point v3,
                point v4, point v5, point v6, point v7,
                point b1, point b2, RailSide side)
{

    // Initialize Points Array
    point *points = new point[8];

    // Initialize Transform Points
    point t1;
    point t2;

    // Left Rail Side
    if(RAIL_LEFT == side)
    {
        // Set Color
        glColor3f(0.0f, 0.0f, 0.0f);

        t1 = scalarMultiply(-0.5, b1);
        t2 = scalarMultiply(-0.5, b2);
    }
    // Center Rail Side
    else if(RAIL_CENTER == side)
    {
        // Set Color
        glColor3f(0.0f, 0.0f, 0.0f);

        t1 = scalarMultiply(-0.2, b1);
        t2 = scalarMultiply(-0.2, b2);
    }
    // Right Rail Side
    else
    {
        // Set Color
        glColor3f(0.0f, 0.0f, 0.0f);

        t1 = scalarMultiply(0.5, b1);
        t2 = scalarMultiply(0.5, b2);
    }

    // Initialize Points
    point p0 = addVectors(v0, t1); point p1 = addVectors(v1, t1);
    point p2 = addVectors(v2, t1); point p3 = addVectors(v3, t1);
    point p4 = addVectors(v4, t2); point p5 = addVectors(v5, t2);
    point p6 = addVectors(v6, t2); point p7 = addVectors(v7, t2);

    // Set Points Array
    points[0] = p0; points[1] = p1;
    points[2] = p2; points[3] = p3;
    points[4] = p4; points[5] = p5;
    points[6] = p6; points[7] = p7;

    // Draw Top Face of Rail Quad
    glBegin(GL_QUADS);
        glVertex3f(p1.x, p1.y, p1.z);
        glVertex3f(p5.x, p5.y, p5.z);
        glVertex3f(p4.x, p4.y, p4.z);
        glVertex3f(p0.x, p0.y, p0.z);
    glEnd();

    // Draw Right Face of Rail Quad
    glBegin(GL_QUADS);
        glVertex3f(p2.x, p2.y, p2.z);
        glVertex3f(p6.x, p6.y, p6.z);
        glVertex3f(p5.x, p5.y, p5.z);
        glVertex3f(p1.x, p1.y, p1.z);
    glEnd();

    // Draw Bottom Face of Rail Quad
    glBegin(GL_QUADS);
        glVertex3f(p7.x, p7.y, p7.z);
        glVertex3f(p6.x, p6.y, p6.z);
        glVertex3f(p2.x, p2.y, p2.z);
        glVertex3f(p3.x, p3.y, p3.z);
    glEnd();

    // Draw Left Face of Rail Quad
    glBegin(GL_QUADS);
        glVertex3f(p0.x, p0.y, p0.z);
        glVertex3f(p4.x, p4.y, p4.z);
        glVertex3f(p7.x, p7.y, p7.z);
        glVertex3f(p3.x, p3.y, p3.z);
    glEnd();

    return points;
}

/**
 * drawLeftConnector - Draws the Left Cross-Section
 *                     of the Roller Coaster
 */
void drawLeftConnector(point l1, point l2, point l5, point l6,
                       point m0, point m3, point m4, point m7)
{
    // Set Color
    glColor3f(0.8f, 0.33f, 0.0f);

    // Draw Right Face of Left Connector
    glBegin(GL_QUADS);
        glVertex3f(l1.x, l1.y, l1.z);
        glVertex3f(l2.x, l2.y, l2.z);
        glVertex3f(m3.x, m3.y, m3.z);
        glVertex3f(m0.x, m0.y, m0.z);
    glEnd();

    // Draw Top Face of Left Connector
    glBegin(GL_QUADS);
        glVertex3f(l5.x, l5.y, l5.z);
        glVertex3f(l1.x, l1.y, l1.z);
        glVertex3f(m0.x, m0.y, m0.z);
        glVertex3f(m4.x, m4.y, m4.z);
    glEnd();

    // Draw Left Face of Left Connector
    glBegin(GL_QUADS);
        glVertex3f(l6.x, l6.y, l6.z);
        glVertex3f(l5.x, l5.y, l5.z);
        glVertex3f(m4.x, m4.y, m4.z);
        glVertex3f(m7.x, m7.y, m7.z);
    glEnd();

    // Draw Bottom Face of Left Connector
    glBegin(GL_QUADS);
        glVertex3f(l6.x, l6.y, l6.z);
        glVertex3f(l2.x, l2.y, l2.z);
        glVertex3f(m3.x, m3.y, m3.z);
        glVertex3f(m7.x, m7.y, m7.z);
    glEnd();
}

/**
 * drawRightConnector - Draws the Right Cross Section of the
 *                      Roller Coaster
 */
void drawRightConnector(point m1, point m2, point m5, point m6,
                        point r0, point r3, point r4, point r7)
{
    // Set Color
    glColor3f(0.8f, 0.33f, 0.0f);

    // Draw Right Face of Right Connector
    glBegin(GL_QUADS);
        glVertex3f(m1.x, m1.y, m1.z);
        glVertex3f(m2.x, m2.y, m2.z);
        glVertex3f(r3.x, r3.y, r3.z);
        glVertex3f(r0.x, r0.y, r0.z);
    glEnd();

    // Draw Top Face of Right Connector
    glBegin(GL_QUADS);
        glVertex3f(m5.x, m5.y, m5.z);
        glVertex3f(m1.x, m1.y, m1.z);
        glVertex3f(r0.x, r0.y, r0.z);
        glVertex3f(r4.x, r4.y, r4.z);
    glEnd();

    // Draw Left Face of Right Connector
    glBegin(GL_QUADS);
        glVertex3f(m6.x, m6.y, m6.z);
        glVertex3f(m5.x, m5.y, m5.z);
        glVertex3f(r4.x, r4.y, r4.z);
        glVertex3f(r7.x, r7.y, r7.z);
    glEnd();

    // Draw Bottom Face of Right Connector
    glBegin(GL_QUADS);
        glVertex3f(m6.x, m6.y, m6.z);
        glVertex3f(m2.x, m2.y, m2.z);
        glVertex3f(r7.x, r7.y, r7.z);
        glVertex3f(r3.x, r3.y, r3.z);
    glEnd();
}

/**
 * drawSupports - Draws the Rail Supports for the Roller Coaster
 */
void drawSupports(point p2, point p3, point p6, point p7)
{
    // Initialize Support Vector
    point supportVector;
    supportVector.x = 0.0;
    supportVector.y = -400.0;
    supportVector.z = 0.0;

    // Create Base Support Points
    point b2 = addVectors(p2, supportVector);
    point b3 = addVectors(p3, supportVector);
    point b6 = addVectors(p6, supportVector);
    point b7 = addVectors(p7, supportVector);

    // Set Color of Supports
    glColor3f(0.3f, 0.3f, 0.3f);

    // Draw Face of Support
    glBegin(GL_QUADS);
        glVertex3f(p2.x, p2.y, p2.z);
        glVertex3f(p3.x, p3.y, p3.z);
        glVertex3f(b3.x, b3.y, b3.z);
        glVertex3f(b2.x, b2.y, b2.z);
    glEnd();

    // Draw Face of Support
    glBegin(GL_QUADS);
        glVertex3f(p2.x, p2.y, p2.z);
        glVertex3f(p6.x, p6.y, p6.z);
        glVertex3f(b6.x, b6.y, b6.z);
        glVertex3f(b2.x, b2.y, b2.z);
    glEnd();

    // Draw Face of Support
    glBegin(GL_QUADS);
        glVertex3f(p7.x, p7.y, p7.z);
        glVertex3f(p6.x, p6.y, p6.z);
        glVertex3f(b6.x, b6.y, b6.z);
        glVertex3f(b7.x, b7.y, b7.z);
    glEnd();

    // Draw Face of Support
    glBegin(GL_QUADS);
        glVertex3f(p3.x, p3.y, p3.z);
        glVertex3f(p7.x, p7.y, p7.z);
        glVertex3f(b7.x, b7.y, b7.z);
        glVertex3f(b3.x, b3.y, b3.z);
    glEnd();
}

/**
 * drawCoaster - Draws the Combined parts of the Roller Coaster
 *               Calculates the Verties necessary to make the
 *               Rail Cross-Section
 */
void drawCoaster()
{
    // Scale Factor
    double s = 0.04;

    // Compile the Sky into the Display List
    glNewList(_listIndex+2, GL_COMPILE);

    // Iterate over all the Spline Points
    for (int i = 0; i < _numSplinePoints-1; i++)
    {
        // Calculate V0 (v0 = p0 + s(n0 - b0))
        point a0 = subtractVectors(_normalPoints[i], _biNormalPoints[i]);
        point s0 = scalarMultiply(s, a0);
        point v0 = addVectors(_splinePoints[i], s0);

        // Calculate V1 (v1 = p0 + s(n0 + b0))
        point a1 = addVectors(_normalPoints[i], _biNormalPoints[i]);
        point s1 = scalarMultiply(s, a1);
        point v1 = addVectors(_splinePoints[i], s1);

        // Calculate V2 (v2 = p0 + s(-n0 + b0))
        point n2 = negateVector(_normalPoints[i]);
        point a2 = addVectors(n2, _biNormalPoints[i]);
        point s2 = scalarMultiply(s, a2);
        point v2 = addVectors(_splinePoints[i], s2);

        // Calculate V3 (v3 = p0 + s(-n0 - b0))
        point n3 = negateVector(_normalPoints[i]);
        point a3 = subtractVectors(n3, _biNormalPoints[i]);
        point s3 = scalarMultiply(s, a3);
        point v3 = addVectors(_splinePoints[i], s3);

        // Calculate V4 (v4 = p1 + s(n1 - b1))
        point a4 = subtractVectors(_normalPoints[i+1], _biNormalPoints[i+1]);
        point s4 = scalarMultiply(s, a4);
        point v4 = addVectors(_splinePoints[i+1], s4);

        // Calculate V5 (v1 = p1 + s(n1 + b1))
        point a5 = addVectors(_normalPoints[i+1], _biNormalPoints[i+1]);
        point s5 = scalarMultiply(s, a5);
        point v5 = addVectors(_splinePoints[i+1], s5);

        // Calculate V6 (v6 = p1 + s(-n1 + b1))
        point n6 = negateVector(_normalPoints[i+1]);
        point a6 = addVectors(n6, _biNormalPoints[i+1]);
        point s6 = scalarMultiply(s, a6);
        point v6 = addVectors(_splinePoints[i+1], s6);

        // Calculate V7 (v7 = p1 + s(-n1 - b1))
        point n7 = negateVector(_normalPoints[i+1]);
        point a7 = subtractVectors(n7, _biNormalPoints[i+1]);
        point s7 = scalarMultiply(s, a7);
        point v7 = addVectors(_splinePoints[i+1], s7);

        // Draw Left Rail
        point* lRailPoints = drawRail(v0,v1,v2,v3,v4,v5,v6,v7, _biNormalPoints[i], _biNormalPoints[i+1], RAIL_LEFT);

        // Draw Right Rail
        point* rRailPoints = drawRail(v0,v1,v2,v3,v4,v5,v6,v7, _biNormalPoints[i], _biNormalPoints[i+1], RAIL_RIGHT);

        // Draw Middle Rail
        point* mRailPoints = drawRail(v0,v1,v2,v3,v4,v5,v6,v7, _normalPoints[i], _normalPoints[i+1], RAIL_CENTER);

        // Draw Rail Cross-Section every 30 Spline Points
        if(i % 30 == 0)
        {
            // Draw Left-Middle Track Connector
            drawLeftConnector(lRailPoints[1], lRailPoints[2], lRailPoints[5], lRailPoints[6],
                              mRailPoints[0], mRailPoints[3], mRailPoints[4], mRailPoints[7]);

            // Draw Right-Middle Track Connector
            drawRightConnector(mRailPoints[1], mRailPoints[2], mRailPoints[5], mRailPoints[6],
                               rRailPoints[0], rRailPoints[3], rRailPoints[4], rRailPoints[7]);
        }

        // Draw Rail Supports every 240 Spline Points
        if(i % 240 == 0)
        {
            // Draw Left Rail Supports
            drawSupports(lRailPoints[2], lRailPoints[3], lRailPoints[6], lRailPoints[7]);

            // Draw Right Rail Supports
            drawSupports(rRailPoints[2], rRailPoints[3], rRailPoints[6], rRailPoints[7]);
        }
    }

    // End Compilation of the Roller Coaster
    glEndList();
}

/**
 * drawGround - Initializes the Ground Texture. It then
 *              stores the Vertices into a Display List
 *              be later displayed
 */
void drawGround()
{
    // Compile the Ground into the Display List
    glNewList(_listIndex, GL_COMPILE);

    // Activate Ground Texture
    glEnable(GL_TEXTURE_2D);

    // Load Ground Texture
    glBindTexture(GL_TEXTURE_2D, texture[0]);

    // Initialize Texture Properties
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repeat Pattern in S
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repeat Pattern in T
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Bilinear Interpolation
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Bilinear Interpolation

    // Draw Ground
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 1.0f); glVertex3f(200.0f, -10.0, 200.0f);     // Vertex 1
        glTexCoord2f(0.0f, 0.0f); glVertex3f(200.0f, -10.0, -200.0f);   // Vertex 2
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-200.0f, -10.0, -200.0f);  // Vertex 3
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-200.0f, -10.0, 200.0f);   // Vertex 4
    glEnd();

    // Disable Ground Texture
    glDisable(GL_TEXTURE_2D);

    // End Compilation of the Ground
    glEndList();
}

/**
 * drawRightSky
 */
void drawRightSky()
{
    // Load Sky Texture
    glBindTexture(GL_TEXTURE_2D, texture[4]);

    // Initialize Texture Properties
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repeat Pattern in S
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repeat Pattern in T
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Bilinear Interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Bilinear Interpolation

    // Right Face
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(200.0, 190.0, 200.0);  // Vertex 1
        glTexCoord2f(1.0, 0.0); glVertex3f(200.0, 190.0, -200.0); // Vertex 2
        glTexCoord2f(1.0, 1.0); glVertex3f(200.0, -10.0, -200.0); // Vertex 3
        glTexCoord2f(0.0, 1.0); glVertex3f(200.0, -10.0, 200.0);  // Vertex 4
    glEnd();
}

/**
 * drawBackSky
 */
void drawBackSky()
{
    // Load Sky Texture
    glBindTexture(GL_TEXTURE_2D, texture[1]);

    // Initialize Texture Properties
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repeat Pattern in S
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repeat Pattern in T
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Bilinear Interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Bilinear Interpolation

    // Back Face
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(-200.0, 190.0, 200.0); // Vertex 1
        glTexCoord2f(1.0, 0.0); glVertex3f(200.0, 190.0, 200.0);  // Vertex 2
        glTexCoord2f(1.0, 1.0); glVertex3f(200.0, -10.0, 200.0);  // Vertex 3
        glTexCoord2f(0.0, 1.0); glVertex3f(-200.0, -10.0, 200.0); // Vertex 4
    glEnd();
}

/**
 * drawLeftSky
 */
void drawLeftSky()
{
    // Load Sky Texture
    glBindTexture(GL_TEXTURE_2D, texture[3]);

    // Initialize Texture Properties
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repeat Pattern in S
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repeat Pattern in T
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Bilinear Interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Bilinear Interpolation

    // Left Face
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(-200.0, 190.0, -200.0); // Vertex 1
        glTexCoord2f(1.0, 0.0); glVertex3f(-200.0, 190.0, 200.0);  // Vertex 2
        glTexCoord2f(1.0, 1.0); glVertex3f(-200.0, -10.0, 200.0);  // Vertex 3
        glTexCoord2f(0.0, 1.0); glVertex3f(-200.0, -10.0, -200.0); // Vertex 4
    glEnd();
}

/**
 * drawFrontSky
 */
void drawFrontSky()
{
    // Load Sky Texture
    glBindTexture(GL_TEXTURE_2D, texture[2]);

    // Initialize Texture Properties
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repeat Pattern in S
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repeat Pattern in T
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Bilinear Interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Bilinear Interpolation

    // Front Face
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(200.0, 190.0, -200.0); // Vertex 1
        glTexCoord2f(1.0, 0.0); glVertex3f(-200.0, 190.0, -200.0); // Vertex 2
        glTexCoord2f(1.0, 1.0); glVertex3f(-200.0, -10.0, -200.0); // Vertex 3
        glTexCoord2f(0.0, 1.0); glVertex3f(200.0, -10.0, -200.0); // Vertex 4
    glEnd();
}

/**
 * drawTopSky
 */
void drawTopSky()
{
    // Load Sky Texture
    glBindTexture(GL_TEXTURE_2D, texture[5]);

    // Initialize Texture Properties
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repeat Pattern in S
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repeat Pattern in T
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Bilinear Interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Bilinear Interpolation

    // Top Face
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(-200.0, 190.0, 200.0);  // Vertex 1
        glTexCoord2f(1.0, 0.0); glVertex3f(200.0, 190.0, 200.0);   // Vertex 2
        glTexCoord2f(1.0, 1.0); glVertex3f(200.0, 190.0, -200.0);  // Vertex 3
        glTexCoord2f(0.0, 1.0); glVertex3f(-200.0, 190.0, -200.0); // Vertex 4
    glEnd();
}

/**
 * drawSky - Constructs the SkyBox
 */
void drawSky()
{
    // Compile the Sky into the Display List
    glNewList(_listIndex+1, GL_COMPILE);

    // Activate Texture
    glEnable(GL_TEXTURE_2D);

    // Construct Sky Box
    drawLeftSky();  // Left Face
    drawRightSky(); // Right Face
    drawBackSky();  // Back Face
    drawFrontSky(); // Front Face
    drawTopSky();   // Top Face

    // Disable Texture
    glDisable(GL_TEXTURE_2D);

    // End Compilation of the Ground
    glEndList();
}

/**
 * getCurvePoint - Calculates a Point on the Spline, given a set of Control Points
 *                 and a value u that signifies the portion of the distance
 *                 between the two nearest control points
 *
 * param p0 - Control Point P0
 * param p1 - Control Point P1
 * param p2 - Control Point P2
 * param p3 - Control Point P3
 * param u  - Distance between two nearest control points
 */
double getCurvePoint(double p0, double p1, double p2, double p3, double u)
{
    // Create Spline Point
    double splinePoint;

    // Calculate Parameters [u^3 u^2 u 1] * [-1  3 -3  1] * [p0]
    //                                      [ 2 -5  4 -1]   [p1]
    //                                      [-1  0  1  0]   [p2]
    //                                      [ 0  2  0  0]   [p3]
    double param1 = (-p0 + 3*p1 - 3*p2 + p3) * pow(u,3);
    double param2 = (2*p0 - 5*p1 + 4*p2 - p3) * pow(u,2);
    double param3 = (-p0 + p2) * u;
    double param4 = (2 * p1);

    // Initialize Spline Point
    splinePoint = param1 + param2 + param3 + param4;

    return splinePoint;
}

/**
 * getSplineTangent - Calculates a Tangent on the Curve, given a set of Control Points
 *                    and a value t that signifies the portion of the distance
 *                    between the two nearest control points
 *
 * param p0 - Control Point P0
 * param p1 - Control Point P1
 * param p2 - Control Point P2
 * param p3 - Control Point P3
 * param t - Distance between two nearest control points
 */
double getSplineTangent(double p0, double p1, double p2, double p3, double u)
{
    // Create Spline Tangent Point
    double tangentPoint;

    // Calculate Parameters [3u^2 2u 1 0] * [-1  3 -3  1] * [p0]
    //                                      [ 2 -5  4 -1]   [p1]
    //                                      [-1  0  1  0]   [p2]
    //                                      [ 0  2  0  0]   [p3]
    double param1 = (-p0 + 3*p1 - 3*p2 + p3) * (3 * pow(u,2));
    double param2 = (2*p0 - 5*p1 + 4*p2 -p3) * (2*u);
    double param3 = -p0 + p2;

    // Initialize Spline Tangent Point
    tangentPoint = param1 + param2 + param3;

    return tangentPoint;
}

/**
 * calcCurvePoint - Calculates a Point on the Spline Curve
 *
 * @param p0 - Control Point p0
 * @param p1 - Control Point p1
 * @param p2 - Control Point p2
 * @param p3 - Control Point p3
 * @param u  - Distance between two nearest control points
 */
point calcCurvePoint(point p0, point p1, point p2, point p3, double u)
{
    // Create Spline Point
    point splinePoint;

    // Calculate X,Y,Z Location of the Spline Point
    splinePoint.x = 0.5 * getCurvePoint(p0.x, p1.x, p2.x, p3.x, u);
    splinePoint.y = 0.5 * getCurvePoint(p0.y, p1.y, p2.y, p3.y, u);
    splinePoint.z = 0.5 * getCurvePoint(p0.z, p1.z, p2.z, p3.z, u);

    return splinePoint;
}

/**
 * calcTangentPoint - Calculates the Tangent of a
 *                    Point on the Spline Curve
 *
 * @param p0 - Control Point p0
 * @param p1 - Control Point p1
 * @param p2 - Control Point p2
 * @param p3 - Control Point p3
 * @param u  - Distance between two nearest control points
 */
point calcTangentPoint(point p0, point p1, point p2, point p3, double u)
{
    // Create Tangent Point
    point tangentPoint;

    // Calculate X,Y,Z Location of the Spline Point
    tangentPoint.x = 0.5 * getSplineTangent(p0.x, p1.x, p2.x, p3.x, u);
    tangentPoint.y = 0.5 * getSplineTangent(p0.y, p1.y, p2.y, p3.y, u);
    tangentPoint.z = 0.5 * getSplineTangent(p0.z, p1.z, p2.z, p3.z, u);

    return tangentPoint;
}

/**
 * crossProduct - Calcuates the Cross Product
 *                betweens Vectors A and B, returning
 *                an Orthogonal ("Normal") Vector C
 *
 * @param a - Vector A
 * @param b - Vector B
 */
point crossProduct(point a, point b)
{
    // Initialize the Return Vector
    point v;

    // Calculate the Cross Product
    v.x = (a.y * b.z) - (a.z * b.y);
    v.y = (a.z * b.x) - (a.x * b.z);
    v.z = (a.x * b.y) - (a.y * b.x);

    return v;
}

/**
 * generateSplinePoints
 */
void generateSplinePoints(spline* splines)
{
    // Get Number of Control Points
    int numControlPoints = splines[0].numControlPoints;

    // Index into Spline Array
    int index = 0;

    // Iterate over Splines Array
    for(int i = 0; i < numControlPoints; ++i)
    {
        // Initialize u
        double u = 0.005;

        // Increment u with fixed step size
        while(u < 1.0)
        {
            // Create Control Points
            point p0, p1, p2, p3;

            if(i == numControlPoints-3)
            {
                // Get Points From Array
                p0 = splines[0].points[i];
                p1 = splines[0].points[i+1];
                p2 = splines[0].points[i+2];
                p3 = splines[0].points[0];
            }
            else if(i== numControlPoints-2)
            {
                // Get Points From Array
                p0 = splines[0].points[i];
                p1 = splines[0].points[i+1];
                p2 = splines[0].points[0];
                p3 = splines[0].points[1];
            }
            else if(i== numControlPoints-1)
            {
                // Get Points From Array
                p0 = splines[0].points[i];
                p1 = splines[0].points[0];
                p2 = splines[0].points[1];
                p3 = splines[0].points[2];
            }
            else
            {
                // Get Points From Array
                p0 = splines[0].points[i];
                p1 = splines[0].points[i+1];
                p2 = splines[0].points[i+2];
                p3 = splines[0].points[i+3];
            }

            // Initialize Points
            point splinePoint;
            point tangentPoint;
            point normalPoint;
            point biNormalPoint;

            // Calculate the Point on the Curve
            splinePoint = calcCurvePoint(p0, p1, p2, p3, u);
            point tPoint = calcTangentPoint(p0, p1, p2, p3, u);
            point nPoint;
            point bPoint;

            // Normlize Tangent Vector
            tangentPoint = normalize(tPoint);

            // If first point in chain of spline
            // splines derive N with T and B
            if(index == 0)
            {
                // Initial Guess of B
                point guessPoint;
                guessPoint.x = 0;
                guessPoint.y = 0;
                guessPoint.z = -1;

                // Calculate the Normal Point (T(i) X B(i))
                nPoint = crossProduct(tangentPoint, guessPoint);

                // Normalize Vectors
                normalPoint = normalize(nPoint);

                // Calculate the Binormal Point (T(i) X (N(i))
                bPoint = crossProduct(tangentPoint, normalPoint);

                // Normalize Binormal Vector
                biNormalPoint = normalize(bPoint);
            }
            else
            {
                // Get Previous Binormal Point
                point previousBinormal = _biNormalPoints[index-1];

                // Get Update Normal by computing the cross product
                // of the current tangent with the previous binormal
                // (B(i-1) X T(I))
                nPoint = crossProduct(previousBinormal, tangentPoint);

                // Normalize Normal Vector
                normalPoint = normalize(nPoint);

                // Calculate the Binormal Point (T(i) X (N(i))
                bPoint = crossProduct(tangentPoint, normalPoint);

                // Normalize Binormal Vector
                biNormalPoint = normalize(bPoint);
            }

            // Add Point to Vector
            _splinePoints.push_back(splinePoint);
            _tangentPoints.push_back(tangentPoint);
            _normalPoints.push_back(normalPoint);
            _biNormalPoints.push_back(biNormalPoint);

            // Update u (u(new) = u(current) + (deltaTime) * sqrt(2*g*(h(max) - h)) / ||(dp/du)||
            u += deltaTime * (sqrt(2*GRAVITY*(MAX_HEIGHT - splinePoint.y))/magnitude(tangentPoint));

            // Increment Index
            index++;
        }
    }

    // Set Actual Number of Spline Points
    _numSplinePoints = index;
}

/**
 * updateCamera
 */
void updateCamera()
{
    // Initialize Camera
    point eye;
    point center;
    point up;

    // Update Up Vector
    up.x = _normalPoints[_cameraIndex].x;
    up.y = _normalPoints[_cameraIndex].y;
    up.z = _normalPoints[_cameraIndex].z;

    // Update Eye Position (p(u))
    eye.x = _splinePoints[_cameraIndex].x + (0.25 * up.x);
    eye.y = _splinePoints[_cameraIndex].y + (0.25 * up.y);
    eye.z = _splinePoints[_cameraIndex].z + (0.25 * up.z);

    // Update Center Position (p(u) + a*t(u))
    double a = 1.0;
    center.x = eye.x + (a * _tangentPoints[_cameraIndex].x);
    center.y = eye.y + (a * _tangentPoints[_cameraIndex].y);
    center.z = eye.z + (a * _tangentPoints[_cameraIndex].z);

    // Update Camera Index
    if(_cameraIndex == _numSplinePoints-1)
    {
        _cameraIndex = 0;
    }
    else
    {
        _cameraIndex++;
    }

    //Update Camera Position
    gluLookAt(eye.x, eye.y, eye.z,
              center.x, center.y, center.z,
              up.x, up.y, up.z);
}
/**
 * display
 */
void display()
{
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset transformation
    glLoadIdentity();

    // Use Texture Color Directly
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // Transformations
    glScalef(_scale[0], _scale[1], _scale[2]);
    glRotatef(_theta[0], 1, 0, 0);
    glRotatef(_theta[1], 0, 1, 0);
    glRotatef(_theta[2], 0, 0, 1);
    glTranslatef(_translate[0], _translate[1], _translate[2]);

    // Update Camera
    updateCamera();

    // Draw
    glCallList(_listIndex);   // Ground
    glCallList(_listIndex+1); // Sky Box
    glCallList(_listIndex+2); // Roller Coaster

    // Double buffer flush
    glutSwapBuffers();
}

/*
 * reshape - Called every time window is resized
 *           to update the projection matrix, and
 *           to preserve aspect ratio
 */
void reshape(int w, int h)
{
    // Initialize Aspect
    GLfloat aspect = (GLfloat)w/(GLfloat)h;

    // Setup image size
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Setup camera
    gluPerspective(60.0, aspect, 0.01, 1000.0);

    // Set back to ModelView
    glMatrixMode(GL_MODELVIEW);
}

/**
 * mousebutton
 */
void mousebutton(int button, int state, int x, int y)
{
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      _leftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      _middleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      _rightMouseButton = (state==GLUT_DOWN);
      break;
  }

  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      g_ControlState = TRANSLATE;
      break;

    case GLUT_ACTIVE_SHIFT:
      g_ControlState = SCALE;
      break;

    default:
      if(_translateEnabled)
      {
          g_ControlState = TRANSLATE;
      }
      else
      {
          g_ControlState = ROTATE;
      }
      break;
  }

  _mousePos[0] = x;
  _mousePos[1] = y;
}

/**
 * mouseDrag - Converts mouse drags into information
 *             about rotation/translation/scaling
 */
void mousedrag(int x, int y)
{
    // Update Change in Mouse Position
    int vMouseDelta[2] = {x - _mousePos[0], y - _mousePos[1]};

    // Check Control State
    switch (g_ControlState)
    {
        // Translate Transformation
        case TRANSLATE:
            if (_leftMouseButton)
            {
                _translate[0] += 0.01 * vMouseDelta[0];
                _translate[1] -= 0.01 * vMouseDelta[1];
            }
            else if (_middleMouseButton)
            {
                _translate[2] += 0.01 * vMouseDelta[1];
            }
            break;

        // Rotate Transformation
        case ROTATE:
          if (_leftMouseButton)
          {
            _theta[0] += vMouseDelta[1];
            _theta[1] += vMouseDelta[0];
          }
          else if (_middleMouseButton)
          {
            _theta[2] += vMouseDelta[1];
          }
          break;

        // Scale Transformation
        case SCALE:
          if (_leftMouseButton)
          {
            _scale[0] *= 1.0+vMouseDelta[0]*0.01;
            _scale[1] *= 1.0-vMouseDelta[1]*0.01;
          }
          else if (_middleMouseButton)
          {
            _scale[2] *= 1.0-vMouseDelta[1]*0.01;
          }
          break;
      }

      // Update Current Mouse Position
      _mousePos[0] = x;
      _mousePos[1] = y;
}

/**
 * menufunc - Handles Callbacks to Selections in Right Click Menu
 */
void menufunc(int value)
{
  switch (value)
  {
    case 0:
      exit(0);
      break;

    case 1:
      // Initialize StringStream
      std::stringstream ss;

      // Get Saved Image Count
      ss << _savedFileCount;
      std::string s = ss.str();

      // Clear StringStream
      ss.str(std::string());

      // Check if Length of String
      if(s.length() == 1)
      {
          ss << "00";
          ss << s;
      }
      else if(s.length() == 2)
      {
          ss << "0";
          ss << s;
      }
      else
      {
          ss << s;
      }

      // Add File Extension
      ss << ".jpg";

      // Create File String
      std::string fileString = ss.str();
      char* fileName = (char*) fileString.c_str();

      // Save Screenshot
      saveScreenshot(fileName);
      break;
  }
}

/**
 * mouseidle
 */
void mouseidle(int x, int y)
{
  _mousePos[0] = x;
  _mousePos[1] = y;
}

/**
 * processKeys - Handles Callbacks for Keyboard Keys
 */
void processKeys(unsigned char key, int x, int y)
{
    switch(key)
    {
        // Translate Case
        case 't':
        case 'T':
            // Check if Translate already Enabled
            if(_translateEnabled)
            {
                // Disable Translate Mode
                _translateEnabled = false;
            }
            else
            {
                // Enable Translate Mode
                _translateEnabled = true;
            }

            // Update Mouse Position
            _mousePos[0] = x;
            _mousePos[1] = y;
            break;

        // Enable Automatic Screenshot Captures
        case 's':
        case 'S':
            // Check if Capture Mode already enabled
            if(_captureMode)
            {
                // Disable Automatic Screen Shot Captures
                _captureMode = false;
            }
            else
            {
                // Enable Automatic Screen Shot Captures
                _captureMode = true;
            }
            break;

        // Z Key (Moves Camera in -Y Direction)
        case 'z':
        case 'Z':
            _translate[1] -= 1.0;
            break;

        // X Key (Moves Camera in Y Direction)
        case 'x':
        case 'X':
            _translate[1] += 1.0;
            break;
    }

    // Make the screen update
    glutPostRedisplay();
}

/**
 * processSpecialKeys - Handles Callbacks for Arrow Keys
 */
void processSpecialKeys(int key, int x, int y)
{
    switch(key)
    {
        // Up Key (Moves Camera in -X Direction)
        case GLUT_KEY_UP:
            _translate[0] -= 1.0;
            break;

        // Down Key (Moves Camera in X Direction)
        case GLUT_KEY_DOWN:
            _translate[0] += 1.0;
            break;

        // Left Key (Moves Camera in +Z Direction)
        case GLUT_KEY_LEFT:
            _translate[2] += 1.0;
            break;

        // Right Key (Moves Camera in -Z Direction)
        case GLUT_KEY_RIGHT:
            _translate[2] -= 1.0;
            break;
    }

    // Make the screen update
    glutPostRedisplay();
}

/**
 * idle - Callback that is called
 *        when no other events to be
 *        handled
 */
void idle()
{
    // If Capture Mode Enabled, take a Screen Capture every 4 frames (Max 3832)
    if((_cameraIndex % 4 == 0) && (_savedFileCount < MAX_SCREEN_CAPTURES) && _captureMode)
    {
       captureScreenshot();
    }

    // Make the screen update
    glutPostRedisplay();
}

/*
 * init
 */
void init()
{
    // Initialize Splines
    generateSplinePoints(gSplines);

    glClearColor(0.0, 0.0, 0.0, 0.0);   // set background color
    glEnable(GL_DEPTH_TEST);            // enable depth buffering
    glShadeModel(GL_SMOOTH);            // interpolate colors during rasterization

    // Initialize Display List
    _listIndex = glGenLists(3);

    // Initialize Keyboard Indicators
    _translateEnabled = false;
    _captureMode = false;

    // Initialize Textures
    glGenTextures(6, texture);
    texLoad(0, "images/mountains.jpg");
    texLoad(1, "images/skyBack.jpg");
    texLoad(2, "images/skyFront.jpg");
    texLoad(3, "images/skyLeft.jpg");
    texLoad(4, "images/skyRight.jpg");
    texLoad(5, "images/skyUp.jpg");

    // Initialize Objects
    drawGround();
    drawSky();
    drawCoaster();
}

/**
 * main
 */
int main (int argc, char ** argv)
{
    // Ensure Program has 2 Arguments
    if (argc < 2)
    {
        printf ("usage: %s <trackfile>\n", argv[0]);
        exit(0);
    }

    // Load Splines
    loadSplines(argv[1]);

    // Initialize GLUT
    glutInit(&argc,argv);

    // Request double buffer, depth, and color
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);

    // Set window size/position
    glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
    glutInitWindowPosition(ORIGIN_X, ORIGIN_Y);

    // Create Window
    glutCreateWindow("Roller Coaster");

    // GLUT Callbacks
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutMotionFunc(mousedrag);
    glutPassiveMotionFunc(mouseidle);
    glutReshapeFunc(reshape);
    glutMouseFunc(mousebutton);
    glutKeyboardFunc(processKeys);
    glutSpecialFunc(processSpecialKeys);

    // Create right mouse button menu
    _menuId = glutCreateMenu(menufunc);
    glutSetMenu(_menuId);
    glutAddMenuEntry("Quit", 0);
    glutAddMenuEntry("Save Image", 1);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    // Initialize States
    init();

    glutMainLoop();
    return 0;
}
