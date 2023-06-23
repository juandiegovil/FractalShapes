#define _USE_MATH_DEFINES
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

#include <iostream>
#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"

struct State {
	int iterations = 0;
	int scene = 0;
	bool operator == (State const& other) const {
		return iterations == other.iterations && scene == other.scene;
	}
};

// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader) : shader(shader) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			if (key == GLFW_KEY_R) {
				shader.recompile();
			}
			if (key == GLFW_KEY_0) {
				state.scene = 0;
			}
			if (key == GLFW_KEY_1) {
				state.scene = 1;
			}
			if (key == GLFW_KEY_2) {
				state.scene = 2;
			}
			if (key == GLFW_KEY_3) {
				state.scene = 3;
			}
			if (key == GLFW_KEY_4) {
				state.scene = 4;
			}
			if (key == GLFW_KEY_LEFT) {
				if (state.iterations > 0) {
					state.iterations--;
				}
			}
			if (key == GLFW_KEY_RIGHT) {
				if (state.iterations < 20) {
					state.iterations++;
				}
			}
		}
	}
	State getState() {
		return state;
	}

private:
	ShaderProgram& shader;
	State state;
};

// END EXAMPLES

void generateTriangle(CPU_Geometry &cpuGeom, float size, int scene) {
	// vertices
	float XBottom = size;
	float YBottom = XBottom * tan(M_PI / float(6));
	float yTop = XBottom*sqrtf(3);
	float upDown = (XBottom + yTop) / float(2);
	cpuGeom.verts.push_back(glm::vec3(-XBottom, -YBottom, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.f, yTop - YBottom, 0.f));
	cpuGeom.verts.push_back(glm::vec3(XBottom, -YBottom, 0.f));
	//Log::debug("cpuGeom.verts.size(): {}", cpuGeom.verts.size());

	float fl = sqrtf(pow((cpuGeom.verts[1][0] - cpuGeom.verts[0][0]), 2) + pow(cpuGeom.verts[1][1] - cpuGeom.verts[0][1], 2));
	float sl = sqrtf(pow((cpuGeom.verts[2][0] - cpuGeom.verts[1][0]), 2) + pow(cpuGeom.verts[2][1] - cpuGeom.verts[1][1], 2));
	float tl = sqrtf(pow((cpuGeom.verts[0][0] - cpuGeom.verts[2][0]), 2) + pow(cpuGeom.verts[0][1] - cpuGeom.verts[2][1], 2));


	// colours (these should be in linear space)
	if (scene == 0) {
		cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
		cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
		cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
	}
	else {
		cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
		cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
		cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));

	}
}

void generateTriangleIteration(glm::vec3 leftIn, glm::vec3 middleIn, glm::vec3 rightIn, CPU_Geometry& cpuGeomI, int iter, int count) {
	glm::vec3 left(0.5f * (leftIn[0]+middleIn[0]), 0.5f * (middleIn[1] + leftIn[1]), 0.f);
	glm::vec3 middle(middleIn[0], rightIn[1], 0.f);
	glm::vec3 right(0.5f*(rightIn[0]+middleIn[0]), 0.5f * (middleIn[1] + rightIn[1]), 0.f);
	cpuGeomI.verts.push_back(glm::vec3(left));
	cpuGeomI.verts.push_back(glm::vec3(middle));
	cpuGeomI.verts.push_back(glm::vec3(right));
	cpuGeomI.cols.push_back(glm::vec3(0.f, 0.f, 0.f));
	cpuGeomI.cols.push_back(glm::vec3(0.f, 0.f, 0.f));
	cpuGeomI.cols.push_back(glm::vec3(0.f, 0.f, 0.f));
	count++;

	if (count < iter) {
		int size = GLsizei(cpuGeomI.verts.size());

		float d = (cpuGeomI.verts[size - 1][0] - cpuGeomI.verts[size - 3][0]);
		if (d == 0) {
			d = 2.0f * cpuGeomI.verts[size - 1][0];
		}

		glm::vec3 left(cpuGeomI.verts[size - 2][0] - d, cpuGeomI.verts[size - 2][1], 0.f);
		generateTriangleIteration(left, cpuGeomI.verts[size - 3], cpuGeomI.verts[size -2], cpuGeomI, iter, count);

		glm::vec3 middle(cpuGeomI.verts[size - 3][0] + 0.5f*d, cpuGeomI.verts[size - 3][1] + 0.5*d*sqrtf(3), 0.f);
		generateTriangleIteration(cpuGeomI.verts[size - 3], middle, cpuGeomI.verts[size - 1], cpuGeomI, iter, count);

		glm::vec3 right(cpuGeomI.verts[size - 2][0] + d, cpuGeomI.verts[size - 2][1], 0.f);
		generateTriangleIteration(cpuGeomI.verts[size - 2], cpuGeomI.verts[size -1], right, cpuGeomI, iter, count);
	}
}

