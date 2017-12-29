//----------------------------------------------------------------------------//
// viewer.cpp                                                                 //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//----------------------------------------------------------------------------//
// This program is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU General Public License as published by the Free //
// Software Foundation; either version 2 of the License, or (at your option)  //
// any later version.                                                         //
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// This program has been modified from its original version. It was last      //
// edited on or before January 1st, 2003 by AnthraX101. For information,      //
// contact AnthraX101@swirvemail.com                                          //
//                                                                            //
// Also see README.TXT for other version information and general help.        //
//----------------------------------------------------------------------------//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//----------------------------------------------------------------------------//
// Includes                                                                   //
//----------------------------------------------------------------------------//

#include "cv-viewer.h"
#include "cv-tick.h"

//----------------------------------------------------------------------------//
// The one and only Viewer instance                                           //
//----------------------------------------------------------------------------//

Viewer theViewer;

//----------------------------------------------------------------------------//
// Constructors                                                               //
//----------------------------------------------------------------------------//

Viewer::Viewer()
{
  m_width = 640;
  m_height = 480;
  m_bFullscreen = false;
  m_mouseX = 0;
  m_mouseY = 0;
  m_tiltAngle = -70.0f;
  m_twistAngle = -45.0f;
  m_distance = 100.0f;
  m_bLeftMouseButtonDown = false;
  m_bRightMouseButtonDown = false;
  m_lastTick = 0;
  m_bPaused = false;
  m_calCoreModel = 0;
  m_calCoreAnimation = 0;
  m_time = 0.0f;
  m_texture = 0;
  m_scale = 1.0f;
  m_lodLevel = 1.0f;
  m_vertexCount = 0;
  m_faceCount = 0;
}

//----------------------------------------------------------------------------//
// Destructor                                                                 //
//----------------------------------------------------------------------------//

Viewer::~Viewer()
{
}

//----------------------------------------------------------------------------//
// Get the viewer caption                                                     //
//----------------------------------------------------------------------------//

std::string Viewer::getCaption()
{
  return "- Cal3D MiniViewer -";
}

//----------------------------------------------------------------------------//
// Get the fullscreen flag                                                    //
//----------------------------------------------------------------------------//

bool Viewer::getFullscreen()
{
  return m_bFullscreen;
}

//----------------------------------------------------------------------------//
// Get the window height                                                      //
//----------------------------------------------------------------------------//

int Viewer::getHeight()
{
  return m_height;
}

//----------------------------------------------------------------------------//
// Get the window width                                                       //
//----------------------------------------------------------------------------//

int Viewer::getWidth()
{
  return m_width;
}

//----------------------------------------------------------------------------//
// Create the viewer                                                          //
//----------------------------------------------------------------------------//

