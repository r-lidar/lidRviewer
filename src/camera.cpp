#include "camera.h"
#include "sdlglutils.h"
#include "Frustum.h"

#include <Rcpp.h>
#include <cmath>

#include <GL/glu.h>

static bool InvertMatrix(const GLfloat m[16], GLfloat invOut[16])
{
  GLfloat inv[16], det;
  int i;

  inv[0] = m[5]  * m[10] * m[15] -
    m[5]  * m[11] * m[14] -
    m[9]  * m[6]  * m[15] +
    m[9]  * m[7]  * m[14] +
    m[13] * m[6]  * m[11] -
    m[13] * m[7]  * m[10];

  inv[4] = -m[4]  * m[10] * m[15] +
    m[4]  * m[11] * m[14] +
    m[8]  * m[6]  * m[15] -
    m[8]  * m[7]  * m[14] -
    m[12] * m[6]  * m[11] +
    m[12] * m[7]  * m[10];

  inv[8] = m[4]  * m[9] * m[15] -
    m[4]  * m[11] * m[13] -
    m[8]  * m[5] * m[15] +
    m[8]  * m[7] * m[13] +
    m[12] * m[5] * m[11] -
    m[12] * m[7] * m[9];

  inv[12] = -m[4]  * m[9] * m[14] +
    m[4]  * m[10] * m[13] +
    m[8]  * m[5] * m[14] -
    m[8]  * m[6] * m[13] -
    m[12] * m[5] * m[10] +
    m[12] * m[6] * m[9];

  inv[1] = -m[1]  * m[10] * m[15] +
    m[1]  * m[11] * m[14] +
    m[9]  * m[2] * m[15] -
    m[9]  * m[3] * m[14] -
    m[13] * m[2] * m[11] +
    m[13] * m[3] * m[10];

  inv[5] = m[0]  * m[10] * m[15] -
    m[0]  * m[11] * m[14] -
    m[8]  * m[2] * m[15] +
    m[8]  * m[3] * m[14] +
    m[12] * m[2] * m[11] -
    m[12] * m[3] * m[10];

  inv[9] = -m[0]  * m[9] * m[15] +
    m[0]  * m[11] * m[13] +
    m[8]  * m[1] * m[15] -
    m[8]  * m[3] * m[13] -
    m[12] * m[1] * m[11] +
    m[12] * m[3] * m[9];

  inv[13] = m[0]  * m[9] * m[14] -
    m[0]  * m[10] * m[13] -
    m[8]  * m[1] * m[14] +
    m[8]  * m[2] * m[13] +
    m[12] * m[1] * m[10] -
    m[12] * m[2] * m[9];

  inv[2] = m[1]  * m[6] * m[15] -
    m[1]  * m[7] * m[14] -
    m[5]  * m[2] * m[15] +
    m[5]  * m[3] * m[14] +
    m[13] * m[2] * m[7] -
    m[13] * m[3] * m[6];

  inv[6] = -m[0]  * m[6] * m[15] +
    m[0]  * m[7] * m[14] +
    m[4]  * m[2] * m[15] -
    m[4]  * m[3] * m[14] -
    m[12] * m[2] * m[7] +
    m[12] * m[3] * m[6];

  inv[10] = m[0]  * m[5] * m[15] -
    m[0]  * m[7] * m[13] -
    m[4]  * m[1] * m[15] +
    m[4]  * m[3] * m[13] +
    m[12] * m[1] * m[7] -
    m[12] * m[3] * m[5];

  inv[14] = -m[0]  * m[5] * m[14] +
    m[0]  * m[6] * m[13] +
    m[4]  * m[1] * m[14] -
    m[4]  * m[2] * m[13] -
    m[12] * m[1] * m[6] +
    m[12] * m[2] * m[5];

  inv[3] = -m[1] * m[6] * m[11] +
    m[1] * m[7] * m[10] +
    m[5] * m[2] * m[11] -
    m[5] * m[3] * m[10] -
    m[9] * m[2] * m[7] +
    m[9] * m[3] * m[6];

  inv[7] = m[0] * m[6] * m[11] -
    m[0] * m[7] * m[10] -
    m[4] * m[2] * m[11] +
    m[4] * m[3] * m[10] +
    m[8] * m[2] * m[7] -
    m[8] * m[3] * m[6];

  inv[11] = -m[0] * m[5] * m[11] +
    m[0] * m[7] * m[9] +
    m[4] * m[1] * m[11] -
    m[4] * m[3] * m[9] -
    m[8] * m[1] * m[7] +
    m[8] * m[3] * m[5];

  inv[15] = m[0] * m[5] * m[10] -
    m[0] * m[6] * m[9] -
    m[4] * m[1] * m[10] +
    m[4] * m[2] * m[9] +
    m[8] * m[1] * m[6] -
    m[8] * m[2] * m[5];

  det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

  if (det == 0)
    return false;

  det = 1.0 / det;

  for (i = 0; i < 16; i++)
    invOut[i] = inv[i] * det;

  return true;
}