glm::vec3 triangleMidPoint(glm::vec3 first, glm::vec3 second, glm::vec3 third) {
	float x = float(1) / float(3) * first[0] + float(1) / float(3) * second[0] + float(1) / float(3) * third[0];
	float y = float(1) / float(3) * first[1] + float(1) / float(3) * second[1] + float(1) / float(3) * third[1];
	return glm::vec3(x, y, 0.f);
}

void generateInnerTriangles(CPU_Geometry& cpuGeom, glm::vec3 first, glm::vec3 second, glm::vec3 third, int iter, int count) {
	glm::vec3 Midpoint = triangleMidPoint(first, second, third);
	cpuGeom.verts.push_back(first);
	//cpuGeom.verts.push_back(second);
	cpuGeom.verts.push_back(Midpoint);
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	//cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	cpuGeom.verts.push_back(second);
	//cpuGeom.verts.push_back(third);
	cpuGeom.verts.push_back(Midpoint);
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	//cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	cpuGeom.verts.push_back(third);
	//cpuGeom.verts.push_back(first);
	cpuGeom.verts.push_back(Midpoint);
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	//cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	count++;
	if (count - 1 < iter) {
		int size = GLsizei(cpuGeom.verts.size());
		generateInnerTriangles(cpuGeom, cpuGeom.verts[size - 6], cpuGeom.verts[size - 4], cpuGeom.verts[size - 5], iter, count);
		generateInnerTriangles(cpuGeom, cpuGeom.verts[size - 4], cpuGeom.verts[size - 2], cpuGeom.verts[size - 3], iter, count);
		generateInnerTriangles(cpuGeom, cpuGeom.verts[size - 2], cpuGeom.verts[size - 1], cpuGeom.verts[size - 6], iter, count);
	}
}

glm::vec3 segmentThirdPoint(glm::vec3 first, glm::vec3 second) {
	float x = float(2) / float(3) * first[0] + float(1) / float(3) * second[0];
	float y = float(2) / float(3) * first[1] + float(1) / float(3) * second[1];
	//float d = sqrt(pow((x - first[0]), 2) + pow((y - first[1]), 2));
	return glm::vec3(x, y, 0.f);
}

glm::vec3 segmentMidPoint(glm::vec3 first, glm::vec3 second) {
	float x = float(1) / float(2) * first[0] + float(1) / float(2) * second[0];
	float y = float(1) / float(2) * first[1] + float(1) / float(2) * second[1];
	return glm::vec3(x, y, 0.f);
}

glm::vec3 getLengthMidPoint(glm::vec3 mid, glm::vec3 first, glm::vec3 second) {
	float l = sqrt(pow((second[0] - first[0]), 2) + pow(second[1]-first[1], 2));
	float midl = sqrt(pow((mid[0]), 2) + pow(mid[1], 2));
	l = l / float(2) * sqrt(3);
	float addedl = l / midl + 1;
	mid[0] = addedl * mid[0];
	mid[1] = addedl * mid[1];
	return mid;
}

