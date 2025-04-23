/*
	Name: Chen K
	ID: 208051391
*/

/* Includes */
#include <windows.h>
#include <stdio.h>
#include <glut.h>
#include <stdlib.h>
#include <math.h>
#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"

/* Defines */
#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed
#endif

#define FPS 60
#define WINDOW_WIDTH (640*2)
#define WINDOW_HEIGHT (640*2)
#define TO_RADIANS 3.14/180.0

/* Structs */
typedef struct _POS
{
	float x;
	float y;
	float z;
} POS;

typedef struct _DIRECTION
{
	bool forward;
	bool backwards;
	bool left;
	bool right;
} DIRECTION;

typedef struct _CAMERA
{
	POS position;
	POS lookAt;
} CAMERA;

typedef struct _ROBOT
{
	POS position;
	DIRECTION dir;
	float body_rot_deg;
	float head_rot_deg;
	float head_nod_deg;
	float arm_rot_deg;
	float forearm_rot_deg;
	float wrist_rot_deg;
	bool turning_left;
	bool turning_right;
	bool nod_up;
	bool nod_down;
} ROBOT;

/* Global state */
int windowWidth = WINDOW_WIDTH;
int windowHeight = WINDOW_HEIGHT;

bool third_person = true;
CAMERA third_party_camera = { .position = {.x = 5.0, .y = -5.0, .z = 5.0},
							  .lookAt =   {.x = 0.0, .y =  0.0, .z = 0.0} };

ROBOT robot = { .position = {.x = 0.0, .y = 0.0, .z = 0.0 },
				.dir = {.forward = false, .backwards = false, .left = false, .right = false},
				.body_rot_deg = 0.0, .head_rot_deg = 0.0, .head_nod_deg = 0.0,
				.arm_rot_deg = 0.0, .forearm_rot_deg = 45.0, .wrist_rot_deg = 0,
				.turning_left = false, .turning_right = false, .nod_up = false, .nod_down = false };

GLfloat ambientIntensity = 0.2f;
GLfloat pointLight0Intensity = 0.8f;
GLfloat pointLight0Pos[] = { 0.0f,0.0f,10.0f,1.0f };
bool show_point_light_sphere = true;

/* Functions */

// Initialize GLUT and IMGUI
void init(void)
{
	// GLUT init
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Robot Project");

	glutInitDisplayMode(GLUT_RGBA);
	glClearColor(0.1, 0.1, 0.1, 0);
	glClearDepth(1.0f);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
	glDepthFunc(GL_LEQUAL);    // Set the type of depth-test

	// IMGUI init
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	ImGui::StyleColorsDark();
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 32.0f);
	ImGui_ImplGLUT_Init();
	ImGui_ImplOpenGL2_Init();
}

// Destroy IMGUI
void destroy(void)
{
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGLUT_Shutdown();
	ImGui::DestroyContext();
}

// Limit the possible values of a degree variable to be between 0 and 360
void resetDegrees(float& var)
{
	if (var >= 360.0)
	{
		var -= 360.0;
	}
	if (var < 0.0)
	{
		var += 360.0;
	}
}

// Limit the possible values of a degree variable
void limitDegrees(float& var, float low, float high)
{
	if (var >= high)
	{
		var = high;
	}
	if (var <= low)
	{
		var = low;
	}
}

// This function is for debugging purposes - it shows the x,y,z axis
void drawLines()
{
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	// X
	glColor3f(255, 0, 0);
	glVertex3f(-100, 0, 0);
	glVertex3f(100, 0, 0);
	glEnd();

	glBegin(GL_LINES);
	// Y
	glColor3f(0, 255, 0);
	glVertex3f(0, -100, 0);
	glVertex3f(0, 100, 0);
	glEnd();

	glBegin(GL_LINES);
	// Z
	glColor3f(0, 0, 255);
	glVertex3f(0, 0, -100);
	glVertex3f(0, 0, 100);
	glEnd();
}