Camera::Camera()
{
  const char *hand1[] =
  {
    /* width height num_colors chars_per_pixel */
    " 16 16 3 1 ",
    /* colors */
    "X c #000000",
    ". c #ffffff",
    "  c None",
    /* pixels */
    "       XX       ",
    "   XX X..XXX    ",
    "  X..XX..X..X   ",
    "  X..XX..X..X X ",
    "   X..X..X..XX.X",
    "   X..X..X..X..X",
    " XX X.......X..X",
    "X..XX..........X",
    "X...X.........X ",
    " X............X ",
    "  X...........X ",
    "  X..........X  ",
    "   X.........X  ",
    "    X.......X   ",
    "     X......X   ",
    "     X......X   ",
    "0,0"
  };

  const char *hand2[] =
  {
    /* width height num_colors chars_per_pixel */
    " 16 16 3 1 ",
    /* colors */
    "X c #000000",
    ". c #ffffff",
    "  c None",
    /* pixels */
    "                ",
    "                ",
    "                ",
    "                ",
    "    XX XX XX    ",
    "   X..X..X..XX  ",
    "   X........X.X ",
    "    X.........X ",
    "   XX.........X ",
    "  X...........X ",
    "  X...........X ",
    "  X..........X  ",
    "   X.........X  ",
    "    X.......X   ",
    "     X......X   ",
    "     X......X   ",
    "0,0"
  };

  const char *move[] =
  {
    /* width height num_colors chars_per_pixel */
    " 16 16 1 ",
    /* colors */
    "X c #000000",
    ". c #ffffff",
    "  c None",
    /* pixels */
    "       XX       ",
    "      X..X      ",
    "     X....X     ",
    "    X......X    ",
    "   X XX..XX X   ",
    "  X.X X..X X.X  ",
    " X..XXX..XXX..X ",
    "X..............X",
    "X..............X",
    " X..XXX..XXX..X ",
    "  X.X X..X X.X  ",
    "   X XX..XX X   ",
    "    X......X    ",
    "     X....X     ",
    "      X..X      ",
    "       XX       ",
    "0,0"
    };

  _hand1 = cursorFromXPM(hand1);
  _hand2 = cursorFromXPM(hand2);
  _move  = cursorFromXPM(move);

  SDL_SetCursor(_hand1);
  holdleft = false;
  holdright = false;
  angleY = 20;
  angleZ = -30;
  deltaX = 0;
  deltaY = 0;
  deltaZ = 0;
  distance = 300;
  changed = true;
  panSensivity = 10;
  rotateSensivity = 0.3;
  zoomSensivity = 30;
}

void Camera::OnMouseMotion(const SDL_MouseMotionEvent & event)
{
  if (holdleft) // Rotate
  {
    //printf("Rotate\n");
    angleZ += event.xrel*rotateSensivity;
    angleY += event.yrel*rotateSensivity;

    if (angleY > 90)
      angleY = 90;
    else if (angleY < -90)
      angleY = -90;

    changed = true;
  }
  else if (holdright) // Pan
  {
    panSensivity = distance*0.01;
    //printf("Pan\n");
    deltaX += event.xrel*panSensivity;
    deltaY -= event.yrel*panSensivity;
    changed = true;
  }
}