void generateSnowflake(CPU_Geometry& cpuGeom, int iter) {
	glm::vec3 hold = cpuGeom.verts[1];
	cpuGeom.verts[1] = cpuGeom.verts[2];
	cpuGeom.verts[2] = hold;

	for (int j = 0; j < iter; j++) {
		int size = GLsizei(cpuGeom.verts.size());
		int i = 0;
		while (i < (size * 3 + size - 3)) {

			glm::vec3 second;
			glm::vec3 third;
			int z = 0;
			if (i == 0) {
				second = cpuGeom.verts[i + 1];
				third = cpuGeom.verts[size - 1];
				z++;
			}
			else if (i + 1 == (size * 3 + size - 3)) {
				second = cpuGeom.verts[0];
				third = cpuGeom.verts[4];
				z++;
			}
			else if (j == 0 && i == 4) {
				second = cpuGeom.verts[i + 1];
				third = cpuGeom.verts[0];
				z++;
			}
			else {
				second = cpuGeom.verts[i + 1];
				third = cpuGeom.verts[i + 2];
			}

			glm::vec3 triangleMidpoint = triangleMidPoint(cpuGeom.verts[i], second, third);
			while (z < 2) {
				int k = i + 1;
				if (k == (size * 3 + size - 3)) {
					k = 0;
				}

				glm::vec3 midpoint = segmentMidPoint(cpuGeom.verts[i], cpuGeom.verts[k]);
				glm::vec3 r = glm::vec3(midpoint[0] - triangleMidpoint[0], midpoint[1] - triangleMidpoint[1], 0.f);
				glm::vec3 next = segmentThirdPoint(cpuGeom.verts[i], cpuGeom.verts[k]);
				glm::vec3 after = segmentThirdPoint(cpuGeom.verts[k], cpuGeom.verts[i]);
				midpoint = getLengthMidPoint(r, cpuGeom.verts[i], next);
				midpoint = glm::vec3(midpoint[0] + triangleMidpoint[0], midpoint[1] + triangleMidpoint[1], 0.f);

				if (k > 0) {
					cpuGeom.verts.insert(cpuGeom.verts.begin() + i + 1, next);
					cpuGeom.verts.insert(cpuGeom.verts.begin() + i + 2, midpoint);
					cpuGeom.verts.insert(cpuGeom.verts.begin() + i + 3, after);
				}
				else {
					cpuGeom.verts.push_back(next);
					cpuGeom.verts.push_back(midpoint);
					cpuGeom.verts.push_back(after);
				}
				cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
				cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
				cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
				i = i + 4;
				z++;
			}

		}
	}
	
}

glm::vec3 segmentAlternateMidPoint(glm::vec3 first, glm::vec3 second) {
	float xd = second[0] - first[0];
	if (xd == 0) {
		xd = 2.0f * first[0];
		float ratiox = (xd / sqrtf(2)) / xd;
	}
	float yd = second[1] - first[1];
	if (yd == 0) {
		yd = 2.0f * first[1];
	}
	float ratiox = (xd / sqrtf(2)) / xd;
	float ratioy = (yd / sqrtf(2)) / yd;
	float x = (1 - ratiox) * first[0] + ratiox * second[0];
	float y = (1 - ratiox) * first[1] + ratioy * second[1];
	if (second[0] == first[0]) {
		x = second[0];
	}
	if (second[1] == first[1]) {
		y = second[1];
	}
	return glm::vec3(x, y, 0.f);
}

glm::vec3 rotatePoint(glm::vec3 center, glm::vec3 point, float sign) {
	float theta = sign * M_PI / float(4);
	float centerx = center[0];
	float centery = center[1];
	float rotpointx = point[0];
	float rotpointy = point[1];
	float x = (centerx + ((rotpointx - centerx) * cos(theta)) - ((rotpointy - centery) * sin(theta)));
	float y = (centery + ((rotpointx - centerx) * sin(theta)) + ((rotpointy - centery) * cos(theta)));

	//x = x / sqrt(pow((x - rotpointx), 2) + pow(y, 2));
	//y = y / sqrt(pow((x - rotpointy), 2) + pow(y, 2));
	return glm::vec3(x, y, 0.f);
}

