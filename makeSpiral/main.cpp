#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>

#include <fstream>	
using namespace std;

//buttons
// x y z
// q w e
//rotate

// THETA:   0,1
// U:		2,3
// alfa:	4,5

GLfloat perspectiveNumber = 10;
GLfloat pNumberShange = 0.1;
GLsizei winWidth = 800, winHeight = 600;

vec3 spiral[20][20] = {};

vec3 spiralFaces[19 * 19 * 4];
vec3 orderedFaces[19 * 19 * 4];
vec3 finalSpiral[19 * 19 * 4];

mat4 w2v,scaled, projection[2], translation[2];
bool rotateAtX = false, rotateAtY = false, rotateAtZ = false;

int piSzorzo = 2;
float pi_180 = piSzorzo * (pi() / 180);

float alphaX = pi()/2;
float alphaY = pi()/3;
float alphaZ = 0;

GLint keyStates[256];

int count1 = 0;
int count2 = 0;

float incTetha = 0.1;
float incU = 0.1;
float changeIncVals = 0.01;
float alfa = 1;


void createSpiral() {

	count1 = 0;
	count2 = 0;

	for (float theta = -2 * pi(); theta < 2 * pi(); theta += incTetha) {
		count1++;	
		count2 = 0;
		for (float u = -2 * pi(); u < 2 * pi(); u += incU) {

			if (count2 < 20 && count1 < 20) {
				spiral[count1][count2] = { float(u*cos(alfa*theta)) , float(u*sin(alfa*theta)) , theta };
				count2++;
			}
			else {
				break;
			}

		}
	}
}


void initMatrices(){
    
	scaled = scale(vec3(1.0, 1.0, 1.0));

	vec2 windowSize = { 4*pi(), 4*pi() };
	vec2 windowPosition = { -2*pi(), -2*pi() };

    vec2 viewportSize = { 450, 450 };
    vec2 viewportPosition = { winWidth / 2 - viewportSize.x / 2, winHeight / 2 - viewportSize.y / 2 };
    
	w2v = windowToViewport3(windowPosition, windowSize, viewportPosition, viewportSize);
    
	projection[0] = perspective(perspectiveNumber);
    projection[1] = ortho();
    
}

void init() {
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, winWidth, 0.0, winHeight);
    glShadeModel(GL_FLAT);
    glEnable(GL_POINT_SMOOTH);
    glPointSize(5.0);
    glLineWidth(1.0);
    
    initMatrices();
}

void drawFace(vec3 r[]) {

	for (int k = 0; k < 19 * 19 *4; k += 4) {

		glColor3f(1, 0, 0);
		glBegin(GL_POLYGON);
	
		for (int i = k; i<k+4; i++)
			glVertex2f(r[i].x, r[i].y);

		glEnd();

		glColor3f(0, 0, 0);
		glBegin(GL_LINE_LOOP);

		for (int i = k; i<k+4; i++)
			glVertex2f(r[i].x, r[i].y);

		glEnd();
	}

}