// Draw the whole robot
void drawRobot()
{
	glPushMatrix();
	{
		// Body
		float height = 3.0f;
		float width = 1.0f;
		// Start drawing from current robot position
		glTranslatef(robot.position.x, robot.position.y, robot.position.z);
		glRotatef(robot.body_rot_deg, 0, 0, 1); // Rotate the body
		glTranslatef(0.0f, 0.0f, height / 2.0); // Translate above ground
		glPushMatrix();							// To save original scale
		{
			glColor3f(0.5, 0.5, 0.5);
			glScalef(width, width, height);
			glutSolidCube(1.0f);
			glColor3f(0, 0, 0);
			glutWireCube(1.0f);
		}
		glPopMatrix();

		// Head
		float head_to_body_ratio = 1.2f;
		float head_size = width * head_to_body_ratio;
		glPushMatrix();								// To save original scale and middle body location
		{
			glRotatef(robot.head_rot_deg, 0, 0, 1);		// Rotate the head
			glTranslatef(0.0f, 0.0f, height / 2.0);		// Translate to the edge of the body
			glTranslatef(0.0f, 0.0f, head_size / 2.0f); // Translate to the middle of the head
			glRotatef(robot.head_nod_deg, 1, 0, 0);		// Nod the head

			glColor3f(0.1, 0.1, 1);
			glScalef(head_size, head_size, head_size);
			glutSolidCube(1.0f);
			glColor3f(0, 0, 0);
			glutWireCube(1.0f);
		}
		glPopMatrix();

		// Arm
		float body_to_arm_height_ratio = 4.0;
		float body_to_arm_width_ratio = 3.0;
		float arm_height = height / body_to_arm_height_ratio;
		float arm_width = width / body_to_arm_width_ratio;
		glTranslatef(width / 2.0f, 0.0f, 0.0f);		// Translate to the edge of the body
		glTranslatef(arm_width / 2.0f, 0.0f, 0.0f); // Translate to the middle of the arm
		glTranslatef(0.0f, 0.0f, arm_height * 1.2);	// Translate up the body

		// Arm rotation
		glTranslatef(0.0f, 0.0f, arm_height * 0.5);		// Translate to the joint
		glRotatef(robot.arm_rot_deg, 1.0f, 0.0f, 0.0f);	// Rotate the arm
		glTranslatef(0.0f, 0.0f, -arm_height * 0.5);	// Translate back to the middle of the arm

		// Now draw the arm itself
		glPushMatrix();								// To save original scale
		{
			glColor3f(0.7, 0.7, 0.7);
			glScalef(arm_width, arm_width, arm_height);
			glutSolidCube(1.0f);
		}
		glPopMatrix();

		// Forearm
		float arm_to_forearm_height = 1;
		float forearm_height = arm_height / arm_to_forearm_height;
		float forearm_width = arm_width;
		glTranslatef(0.0f, 0.0f, -(arm_height / 2));	// Translate down the lower edge of arm

		// Forearm rotation
		glTranslatef(0.0f, -arm_width * 0.5, 0);		// Translate to the joint
		glRotatef(robot.forearm_rot_deg, 1, 0, 0);
		glTranslatef(0.0f, arm_width * 0.5, 0);			// Translate back to the middle of the arm

		glTranslatef(0.0f, 0.0f, -(forearm_height / 2.0)); // Translate to the middle of the forearm
		glPushMatrix();									   // To save original scale
		{
			glColor3f(0.7, 0.7, 0.7);
			glScalef(forearm_width, forearm_width, forearm_height);
			glutSolidCube(1.0f);
		}
		glPopMatrix();

		// Wrist
		float wrist_width = forearm_width * 1.2;
		float wrist_height = wrist_width;
		glTranslatef(0.0f, 0.0f, -(forearm_height / 2)); // Translate down the lower edge of forearm
		glRotatef(robot.wrist_rot_deg, 0, 0, 1);

		glTranslatef(0.0f, 0.0f, -(wrist_height / 2));   //Translate to the middle of the wrist
		glPushMatrix();									 // To save original scale
		{
			glColor3f(0.7, 0.7, 0.7);
			glScalef(wrist_width, wrist_width, wrist_height);
			glutSolidCube(1.0f);
			glColor3f(0, 0, 0);
			glutWireCube(1.0f);
		}
		glPopMatrix();
	}
	glPopMatrix();
}