void generateDragonCurve(CPU_Geometry& cpuGeom, int iter) {
	cpuGeom.verts.push_back(glm::vec3(-0.5f, 0.f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.5f, 0.f, 0.f));
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	std::vector<int> colorSet = {2};

	int colors = 1;
	for (int i = 0; i < iter; i++) {
		int size = GLsizei(cpuGeom.verts.size());
		float theta = 1.f;
		bool firstRun = true;
		for (int j = 0; j < (size + 1 * (size - 1) - 1); j = j + 2) {
			//std::cout << "Our result: " << cpuGeom.verts[j] << std::endl;
			glm::vec3 midpoint = segmentAlternateMidPoint(cpuGeom.verts[j], cpuGeom.verts[j + 1]);
			//std::cout << "Our result: " << midpoint << std::endl;
			if (theta == -1.f) {
				theta = 1.f;
				firstRun == false;
			}
			else {
				theta = -1.f;
			}

			glm::vec3 newPoint = rotatePoint(cpuGeom.verts[j], midpoint, theta);
			//std::cout << "Our result: " << newPoint << std::endl;
		
			cpuGeom.verts.insert(cpuGeom.verts.begin() + j + 1, newPoint);
			//cpuGeom.verts.insert(cpuGeom.verts.begin() + j + 2, newPoint);
		}
		if (colors == 0) {
			for (int c = 0; c < size - 1; c++) {
				cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
			}
			//std::cout << "Our result: " << colors << std::endl;
			//	cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
			colors++;
		}
		else if (colors == 1) {
			for (int c = 0; c < size - 1; c++) {
				cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
			}
			//std::cout << "Our result: " << colors << std::endl;
			//	cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
			colors++;
		}
		else {
			for (int c = 0; c < size - 1; c++) {
				cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
			}
			//	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
			colors = 0;
		}
		colorSet.push_back(GLsizei(cpuGeom.verts.size()));
	}

	colors == 0;
	for (int i = 0; i < iter; i++) {
		//std::cout << "Our result: " << colorSet.at(i) << std::endl;
		cpuGeom.verts.insert(cpuGeom.verts.begin() + colorSet.at(i) + i, cpuGeom.verts[colorSet.at(i) - 1 + i]);
		//std::cout << "Our result: " << colorSet.at(i) << std::endl;
		cpuGeom.cols.insert(cpuGeom.cols.begin() + colorSet.at(i) + i, cpuGeom.cols[colorSet.at(i) + i]);
	}
}