bool Viewer::onCreate(int argc, char *argv[])
{
  // show some information
  std::cout << "o----------------------------------------------------------------o" << std::endl;
  std::cout << "|                      Cal3D MiniViewer 0.7                      |" << std::endl;
  std::cout << "|           (C) 2001, 2002 Bruno 'Beosil' Heidelberger           |" << std::endl;
  std::cout << "|  Modified by AnthraX101 for support for eGenesis style files.  |" << std::endl;
  std::cout << "|             Edited on or before January 1st, 2003.             |" << std::endl;
  std::cout << "|               Contact: AnthraX101@swirvemail.com               |" << std::endl;
  std::cout << "o----------------------------------------------------------------o" << std::endl;
  std::cout << "| This program is free software; you can redistribute it and/or  |" << std::endl;
  std::cout << "| modify it under the terms of the GNU General Public License as |" << std::endl;
  std::cout << "| published by the Free Software Foundation; either version 2 of |" << std::endl;
  std::cout << "| the License, or (at your option) any later version.            |" << std::endl;
  std::cout << "o----------------------------------------------------------------o" << std::endl;
  std::cout << std::endl;

  // parse the command line arguments
  int arg;
  for(arg = 1; arg < argc; arg++)
  {
    // check for fullscreen flag
    if(strcmp(argv[arg], "--fullscreen") == 0) m_bFullscreen = true;
    // check for window flag
    else if(strcmp(argv[arg], "--window") == 0) m_bFullscreen = false;
    // check for dimension flag
    else if((strcmp(argv[arg], "--dimension") == 0) && (argc - arg > 2))
    {
      m_width = atoi(argv[++arg]);
      m_height = atoi(argv[++arg]);
      if((m_width <= 0) || (m_height <= 0))
      {
        std::cerr << "Invalid dimension!" << std::endl;
        return false;
      }
    }
    else if((strcmp(argv[arg],"--model") == 0) && (argc - arg > 1))
    {
      if (m_calCoreModel != 0) {
		  std::cerr << "Specified two models!" << std::endl;
	return false;
      }
      CalLoader loader;
      loader.loadCoreModel(m_calCoreModel, argv[++arg]);
      if (m_calCoreModel == 0) {
		  std::cerr << "Could not load model file " << argv[arg] << std::endl;
	return false;
      }
    }
	else if((strcmp(argv[arg],"--mesh") == 0) && (argc - arg > 1))
    {
      if (m_calCoreModel != 0) {
		  std::cerr << "Specified two models!" << std::endl;
	return false;
      }
	  if (!argv[arg+1] || !argv[arg+2]) {
		  std::cerr << "Error in command line input. Expecting --mesh file1.cmf file2.csf" << std::endl;
	  }
	  else if (argv[arg+1] && argv[arg+2]) {
      CalLoader loader;
      loader.loadCoreModel(m_calCoreModel, argv[++arg], argv[++arg]);
      if (m_calCoreModel == 0) {
		  std::cerr << "Could not load skeleton or mesh file" << std::endl;
	return false;
      }
	  }
    }
    else if((strcmp(argv[arg],"--animation") == 0) && (argc - arg > 1))
    {
      if (m_calCoreAnimation != 0) {
	std::cerr << "Specified two animations!" << std::endl;
	return false;
      }
      CalLoader loader;
      loader.loadCoreAnimation(m_calCoreAnimation, argv[++arg]);
      if (m_calCoreAnimation == 0) {
		  std::cerr << "Could not load animation file " << argv[arg] << std::endl;
	return false;
      }
    }
    // check for help flag
    else if(strcmp(argv[arg], "--help") == 0)
    {
      std::cerr << "Usage: " << argv[0] << " [--fullscreen] [--window] [--dimension width height] [--help] [--model file.cdf] [--animation file.caf] [--mesh file.cmf file.csf]" << std::endl;
      return false;
    }
    // must be the model configuration file then
    else
    {
		std::cerr << "Unrecognized command line switch " << argv[arg] << std::endl;
      return false;
    }
  }

  // check if we have successfully loaded a model configuration
  if(m_calCoreModel == 0)
  {
    std::cerr << "No --model or --mesh specified, cannot proceed. Try --help for commands." << std::endl;
    return false;
  }
  
  // create the model instance from the loaded core model
  if(!m_calModel.create(m_calCoreModel))
  {
    CalError::printLastError();
    return false;
  }
  
  return true;
}

//----------------------------------------------------------------------------//
// Handle an idle event                                                       //
//----------------------------------------------------------------------------//

void Viewer::onIdle()
{
  // get the current tick value
  unsigned int tick;
  tick = Tick::getTick();

  // calculate the amount of elapsed seconds
  float elapsedSeconds;
  elapsedSeconds = (float)(tick - m_lastTick) / 1000.0f;
  if (m_bPaused) elapsedSeconds = 0.0;
  if (m_calCoreAnimation) {
    m_time += elapsedSeconds;
    float duration = m_calCoreAnimation->getDuration();
    while (m_time > duration) m_time -= duration;
  }
  
  // update the model
  m_calModel.clearState();
  if (m_calCoreAnimation) m_calModel.blendState(m_calCoreAnimation, 1.0, m_time);
  m_calModel.lockState();
  m_calModel.calculateState();
  m_calModel.updateSpringSystem(elapsedSeconds);
  m_calModel.updateVertices();
  
  // current tick will be last tick next round
  m_lastTick = tick;

// DEBUG-CODE //////////////////////////////////////////////////////////////////
/*
static unsigned int _frame = 0;
_frame++;
static unsigned int _tick = tick;
if(tick - _tick > 2000)
{
  std::cout << 1000.0f * _frame / (float)(tick - _tick) << " fps, #vertices = " << m_vertexCount << ", #faces = " << m_faceCount << std::endl;
  _frame = 0;
  _tick = tick;
}
*/
////////////////////////////////////////////////////////////////////////////////

  // update the screen
  glutPostRedisplay();
}

//----------------------------------------------------------------------------//
// Initialize the demo                                                        //
//----------------------------------------------------------------------------//