void drawObjects()
{
	//Draw Tetrahedron
	glPushMatrix();
	{
		glColor3f(0.5, 0.0, 0.0);
		glTranslatef(5.0f, 0.0f, 0.6f);
		glutSolidTetrahedron();
	}
	glPopMatrix();

	//Draw Cube
	glPushMatrix();
	{
		glColor3f(0.0, 0.5, 0.0);
		glTranslatef(5.0f, 8.0f, 1.0);
		glutSolidCube(2.0f);
	}
	glPopMatrix();

	//Draw metallic Octahedron
	glPushAttrib(GL_LIGHTING_BIT);
	glPushMatrix();
	{
		// Metallic color (it's shiny!)
		glColor3f(0.50754, 0.50754, 0.50754);
		//float metalAmb[4] = { 0.19225f, 0.19225f, 0.19225f, 1.0f };
		float metalAmb[4] = { 0.25, 0.25, 0.25, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, metalAmb);
		//float metalDiff[4] = { 0.50754 , 0.50754 , 0.50754 , 1.0 };
		float metalDiff[4] = { 0.4 , 0.4 , 0.4 , 1.0 };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, metalDiff);
		//float metalSpec[4] = { 0.508273 , 	0.508273 , 	0.508273 , 1.0 };
		float metalSpec[4] = { 0.774597 , 	0.774597 , 	0.774597 , 1.0 };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, metalSpec);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.4 * 128.0);
		glTranslatef(-5.0f, -8.0f, 1.0);
		glutSolidOctahedron();
	}
	glPopMatrix();
	glPopAttrib();
}

// Draw the tiled surface
void drawSurface()
{
	glPushAttrib(GL_LIGHTING_BIT);
	{
		GLfloat white[] = { 0.296648f, 0.296648f, 0.296648f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64.0f);

		for (int i = -25; i < 25; i++) //loop through the height of the map
		{
			for (int j = -25; j < 25; j++) //loop through the width of the map
			{
				glPushMatrix(); //push the matrix so that our translations only affect this tile
				{
					glTranslatef(2 * j, 2 * -i, 0); //translate the tile to where it should belong

					// Color each tile alternating between black & white
					int modi = i % 2;
					int modj = j % 2;
					if (modi == -1)
						modi = 1;
					if (modj == -1)
						modj = 1;
					if ((modi == 0 && modj == 0) ||
						(modi == 1 && modj == 1))
						glColor3f(0.8, 0.8, 0.8);
					else
						glColor3f(0.2, 0.2, 0.2);

					glBegin(GL_QUADS); //begin drawing our quads
					{
						glNormal3f(0.0, 0.0, 1.0);
						glVertex3f(0.0, 0.0, 0.0);
						glVertex3f(2.0, 0.0, 0.0);
						glVertex3f(2.0, 2.0, 0.0);
						glVertex3f(0.0, 2.0, 0.0);
					}
					glEnd();
				}
				glPopMatrix(); //pop the matrix
			}
		}
	}
	glPopAttrib();
}