void generateHilbertCurve(CPU_Geometry& cpuGeom, int iter) {
	float startingCoordinate = 0.5f;
	float startingLength = 2.0f;
	float length = 1.0f;
	cpuGeom.verts.push_back(glm::vec3(startingCoordinate, -startingCoordinate, 0.f));
	cpuGeom.verts.push_back(glm::vec3(cpuGeom.verts[0][0], cpuGeom.verts[0][1] + length, 0.f));
	cpuGeom.verts.push_back(glm::vec3(cpuGeom.verts[1][0] - length, cpuGeom.verts[1][1], 0.f));
	cpuGeom.verts.push_back(glm::vec3(cpuGeom.verts[2][0], cpuGeom.verts[2][1] - length, 0.f));
	//cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	//cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	//cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	//cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	std::vector<int> pattern = {0, 1, 2};
	//std::vector<int> Newpattern = {};
	for (int j = 0; j < iter; j++) {
		int size = GLsizei(cpuGeom.verts.size());
		int k = 0;
		int repeat = 0;
		float coordinate = 1 - (startingCoordinate / float(pow(2, iter)));
		length = (startingLength - (2 * (1 - coordinate))) / float(2*pow(2,iter) - 1);
		cpuGeom.verts[0] = glm::vec3(coordinate, -coordinate, 0.f);
		std::vector<int> newPattern(pattern.size() * 4 + 3, 0);
		//std::cout << "Our result: " << pattern.size() << std::endl;
		//std::cout << "Our result: " << newPattern.size() << std::endl;
		for (int i = 1; i < (pattern.size() * 4 + 4); i++) {
			glm::vec3 next;
			if (repeat == 0) {
				if (pattern.at(k) == 0) {
					newPattern.at(i - 1) = 1;
				}
				else if (pattern.at(k) == 1) {
					newPattern.at(i - 1) = 0;
				}
				else if (pattern.at(k) == 2) {
					newPattern.at(i - 1) = 3;
				}
				else {
					newPattern.at(i - 1) = 2;
				}
				//std::cout << "Our result: " << repeat << std::endl;
			}
			else if (repeat < 3) {
				//std::cout << "Our result: " << pattern.at(k) << std::endl;
				newPattern.at(i - 1) = pattern.at(k);
			}
			else {
				//std::cout << "Our result: " << repeat << std::endl;
				if (pattern.at(k) == 0) {
					newPattern.at(i - 1) = 3;
				}
				else if (pattern.at(k) == 1) {
					newPattern.at(i - 1) = 2;
				}
				else if (pattern.at(k) == 2) {
					newPattern.at(i - 1) = 1;
				}
				else {
					newPattern.at(i - 1) = 0;
				}
			}

			if (newPattern.at(i - 1) == 0) {
				next = glm::vec3(cpuGeom.verts[i - 1][0], cpuGeom.verts[i - 1][1] + length, 0.f);
			}
			else if (newPattern.at(i - 1) == 1) {
				next = glm::vec3(cpuGeom.verts[i - 1][0] - length, cpuGeom.verts[i - 1][1], 0.f);
			}
			else if (newPattern.at(i - 1) == 2) {
				next = glm::vec3(cpuGeom.verts[i - 1][0], cpuGeom.verts[i - 1][1] - length, 0.f);
			}
			else if (newPattern.at(i - 1) == 3) {
				next = glm::vec3(cpuGeom.verts[i - 1][0] + length, cpuGeom.verts[i - 1][1], 0.f);
			}
			k++;


			if (i < size) {
				cpuGeom.verts[i] = next;
			}
			else {
				cpuGeom.verts.push_back(next);
				//cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
			}
			if (k == size - 1) {
				//std::cout << "Our result: " << GLsizei(cpuGeom.verts.size()) << std::endl;
				k = 0;
				if (repeat == 0) {
					//std::cout << "Our result: " << GLsizei(cpuGeom.verts.size()) << std::endl;
					cpuGeom.verts.push_back(glm::vec3(cpuGeom.verts[GLsizei(cpuGeom.verts.size()) - 1][0], cpuGeom.verts[GLsizei(cpuGeom.verts.size()) - 1][1] + length, 0.f));
					newPattern.at(i) = 0;
				}
				else if (repeat == 1) {
					cpuGeom.verts.push_back(glm::vec3(cpuGeom.verts[GLsizei(cpuGeom.verts.size()) - 1][0] - length, cpuGeom.verts[GLsizei(cpuGeom.verts.size()) - 1][1], 0.f));
					newPattern.at(i) = 1;
				}
				else if (repeat == 2) {
					cpuGeom.verts.push_back(glm::vec3(cpuGeom.verts[GLsizei(cpuGeom.verts.size()) - 1][0], cpuGeom.verts[GLsizei(cpuGeom.verts.size()) - 1][1] - length, 0.f));
					newPattern.at(i) = 2;
				}
				i++;
				repeat++;
				//cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
			}
			//std::cout << "Our result: " << GLsizei(cpuGeom.verts.size()) << std::endl;
		}
		pattern = newPattern;
	}
	float step = 255.f / float(GLsizei(cpuGeom.verts.size()));
	float colour = -step;
	for (int i = 0; i < GLsizei(cpuGeom.verts.size()); i++) {
		//std::cout << "Our result: " << colour << std::endl;
		colour = colour + step;
		cpuGeom.cols.push_back(glm::vec3(colour / 255.0f, (255.f - colour) / 255.0f, (colour) / 255.0f));
	}
	//std::cout << "Our result: " << GLsizei(cpuGeom.verts.size()) << std::endl;
}