void calcFaces() {

		createSpiral();
		glColor3f(0, 0, 0);

		int countFaces = 0;

		for (int idx = 1; idx < 19; idx++) {
			for (int idx2 = 0; idx2 < 19; idx2++) {

				int countFacepoint = 0;
				vec3 tempFacePoints[4];

				for(int i = idx; i<=idx+1;i++)
					for (int j = idx2; j <= idx2 + 1; j++) {
					
						vec3 spiralPoint = spiral[i][j];

					
						tempFacePoints[countFacepoint] = { spiralPoint.x,spiralPoint.y,spiralPoint.z };
						countFacepoint++;

					}

				vec3 temp = tempFacePoints[3];
				tempFacePoints[3] = tempFacePoints[2];
				tempFacePoints[2] = temp;

				for (int i = 0; i < 4; i++) {
					spiralFaces[countFaces] = { tempFacePoints[i].x, tempFacePoints[i].y, tempFacePoints[i].z };
					countFaces++;
				}

			}
		}

		int maxSpiralPoint = 19 * 19 * 4 - (19 * 4);

		for (int i = 0; i < maxSpiralPoint; i++) {
		
			vec3 spiralPoint = spiralFaces[i];

			mat4 M = rotateX(alphaX)* rotateY(alphaY) * rotateZ(alphaZ);

			vec4 pointH = ihToH(spiralPoint);
			vec4 transformedPoint = M * pointH;
			
			if (transformedPoint.w != 0) {
				vec3 result = hToIh(transformedPoint);

				orderedFaces[i] = { result.x, result.y, result.z };
				
			}
		}


		bool swapped = false;
		do {
			swapped = false;
			for (int i = 0; i < maxSpiralPoint - 4; i += 4) {
				float avgZ1 = 0;
				for (int j = i; j < i + 4; j++) {
					avgZ1 += orderedFaces[j].z;
				}
				avgZ1 /= 4;

				float avgZ2 = 0;
				for (int j = i + 4; j < i + 8; j++) {
					avgZ2 += orderedFaces[j].z;
				}
				avgZ2 /= 4;

				if (avgZ1 > avgZ2) {

					vec3 temp[4];
					int cTemp = 0;
					for (int j = i; j < i + 4; j++) {
						temp[cTemp] = orderedFaces[j];
						cTemp++;
					}
					for (int j = i; j < i + 4; j++) {
						orderedFaces[j] = orderedFaces[j + 4];
					}
					cTemp = 0;
					for (int j = i + 4; j < i + 8; j++) {
						orderedFaces[j] = temp[cTemp];
						cTemp++;
					}
					swapped = true;

				}
			}
		} while (swapped);

		//w2v and projection
		for (int i = 0; i < maxSpiralPoint; i++) {

			vec3 spiralPoint = orderedFaces[i];

			mat4 M = w2v * projection[1] * scaled ;
			
			vec4 pointH = ihToH(spiralPoint);
			vec4 transformedPoint = M * pointH;

			if (transformedPoint.w != 0) {
				vec3 result = hToIh(transformedPoint);

				if (result.z == 0) {

					finalSpiral[i] = { result.x,result.y,result.z };
				}
			}
		}

}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    glColor3f(0, 0, 0);
	calcFaces();
	drawFace(finalSpiral);

    glutSwapBuffers();
}

void update(int value)
{
    
	if (rotateAtX)
		alphaX += pi_180;
	if (rotateAtY)
		alphaY += pi_180;
	if (rotateAtZ)
		alphaZ += pi_180;

	if (alphaX >= 2 * pi()) {
		alphaX = 0;
	}
	if (alphaY >= 2 * pi()) {
		alphaY = 0;
	}
	if (alphaZ >= 2 * pi()) {
		alphaZ = 0;
	}

    glutPostRedisplay();
    
    glutTimerFunc(10, update, 0);
}


void keyPressed(unsigned char key, int x, int y)
{
	keyStates[key] = 1;
}

void keyUp(unsigned char key, int x, int y)
{
	
	if (keyStates['x'] || keyStates['q']) {
		rotateAtX = false;
	}
	if (keyStates['y'] || keyStates['w']) {
		rotateAtY = false;
	}
	if (keyStates['z'] || keyStates['e']) {
		rotateAtZ = false;
	}

	if (keyStates['0']) {
		if(incTetha > 0.1)
			incTetha -= changeIncVals;
		printf("incTetha: %g", incTetha);
	}
	if (keyStates['1']) {
		incTetha += changeIncVals;
	}

	if (keyStates['2']) {

		if (incU > 0.1)
			incU -= changeIncVals;
	}
	if (keyStates['3']) {
		incU += changeIncVals;
	}

	if (keyStates['4']) {

		if (alfa > 0.1)
			alfa -= changeIncVals;
	}
	if (keyStates['5']) {
		alfa += changeIncVals;
	}

	keyStates[key] = 0;
}

void keyOperations(int value)
{
	if (keyStates['x'] || keyStates['q']) {
		rotateAtX = true;
	}
	if (keyStates['y'] || keyStates['w']) {
		rotateAtY = true;
	}
	if (keyStates['z'] || keyStates['e']) {
		rotateAtZ = true;
	}

	glutPostRedisplay();

	glutTimerFunc(10, keyOperations, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(winWidth, winHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("spiral");
    
    init();
    glutDisplayFunc(display);

    glutTimerFunc(10, update, 0);
    
	glutTimerFunc(0, keyOperations, 0);
	glutKeyboardFunc(keyPressed);
	glutKeyboardUpFunc(keyUp);

    glutMainLoop();
    return 0;
}