// Draw the GUI
void drawGUI(void)
{
	ImGui_ImplGLUT_NewFrame();
	ImGui_ImplOpenGL2_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Menu", 0, ImGuiWindowFlags_AlwaysAutoResize);
	if (ImGui::CollapsingHeader("Lights"))
	{
		ImGui::Text("Ambient:");
		ImGui::SetNextItemWidth(300);
		ImGui::SliderFloat("Intensity##Ambient", &ambientIntensity, 0.0f, 1.0f);

		ImGui::Text("Point Light:");
		ImGui::Checkbox("Sphere", &show_point_light_sphere);
		ImGui::SetNextItemWidth(300);
		ImGui::SliderFloat("Intensity##Point", &pointLight0Intensity, 0.0f, 1.0f);

		ImGui::SetNextItemWidth(100);
		ImGui::SliderFloat("X##Point", &pointLight0Pos[0], -10.0f, 10.0f);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100);
		ImGui::SliderFloat("Y##Point", &pointLight0Pos[1], -10.0f, 10.0f);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100);
		ImGui::SliderFloat("Z##Point", &pointLight0Pos[2], 1.0f, 20.0f);
	}

	if (ImGui::CollapsingHeader("Robot"))
	{
		ImGui::SetNextItemWidth(100);
		ImGui::SliderFloat("Body Rotation", &robot.body_rot_deg, 0.0f, 359.0f);
		ImGui::SetNextItemWidth(100);
		ImGui::SliderFloat("Head Rotation", &robot.head_rot_deg, 0.0f, 359.0f);
		ImGui::SetNextItemWidth(100);
		ImGui::SliderFloat("Head Nod", &robot.head_nod_deg, -60.0f, 60.f);

		ImGui::SetNextItemWidth(100);
		ImGui::SliderFloat("Arm Rotation", &robot.arm_rot_deg, 0.0f, 359.0f);
		ImGui::SetNextItemWidth(100);
		ImGui::SliderFloat("Forearm Rotation", &robot.forearm_rot_deg, 0.0f, 90.0f);
		ImGui::SetNextItemWidth(100);
		ImGui::SliderFloat("Wrist Rotation", &robot.wrist_rot_deg, 0.0f, 359.0f);
	}

	if (ImGui::CollapsingHeader("Camera"))
	{
		ImGui::Checkbox("Third Person", &third_person);
		if (third_person)
		{
			ImGui::SetNextItemWidth(100);
			ImGui::SliderFloat("X##Camera", &third_party_camera.position.x, -25.0f, 25.0f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100);
			ImGui::SliderFloat("Y##Camera", &third_party_camera.position.y, -25.0f, 25.0f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100);
			ImGui::SliderFloat("Z##Camera", &third_party_camera.position.z, 1.0f, 40.0f);

			ImGui::SetNextItemWidth(100);
			ImGui::SliderFloat("X##CameraLook", &third_party_camera.lookAt.x, -25.0f, 25.0f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100);
			ImGui::SliderFloat("Y##CameraLook", &third_party_camera.lookAt.y, -25.0f, 25.0f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100);
			ImGui::SliderFloat("Z##CameraLook", &third_party_camera.lookAt.z, 1.0f, 40.0f);

		}
	}

	if (ImGui::Button("Quit"))
		exit(0);
	ImGui::SameLine();

	static bool help = false;
	if (ImGui::Button("Help"))
	{
		if (help)
			help = false;
		else
			help = true;
	}
	
	if (help)
	{
		ImGui::Begin("Help", &help, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Keyboard:");
		ImGui::Text("w s  a d  : move the robot position");
		ImGui::Text("left right: rotate the robot's body");
		ImGui::Text("up   down : nod the robot's head");
		ImGui::Text("Space: toggle POV - 3rd person <--> 1st person");
		ImGui::Text("");
		ImGui::Text("Mouse:");
		ImGui::Text("First person: move the head");
		ImGui::Text("Third person: use the controls menu");
		ImGui::End();
	}
	
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

// Draw the lights - amnbient and positioned light
void drawLights(void)
{
	// Ambient light
	GLfloat ambientColor[] = { ambientIntensity,ambientIntensity,ambientIntensity ,1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

	// Positioned light
	GLfloat lightColor0[] = { pointLight0Intensity, pointLight0Intensity, pointLight0Intensity,1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, pointLight0Pos);

	if (show_point_light_sphere)
	{
		// Draw Sphere representing the point light
		glPushAttrib(GL_LIGHTING_BIT);
		glPushMatrix();
		{
			GLfloat sphere_emission[] = { pointLight0Intensity, pointLight0Intensity, pointLight0Intensity, 1.0f };
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, sphere_emission);
			glColor3f(pointLight0Intensity, pointLight0Intensity, pointLight0Intensity);
			glTranslatef(pointLight0Pos[0], pointLight0Pos[1], pointLight0Pos[2]);
			glutSolidSphere(0.5, 8, 8);
		}
		glPopMatrix();
		glPopAttrib();
	}
}

// Adjust robot position according to user control
void adjustRobotPosition(void)
{
	float yaw = robot.body_rot_deg;
	if (!third_person)
	{
		// In first person, the moving direction is body + head rotation
		yaw += robot.head_rot_deg;
	}

	if (robot.dir.forward)
	{
		robot.position.x += cos((yaw + 90.0) * TO_RADIANS) / 5.0;
		robot.position.y += sin((yaw + 90) * TO_RADIANS) / 5.0;
	}
	if (robot.dir.backwards)
	{
		robot.position.x += cos((yaw + 90.0 + 180.0) * TO_RADIANS) / 5.0;
		robot.position.y += sin((yaw + 90.0 + 180.0) * TO_RADIANS) / 5.0;
	}
	if (robot.dir.left)
	{
		robot.position.x += cos((yaw + 90.0 + 90.0) * TO_RADIANS) / 5.0;
		robot.position.y += sin((yaw + 90.0 + 90.0) * TO_RADIANS) / 5.0;
	}
	if (robot.dir.right)
	{
		robot.position.x += cos((yaw + 90.0 - 90.0) * TO_RADIANS) / 5.0;
		robot.position.y += sin((yaw + 90.0 - 90.0) * TO_RADIANS) / 5.0;
	}
}

// Create a first person view
void first_camera()
{
	float yaw = robot.body_rot_deg + robot.head_rot_deg;

	limitDegrees(robot.head_nod_deg, -60.0, 60.0);			// Limit the head nodding
	glRotatef(-robot.head_nod_deg - 90.0, 1.0, 0.0, 0.0);	// Along X axis
	glRotatef(-yaw, 0.0, 0.0, 1.0);							// Along Z axis

	// Move the camera to the middle of the robot head position
	//
	//													robot head height
	glTranslatef(-robot.position.x, -robot.position.y, -(3.0 + 1.2 / 2.0));
}

// GLUT display callback
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();                // Reset the model-view matrix

	if (third_person)
	{
		// In third person, the cursor is controllable to allow modifing settings via the GUI - show it
		glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		gluLookAt(third_party_camera.position.x, third_party_camera.position.y, third_party_camera.position.z,
			third_party_camera.lookAt.x, third_party_camera.lookAt.y, third_party_camera.lookAt.z,
			0.0, 0.0, 1.0);
		drawRobot();
	}
	else
	{
		// We take control of the mouse and set it to the middle of the screen, so we hide it
		glutSetCursor(GLUT_CURSOR_NONE);
		// Modify the camera to first person view
		first_camera();
		// Do not draw the robot so it won't cover the camera
	}

	//drawLines();
	drawLights();
	drawSurface();
	drawObjects();
	drawGUI();

	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}