void clear(CPU_Geometry& cpuGeom) {
	cpuGeom.verts.clear();
	cpuGeom.cols.clear();
}

void upload(CPU_Geometry& cpuGeom, GPU_Geometry &gpuGeom) {
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired

	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
	std::shared_ptr<MyCallbacks> callbacks = std::make_shared<MyCallbacks>(shader);
	window.setCallbacks(callbacks); // can also update callbacks to new ones

	// GEOMETRY
	CPU_Geometry triangles;
	GPU_Geometry trianglesGPU;

	//vertices
	State state;

	if (state.scene == 0) {
		generateTriangle(triangles, 0.8f, state.scene);
		if (state.iterations > 0) {
			generateTriangleIteration(triangles.verts[0], triangles.verts[1], triangles.verts[2], triangles, state.iterations, 0);
		}
	}
	else if (state.scene == 1) {
		generateTriangle(triangles, 0.8f, state.scene);
		//generateTriangle(triangles);
		generateInnerTriangles(triangles, triangles.verts[0], triangles.verts[2], triangles.verts[1], state.iterations, 0);
	}
	else if (state.scene == 2) {
		generateTriangle(triangles, 0.5f, state.scene);
		generateSnowflake(triangles, state.iterations);
	}
	else if (state.scene == 3) {
		generateDragonCurve(triangles, state.iterations);
	}
	else if (state.scene == 4) {
		generateHilbertCurve(triangles, state.iterations);
	}


	trianglesGPU.setVerts(triangles.verts);
	trianglesGPU.setCols(triangles.cols);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		upload(triangles, trianglesGPU);

		if (!(state == callbacks->getState())) {
			state = callbacks->getState();
			clear(triangles);

			if (state.scene == 0) {
				generateTriangle(triangles, 0.8f, state.scene);
				//std::cout << "Our result: " << state.iterations << std::endl;
				for (int i = 0; i < state.iterations; i++) {
					generateTriangleIteration(triangles.verts[0], triangles.verts[1], triangles.verts[2], triangles, state.iterations, 0);
				}
			}
			else if (state.scene == 1) {
				generateTriangle(triangles, 0.8f, state.scene);
				//generateTriangle(triangles);
				generateInnerTriangles(triangles, triangles.verts[0], triangles.verts[2], triangles.verts[1], state.iterations, 0);
			}
			else if (state.scene == 2) {
				generateTriangle(triangles, 0.5f, state.scene);
				generateSnowflake(triangles, state.iterations);
			}
			else if (state.scene == 3) {
				generateDragonCurve(triangles, state.iterations);
			}
			else if (state.scene == 4) {
				generateHilbertCurve(triangles, state.iterations);
			}

			upload(triangles, trianglesGPU);
		}

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();

		if (state.scene == 0) {
			trianglesGPU.bind();
			glDrawArrays(GL_TRIANGLES, 0, GLsizei(triangles.verts.size()));
		}
		else if (state.scene == 1) {
			glDrawArrays(GL_LINE_STRIP, 0, GLsizei(triangles.verts.size()));
		}
		else if (state.scene == 2) {
			glDrawArrays(GL_LINE_LOOP, 0, GLsizei(triangles.verts.size()));
		}
		else if (state.scene == 3) {
			glDrawArrays(GL_LINE_STRIP, 0, GLsizei(triangles.verts.size()));
		}
		else if (state.scene == 4) {
			glDrawArrays(GL_LINE_STRIP, 0, GLsizei(triangles.verts.size()));
		}

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