bool Viewer::onInit()
{
  // we're done
  std::cout << "Initialization done." << std::endl;
  std::cout << std::endl;
  std::cout << "Quit the viewer by pressing 'q' or ESC" << std::endl;
  std::cout << std::endl;
  
  m_lastTick = Tick::getTick();
  
  return true;
}

//----------------------------------------------------------------------------//
// Handle a key event                                                         //
//----------------------------------------------------------------------------//

void Viewer::onKey(unsigned char key, int x, int y)
{
  switch(key)
  {
    // test for quit event
    case 27:
    case 'q':
    case 'Q':
      exit(0);
      break;
    case '-':
      m_lodLevel -= 0.002f;
      if(m_lodLevel < 0.0f) m_lodLevel = 0.0f;
      break;
    case '+':
      m_lodLevel += 0.002f;
      if(m_lodLevel > 1.0f) m_lodLevel = 1.0f;
      break;
    // test for pause event
    case ' ':
      m_bPaused = !m_bPaused;
      break;
    // test for the lod keys
    default:
      if((key >= '0') && (key <= '9'))
      {
        m_lodLevel = (key == '0') ? 1.0f : (key - '0') * 0.1f;
      }
      break;
  }

  // set the (possible) new lod level
  m_calModel.setLodLevel(m_lodLevel);
}

//----------------------------------------------------------------------------//
// Handle a mouse button down event                                           //
//----------------------------------------------------------------------------//

void Viewer::onMouseButtonDown(int button, int x, int y)
{
  // update mouse button states
  if(button == GLUT_LEFT_BUTTON)
  {
    m_bLeftMouseButtonDown = true;
  }

  if(button == GLUT_RIGHT_BUTTON)
  {
    m_bRightMouseButtonDown = true;
  }

  // update internal mouse position
  m_mouseX = x;
  m_mouseY = y;
}

//----------------------------------------------------------------------------//
// Handle a mouse button up event                                             //
//----------------------------------------------------------------------------//

void Viewer::onMouseButtonUp(int button, int x, int y)
{
  // update mouse button states
  if(button == GLUT_LEFT_BUTTON)
  {
    m_bLeftMouseButtonDown = false;
  }

  if(button == GLUT_RIGHT_BUTTON)
  {
    m_bRightMouseButtonDown = false;
  }

  // update internal mouse position
  m_mouseX = x;
  m_mouseY = y;
}

//----------------------------------------------------------------------------//
// Handle a mouse move event                                                  //
//----------------------------------------------------------------------------//

void Viewer::onMouseMove(int x, int y)
{
  // update twist/tilt angles
  if(m_bLeftMouseButtonDown)
  {
    // calculate new angles
    m_twistAngle += (float)(x - m_mouseX);
    m_tiltAngle -= (float)(y - m_mouseY);
  }

  // update distance
  if(m_bRightMouseButtonDown)
  {
    // calculate new distance
    m_distance -= (float)(y - m_mouseY) / 3.0f;
    if(m_distance < 0.0f) m_distance = 0.0f;
  }

  // update internal mouse position
  m_mouseX = x;
  m_mouseY = y;
}

//----------------------------------------------------------------------------//
// Render the current scene                                                   //
//----------------------------------------------------------------------------//

void Viewer::onRender()
{
  // clear the vertex and face counters
  m_vertexCount = 0;
  m_faceCount = 0;

  // clear all the buffers
  glClearColor(0.0f, 0.0f, 0.3f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the projection transformation
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0f, (GLdouble)m_width / (GLdouble)m_height, m_scale, m_scale * 1000.0);

  // set the model transformation
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // light attributes
  const GLfloat light_ambient[]  = { 0.3f, 0.3f, 0.3f, 1.0f };
  const GLfloat light_diffuse[]  = { 0.52f, 0.5f, 0.5f, 1.0f };
  const GLfloat light_specular[] = { 0.1f, 0.1f, 0.1f, 1.0f };

  // setup the light attributes
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

  // set the light position
  GLfloat lightPosition[] = { 0.0f, -1.0f, 1.0f, 1.0f };
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

  // set camera position
  glTranslatef(0.0f, 0.0f, -m_distance * m_scale);
  glRotatef(m_tiltAngle, 1.0f, 0.0f, 0.0f);
  glRotatef(m_twistAngle, 0.0f, 0.0f, 1.0f);
  // glTranslatef(0.0f, 0.0f, -90.0f * m_scale);

  // render the model
  renderModel();

  // render the cursor
  renderCursor();

  // swap the front- and back-buffer
  glutSwapBuffers();
}