void Camera::OnMouseEvent(const SDL_MouseButtonEvent &event, const SDL_MouseWheelEvent &event_wheel)
{
  //printf("Mouse event\n");

  if (event.button == SDL_BUTTON_LEFT)
  {
    if ((holdleft) && (event.type == SDL_MOUSEBUTTONUP))
    {
      holdleft = false;
      SDL_SetCursor(_hand1);
    }
    else if ((!holdleft) && (event.type == SDL_MOUSEBUTTONDOWN))
    {
      holdleft = true;
      SDL_SetCursor(_hand2);
    }
  }
  else if (event.button == SDL_BUTTON_RIGHT)
  {
    if ((holdright) && (event.type == SDL_MOUSEBUTTONUP))
    {
      holdright = false;
      SDL_SetCursor(_hand1);
    }
    else if ((!holdright) && (event.type == SDL_MOUSEBUTTONDOWN))
    {
      holdright = true;
      SDL_SetCursor(_move);
    }
  }

  // Handle mouse wheel event separately
  if (event_wheel.type == SDL_MOUSEWHEEL)
  {
    //printf("Wheel even\n");

    if (event_wheel.y > 0)
    {
      //printf("Wheel up\n");
      distance += zoomSensivity;
      panSensivity = distance * 0.001;
      zoomSensivity = distance * 0.05;
      changed = true;
    }
    else if (event_wheel.y < 0)
    {
      //printf("Wheel down\n");
      distance -= zoomSensivity;
      panSensivity = distance * 0.001;
      zoomSensivity = distance * 0.05;
      changed = true;
    }
  }
}

void Camera::OnKeyboard(const SDL_KeyboardEvent & event)
{
  if ((event.type == SDL_KEYDOWN)&&(event.keysym.sym == SDLK_HOME))
  {
    angleY = 0;
    angleZ = 0;
    changed = true;
  }
}

void Camera::setPanSensivity(double sensivity)
{
  panSensivity = sensivity;
}

void Camera::setRotateSensivity(double sensivity)
{
  rotateSensivity = sensivity;
}

void Camera::setZoomSensivity(double sensivity)
{
  zoomSensivity = sensivity;
}

void Camera::setDeltaXYZ(double dx, double dy, double dz)
{
  deltaX = dx;
  deltaY = dy;
  deltaZ = dz;
}

void Camera::setDistance(double dist)
{
  if (dist > 0)
    distance = dist;
}

Camera::~Camera()
{
  SDL_FreeCursor(_hand1);
  SDL_FreeCursor(_hand2);
  SDL_SetCursor(NULL);
}

void Camera::look()
{
  glTranslated(deltaX, deltaY, 0.0);
  gluLookAt(distance,0,0,0,0,0,0,0,1);
  glRotated(angleY,0,1,0);
  glRotated(angleZ,0,0,1);
  glRotated(90, 0, 0, 1);

  frustum.CalculateFrustum();

  GLfloat viewMatrix[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, viewMatrix);

  /*glm::mat4 ConversionMatrix = glm::make_mat4(viewMatrix);
   glm::vec4 cameraPos4 = glm::inverse(ConversionMatrix) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
   glm::vec3 CameraPos = glm::vec3(cameraPos4);*/

  /*GLfloat CameraPos[4];
   CameraPos[0] = invMatrix[0] * 0.0f + invMatrix[4] * 0.0f + invMatrix[8]  * 0.0f + invMatrix[12] * 1.0f;
   CameraPos[1] = invMatrix[1] * 0.0f + invMatrix[5] * 0.0f + invMatrix[9]  * 0.0f + invMatrix[13] * 1.0f;
   CameraPos[2] = invMatrix[2] * 0.0f + invMatrix[6] * 0.0f + invMatrix[10] * 0.0f + invMatrix[14] * 1.0f;
   CameraPos[3] = invMatrix[3] * 0.0f + invMatrix[7] * 0.0f + invMatrix[11] * 0.0f + invMatrix[15] * 1.0f;*/

  GLfloat invMatrix[16];
  InvertMatrix(viewMatrix, invMatrix);

  x = invMatrix[12];
  y = invMatrix[13];
  z = invMatrix[14];
}

bool Camera::see(float px, float py, float pz, float hsize)
{
  return frustum.CubeInFrustum(px, py, pz, hsize);
}