// GLUT reshape callback - keep screen not stretched when size is modified
void reshape(GLsizei width, GLsizei height) {
	// Call IMGUI callback first
	ImGui_ImplGLUT_ReshapeFunc(width, height);

   // Compute aspect ratio of the new window
	if (height == 0) height = 1;                // To prevent divide by 0
	GLfloat aspect = (GLfloat)width / (GLfloat)height;

	// Set the viewport to cover the new window
	glViewport(0, 0, width, height);
	windowWidth = width;
	windowHeight = height;

	// Set the aspect ratio of the clipping volume to match the viewport
	glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
	glLoadIdentity();             // Reset
	// Enable perspective projection with fovy, aspect, zNear and zFar
	gluPerspective(90.0f, aspect, 0.1f, 100.0f);
}

// GLUT mouse movement callback - in first person, controls the head and movement direction
void passiveMotion(int x, int y)
{
	printf("Passive Motion X: %d Y: %d\n", x, y);
	if (third_person)
		return;

	int dev_x, dev_y;

	// Calcaulte distance from middle of the screen
	// Longer distance == more movement
	dev_x = (windowWidth / 2) - x;
	dev_y = (windowHeight / 2) - y;

	// Apply the changes to pitch and yaw
#define mouse_speed (10.0)
	robot.head_rot_deg += (float)(dev_x) / mouse_speed;
	robot.head_nod_deg += (float)(dev_y) / mouse_speed;

	// Reset the rotation to be between 0 and 360
	resetDegrees(robot.head_rot_deg);
}