//----------------------------------------------------------------------------//
// Shut the viewer down                                                       //
//----------------------------------------------------------------------------//

void Viewer::onShutdown()
{
  // destroy model instance
  m_calModel.destroy();

  // destroy core animation instance
  if (m_calCoreAnimation) {
	m_calCoreAnimation->destroy();
	delete m_calCoreAnimation;
  }

  // destroy core model instance
  m_calCoreModel->destroy();
  delete m_calCoreModel;
}

//----------------------------------------------------------------------------//
// Render the cursor                                                          //
//----------------------------------------------------------------------------//

void Viewer::renderCursor()
{
  // switch to orthogonal projection for the cursor
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, (GLdouble)m_width, 0, (GLdouble)m_height, -1.0f, 1.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // render the cursor
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBegin(GL_TRIANGLES);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glVertex2i(m_mouseX, m_mouseY);
    glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
    glVertex2i(m_mouseX + 16, m_mouseY - 32);
    glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
    glVertex2i(m_mouseX + 32, m_mouseY - 16);
  glEnd();

  glDisable(GL_BLEND);
}

//----------------------------------------------------------------------------//
// Render the model                                                           //
//----------------------------------------------------------------------------//

void Viewer::renderModel()
{
  // set global OpenGL states
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  // we will use vertex arrays, so enable them
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  // get the number of meshes
  int submeshCount = m_calModel.getSubmeshCount();

  // render all meshes of the model
  int submeshId;
  for(submeshId = 0; submeshId < submeshCount; submeshId++)
  {
    CalSubmesh *submesh = m_calModel.getSubmesh(submeshId);
    
    int vertexCount = submesh->getVertexCount();
    int faceCount = submesh->getFaceCount();
    int *faces = submesh->getBufferedFaces();
    float *vertices = submesh->getBufferedVertices();
    float *normals = submesh->getBufferedNormals();
    
    // set the vertex and normal buffers
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glNormalPointer(GL_FLOAT, 0, normals);
    
    // draw the submesh
    glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, faces);
    
    // adjust the vertex and face counter
    m_vertexCount += vertexCount;
    m_faceCount += faceCount;
  }

  // clear vertex array state
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  // clear light
  glDisable(GL_LIGHTING);
  glDisable(GL_LIGHT0);
  glDisable(GL_DEPTH_TEST);

// DEBUG-CODE //////////////////////////////////////////////////////////////////
/*
  // draw the bone lines
  float lines[1024][2][3];
  int nrLines;
  nrLines =  m_calModel.getSkeleton()->getBoneLines(&lines[0][0][0]);
//  nrLines = m_calModel.getSkeleton()->getBoneLinesStatic(&lines[0][0][0]);

  glLineWidth(3.0f);
  glColor3f(1.0f, 1.0f, 1.0f);
  glBegin(GL_LINES);
    int currLine;
    for(currLine = 0; currLine < nrLines; currLine++)
    {
      glVertex3f(lines[currLine][0][0], lines[currLine][0][1], lines[currLine][0][2]);
      glVertex3f(lines[currLine][1][0], lines[currLine][1][1], lines[currLine][1][2]);
    }
  glEnd();
  glLineWidth(1.0f);

  // draw the bone points
  float points[1024][3];
  int nrPoints;
  nrPoints =  m_calModel.getSkeleton()->getBonePoints(&points[0][0]);
//  nrPoints = m_calModel.getSkeleton()->getBonePointsStatic(&points[0][0]);

  glPointSize(4.0f);
  glBegin(GL_POINTS);
    glColor3f(0.0f, 0.0f, 1.0f);
    int currPoint;
    for(currPoint = 0; currPoint < nrPoints; currPoint++)
    {
      glVertex3f(points[currPoint][0], points[currPoint][1], points[currPoint][2]);
    }
  glEnd();
  glPointSize(1.0f);
*/
////////////////////////////////////////////////////////////////////////////////
}

//----------------------------------------------------------------------------//
// Set the dimension                                                          //
//----------------------------------------------------------------------------//

void Viewer::setDimension(int width, int height)
{
  // store new width and height values
  m_width = width;
  m_height = height;

  // set new viewport dimension
  glViewport(0, 0, m_width, m_height);
}

//----------------------------------------------------------------------------//
