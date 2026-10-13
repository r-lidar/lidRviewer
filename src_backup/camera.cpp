#include "camera.h"

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

void Camera::rotate(int xrel, int yrel)
{
  angleZ += xrel*rotateSensivity;
  angleY += yrel*rotateSensivity;

  if (angleY > 90)
    angleY = 90;
  else if (angleY < -90)
    angleY = -90;

  changed = true;
}

void Camera::pan(int xrel, int yrel)
{
  panSensivity = distance*0.01;
  deltaX += xrel*panSensivity;
  deltaY -= yrel*panSensivity;
  changed = true;
}

void Camera::zoom(int zrel)
{
  if (zrel > 0)
  {
    distance += zoomSensivity;
    panSensivity = distance * 0.001;
    zoomSensivity = distance * 0.05;
  }
  else if (zrel < 0)
  {
    distance -= zoomSensivity;
    panSensivity = distance * 0.001;
    zoomSensivity = distance * 0.05;
  }

  changed = true;
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

void Camera::look()
{
  glTranslated(deltaX, deltaY, 0.0);
  gluLookAt(distance,0,0,0,0,0,0,0,1);
  glRotated(angleY,0,1,0);
  glRotated(angleZ,0,0,1);
  glRotated(90, 0, 0, 1);

  frustum.CalculateFrustum();

  // Get the camera position
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