// GLUT keyboard callback - used for robot movement and switching cameras
void keyboard(unsigned char key, int x, int y)
{
	// Call IMGUI callback first
	ImGui_ImplGLUT_KeyboardFunc(key, x, y);

	// If IMGUI wants to control the keyboard - let it
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard)
	{
		return;
	}

	// Detect robot movement and camera control keys
	switch (key)
	{
	case 'w':
	case 'W':
		robot.dir.forward = true;
		break;
	case 'a':
	case 'A':
		robot.dir.left = true;
		break;
	case 's':
	case 'S':
		robot.dir.backwards = true;
		break;
	case 'd':
	case 'D':
		robot.dir.right = true;
		break;
	case ' ':
		third_person = !third_person;
		break;
	}
}

// GLUT keyboardup callback - used for robot movement
void keyboardUp(unsigned char key, int x, int y)
{
	// Call IMGUI callback first
	ImGui_ImplGLUT_KeyboardUpFunc(key, x, y);

	// If IMGUI wants to control the keyboard - let it
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard)
	{
		return;
	}

	// Detect robot movement and camera control keys up
	switch (key)
	{
	case 'w':
	case 'W':
		robot.dir.forward = false;
		break;
	case 'a':
	case 'A':
		robot.dir.left = false;
		break;
	case 's':
	case 'S':
		robot.dir.backwards = false;
		break;
	case 'd':
	case 'D':
		robot.dir.right = false;
		break;
	}
}

// GLUT keyboardSpecial callback - Used for arrow keys which control robot body and head rotation degress
void keyboardSpecial(int key, int x, int y)
{
	ImGui_ImplGLUT_SpecialFunc(key, x, y);

	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard)
	{
		return;
	}

	switch (key)
	{
	case GLUT_KEY_LEFT:
		robot.turning_left = true;
		break;
	case GLUT_KEY_RIGHT:
		robot.turning_right = true;
		break;
	case GLUT_KEY_UP:
		robot.nod_up = true;
		break;
	case GLUT_KEY_DOWN:
		robot.nod_down = true;
		break;
	}
}

// GLUT keyboardSpecialUp callback - Used for arrow keys which control robot body and head rotation degress
void keyboardSpecialUp(int key, int x, int y)
{
	ImGui_ImplGLUT_SpecialUpFunc(key, x, y);

	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard)
	{
		return;
	}

	switch (key)
	{
	case GLUT_KEY_LEFT:
		robot.turning_left = false;
		break;
	case GLUT_KEY_RIGHT:
		robot.turning_right = false;
		break;
	case GLUT_KEY_UP:
		robot.nod_up = false;
		break;
	case GLUT_KEY_DOWN:
		robot.nod_down = false;
		break;
	}
}

// GLUT timer callback - Used for rotating the robot, warping the mouse and setting the FPS to 60
void timer(int)
{
	// Rotate the body
	float rotation_speed = 2.0f;
	if (robot.turning_left)
	{
		robot.body_rot_deg += rotation_speed;
	}
	if (robot.turning_right)
	{
		robot.body_rot_deg -= rotation_speed;
	}
	resetDegrees(robot.body_rot_deg);

	// Nod the head
	if (robot.nod_up)
	{
		robot.head_nod_deg += 1.0;
	}
	if (robot.nod_down)
	{
		robot.head_nod_deg -= 1.0;
	}
	limitDegrees(robot.head_nod_deg, -60.0, 60.0);

	adjustRobotPosition();
	glutPostRedisplay();
	
	// If first person
	if (!third_person)
	{
		// Center the curser
		glutWarpPointer(windowWidth / 2, windowHeight / 2);
	}

	// Target 60 FPS
	glutTimerFunc(1000 / FPS, timer, 0);
}

// Register all our GLUT callbacks
void registerCallbacks(void)
{
	glutReshapeFunc(reshape);
	glutMotionFunc(ImGui_ImplGLUT_MotionFunc);
	glutMouseFunc(ImGui_ImplGLUT_MouseFunc);
	glutPassiveMotionFunc(passiveMotion);
	glutTimerFunc(0, timer, 0);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(keyboardSpecial);
	glutSpecialUpFunc(keyboardSpecialUp);
	glutDisplayFunc(display);
}

// The program entrypoint
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	init();

	registerCallbacks();
	glutMainLoop();

	destroy();
	return 0;
}
