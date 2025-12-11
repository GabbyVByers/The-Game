#pragma once

#include "PerlinNoise.h"

struct SystemProvince {
	sf::Color color;
	std::vector<sf::Vector2u> corePixels;
	std::vector<sf::Vector2u> marginPixels;
	bool isCoastal = false;
	sf::Vector2f centerMass;
	std::vector<int> neighbourIndices;
	std::vector<sf::Vector2u> pixels;
	std::vector<sf::Vector2u> vertices;
	std::vector<sf::Vertex> borderVertices;
	std::vector<sf::Vertex> triangleVertices;
};

struct RenderProvince {
	int index;
	sf::Color color;
	bool isCoastal = false;
	std::vector<int> neighbourIndices;
	int startIndexBorderVertices;
	int startIndexTriangleVertices;
	int numBorderVertices;
	int numTriangleVertices;
};

class Game {
public:
	// World Generation
	sf::Image worldMap;
	std::vector<SystemProvince> systemProvinces;

	// World Rendering
	sf::Clock deltaClock;
	sf::Transform trasformationMatrix;
	std::vector<sf::Vertex> provinceBorders;
	std::vector<sf::Vertex> provinceTriangles;
	std::vector<RenderProvince> renderProvinces;

	/////////////////////////////////////////////
	//        Constructor / Destructor         //
	/////////////////////////////////////////////

	Game(sf::RenderWindow& window) {
		if (!ImGui::SFML::Init(window))
			assert(false && "Bad ImGui Init");
		ImGui::GetIO().IniFilename = nullptr;
		ImGui::GetIO().FontGlobalScale = 2.0f;
		ImPlot::CreateContext();
	}

	~Game() {
		ImPlot::DestroyContext();
		ImGui::SFML::Shutdown();
	}

	/////////////////////////////////////////////
	//            World Generation             //
	/////////////////////////////////////////////

	void generateWorld(int mapWidth, unsigned int seed) {
		// (1) Generate Perlin Noise Float Array
		PerlinNoise::Initialize(mapWidth, seed);
		PerlinNoise::AddLayer(05, 0.85f);
		PerlinNoise::AddLayer(20, 0.15f);
		std::vector<std::vector<float>> perlinNoise = PerlinNoise::Get2DFloatArray();
		
		// (2) Perlin Noise Float Array + Edge Tapering -> Perlin Noise Image
		worldMap.resize(sf::Vector2u(mapWidth, mapWidth));
		for (int i = 0; i < mapWidth; i++) {
			for (int j = 0; j < mapWidth; j++) {
				float weight = perlinNoise[i][j];
				float halfWidth = (float)mapWidth / 2.0f;
				float dist = sqrt((((float)i - halfWidth) * ((float)i - halfWidth)) + (((float)j - halfWidth) * ((float)j - halfWidth)));
				dist = (halfWidth - dist) / halfWidth;
				dist *= 3.0f;
				dist = fmax(0.0f, dist);
				dist = fmin(1.0f, dist);
				weight *= dist;
				sf::Color taperedColor = sf::Color(255 * weight, 255 * weight, 255 * weight);
				worldMap.setPixel(sf::Vector2u(i, j), taperedColor);
			}
		}
		
		// (3) Perlin Noise Image -> Land Water Image (World Map)
		for (int i = 0; i < mapWidth; i++) {
			for (int j = 0; j < mapWidth; j++) {
				unsigned char depth = worldMap.getPixel(sf::Vector2u(i, j)).r;
				sf::Color landTypeColor;
				if (depth < 127)
					landTypeColor = sf::Color(0, 0, 255);
				else
					landTypeColor = sf::Color(0, 255, 0);
				worldMap.setPixel(sf::Vector2u(i, j), landTypeColor);
			}
		}

		// (4) Seed Pixel Provinces
		int density = 100;
		int stride = mapWidth / density;
		for (int i = 0; i < density; i++) {
			for (int j = 0; j < density; j++) {
				sf::Vector2u position = sf::Vector2u(i * stride, j * stride);
				bool isWater = worldMap.getPixel(position) == sf::Color(0, 0, 255);
				if (isWater) {
					continue;
				}
				sf::Color provColor;
				while (true) {
					provColor = sf::Color(rand() % 255, rand() % 255, rand() % 255);
					bool colorIsUnique = true;
					for (SystemProvince& prov : systemProvinces) {
						if (provColor == prov.color) {
							colorIsUnique = false;
						}
					}
					if (colorIsUnique) {
						break;
					}
				}
				SystemProvince newProvince;
				newProvince.color = provColor;
				newProvince.corePixels.push_back(position);
				std::vector<sf::Vector2i> offsets = {
					sf::Vector2i(-1,  0),
					sf::Vector2i(1,  0),
					sf::Vector2i(0, -1),
					sf::Vector2i(0,  1)
				};
				for (const sf::Vector2i& offset : offsets) {
					sf::Vector2u marginPixel = position;
					marginPixel.x += offset.x;
					marginPixel.y += offset.y;
					bool isFreeRealEstate = worldMap.getPixel(marginPixel) == sf::Color(0, 255, 0);
					if (!isFreeRealEstate) {
						continue;
					}
					newProvince.marginPixels.push_back(marginPixel);
				}
				worldMap.setPixel(position, provColor);
				systemProvinces.push_back(newProvince);
			}
		}

		// (5) Itterate Pixel Province Growth Until Finished
		while (true) {
			for (SystemProvince& province : systemProvinces) {
				if (province.marginPixels.size() == 0) {
					continue;
				}
				std::vector<sf::Vector2u> prunedMarginPixels;
				for (int i = 0; i < province.marginPixels.size(); i++) {
					sf::Vector2u marginPixel = province.marginPixels[i];
					bool isFreeRealEstate = worldMap.getPixel(marginPixel) == sf::Color(0, 255, 0);
					if (isFreeRealEstate) {
						prunedMarginPixels.push_back(marginPixel);
					}
				}
				province.marginPixels = prunedMarginPixels;
				if (province.marginPixels.size() == 0) {
					continue;
				}
				int randMarginIndex = rand() % province.marginPixels.size();
				sf::Vector2u marginPixel = province.marginPixels[randMarginIndex];
				province.corePixels.push_back(marginPixel);
				worldMap.setPixel(marginPixel, province.color);
				province.marginPixels.erase(province.marginPixels.begin() + randMarginIndex);
				std::vector<sf::Vector2i> offsets = {
						sf::Vector2i(-1,  0),
						sf::Vector2i(1,  0),
						sf::Vector2i(0, -1),
						sf::Vector2i(0,  1)
				};
				for (sf::Vector2i& offset : offsets) {
					sf::Vector2u newMarginPixel = marginPixel;
					newMarginPixel.x += offset.x;
					newMarginPixel.y += offset.y;
					bool isFreeRealEstate = worldMap.getPixel(newMarginPixel) == sf::Color(0, 255, 0);
					if (!isFreeRealEstate) {
						continue;
					}
					province.marginPixels.push_back(newMarginPixel);
				}
			}
			bool doneItterating = true;
			for (SystemProvince& province : systemProvinces) {
				if (province.marginPixels.size() > 0) {
					doneItterating = false;
					break;
				}
			}
			if (doneItterating) {
				break;
			}
		}

		// (6) Provinces Get Pixels
		for (int i = 0; i < mapWidth; i++) {
			for (int j = 0; j < mapWidth; j++) {
				sf::Vector2u currentPosition = sf::Vector2u(i, j);
				sf::Color currentColor = worldMap.getPixel(currentPosition);
				if (currentColor == sf::Color(0, 255, 0))
					continue;
				if (currentColor == sf::Color(0, 0, 255))
					continue;
				for (SystemProvince& province : systemProvinces) {
					if (province.color == currentColor) {
						province.pixels.push_back(currentPosition);
						break;
					}
				}
			}
		}

		// (7) Get Unsorted Vertices
		// (7.1) Edge Vertices
		for (SystemProvince& province : systemProvinces) {
			for (sf::Vector2u& pixel : province.pixels) {
				sf::Vector2u& currentPosition = pixel;
				sf::Color currentColor = worldMap.getPixel(currentPosition);
				std::vector<sf::Vector2i> offsets = {
					sf::Vector2i(-1,  0),
					sf::Vector2i(1,  0),
					sf::Vector2i(0, -1),
					sf::Vector2i(0,  1)
				};
				for (sf::Vector2i offset : offsets) {
					sf::Vector2u offsetPosition = currentPosition;
					offsetPosition.x += offset.x;
					offsetPosition.y += offset.y;
					sf::Color offsetColor = worldMap.getPixel(offsetPosition);
					if (currentColor != offsetColor) {
						sf::Vector2u vertexPosition = currentPosition;
						vertexPosition.x *= 2;
						vertexPosition.y *= 2;
						vertexPosition.x += offset.x;
						vertexPosition.y += offset.y;
						province.vertices.push_back(vertexPosition);
					}
				}
			}
		}

		// (7.2) Corner Vertices
		for (SystemProvince& province : systemProvinces) {
			for (sf::Vector2u& pixel : province.pixels) {
				sf::Vector2u& currentPosition = pixel;
				sf::Color currentColor = worldMap.getPixel(currentPosition);
				std::vector<sf::Vector2i> offsets = {
					sf::Vector2i(-1, -1),
					sf::Vector2i(-1,  1),
					sf::Vector2i(1, -1),
					sf::Vector2i(1,  1)
				};
				for (sf::Vector2i offset : offsets) {
					sf::Vector2u offsetPosition_A = currentPosition;
					sf::Vector2u offsetPosition_B = currentPosition;
					offsetPosition_A.x += offset.x;
					offsetPosition_B.y += offset.y;
					sf::Color color_A = worldMap.getPixel(offsetPosition_A);
					sf::Color color_B = worldMap.getPixel(offsetPosition_B);
					if (color_A != color_B) {
						if ((color_A != currentColor) && (color_B != currentColor)) {
							sf::Vector2u vertexPosition = currentPosition;
							vertexPosition.x *= 2;
							vertexPosition.y *= 2;
							vertexPosition.x += offset.x;
							vertexPosition.y += offset.y;
							province.vertices.push_back(vertexPosition);
						}
					}
				}
			}
		}

		// (7.3) New Corner Vertices
		for (SystemProvince& province : systemProvinces) {
			for (sf::Vector2u pixel : province.pixels) {
				sf::Vector2u currentPosition = pixel;
				sf::Color currentColor = worldMap.getPixel(currentPosition);
				std::vector<sf::Vector2i> offsets = {
					sf::Vector2i(-1, -1),
					sf::Vector2i(-1,  1),
					sf::Vector2i(1, -1),
					sf::Vector2i(1,  1)
				};
				for (sf::Vector2i offset : offsets) {
					sf::Vector2u offsetPosition = currentPosition;
					sf::Vector2u position_A = currentPosition;
					sf::Vector2u position_B = currentPosition;
					sf::Vector2u position_C = currentPosition;
					sf::Vector2u position_D = currentPosition;
					position_B.y += offset.y;
					position_C.x += offset.x;
					position_D.x += offset.x;
					position_D.y += offset.y;
					sf::Color color_A = worldMap.getPixel(position_A);
					sf::Color color_B = worldMap.getPixel(position_B);
					sf::Color color_C = worldMap.getPixel(position_C);
					sf::Color color_D = worldMap.getPixel(position_D);
					if (color_A == color_B) {
						if (color_A != color_D) {
							if (color_A != color_C) {
								sf::Vector2u vertexPosition = currentPosition;
								vertexPosition.x *= 2;
								vertexPosition.y *= 2;
								vertexPosition.x += offset.x;
								vertexPosition.y += offset.y;
								province.vertices.push_back(vertexPosition);
							}
						}
					}
					if (color_A == color_C) {
						if (color_A != color_D) {
							if (color_A != color_B) {
								sf::Vector2u vertexPosition = currentPosition;
								vertexPosition.x *= 2;
								vertexPosition.y *= 2;
								vertexPosition.x += offset.x;
								vertexPosition.y += offset.y;
								province.vertices.push_back(vertexPosition);
							}
						}
					}
				}
			}
		}

		// (7.4) Remove Duplicate Vertices
		for (SystemProvince& province : systemProvinces) {
			std::vector<sf::Vector2u> prunedVertices;
			for (sf::Vector2u vertex : province.vertices) {
				bool isDuplicate = false;
				for (sf::Vector2u otherVertex : prunedVertices) {
					if (vertex.x == otherVertex.x) {
						if (vertex.y == otherVertex.y) {
							isDuplicate = true;
						}
					}
				}
				if (!isDuplicate) {
					prunedVertices.push_back(vertex);
				}
			}
			province.vertices = prunedVertices;
		}

		// (8) Sort Vertices
		for (SystemProvince& province : systemProvinces) {
			std::vector<sf::Vector2u> sortedVertices;
			std::vector<sf::Vector2u>& unsortedVertices = province.vertices;
			sortedVertices.push_back(unsortedVertices[0]);
			unsortedVertices.erase(unsortedVertices.begin());

			while (true) {
				if (unsortedVertices.size() == 0) {
					break;
				}
				sf::Vector2u currentVertex = sortedVertices[sortedVertices.size() - 1];
				float closestDistance = FLT_MAX;
				int indexClosestVertex = 0;
				for (int i = 0; i < unsortedVertices.size(); i++) {
					sf::Vector2u otherVertex = unsortedVertices[i];
					if (sortedVertices.size() == 1) {
						if ((float)otherVertex.y < (float)currentVertex.y) {
							continue;
						}
					}
					float distance = (((float)currentVertex.x - (float)otherVertex.x) * ((float)currentVertex.x - (float)otherVertex.x)) +
						(((float)currentVertex.y - (float)otherVertex.y) * ((float)currentVertex.y - (float)otherVertex.y));
					if (distance < closestDistance) {
						closestDistance = distance;
						indexClosestVertex = i;
					}
				}
				sortedVertices.push_back(unsortedVertices[indexClosestVertex]);
				unsortedVertices.erase(unsortedVertices.begin() + indexClosestVertex);
			}
			province.vertices = sortedVertices;
		}

		// (9) Remove Inline Vertices
		for (SystemProvince& province : systemProvinces) {
			std::vector<sf::Vector2u> prunedVertices;
			for (int i = 0; i < province.vertices.size(); i++) {
				int currIndex = i;
				int prevIndex = i - 1;
				int nextIndex = i + 1;
				if (i == 0)
					prevIndex = province.vertices.size() - 1;
				if (i == province.vertices.size() - 1)
					nextIndex = 0;
				sf::Vector2u curr = province.vertices[i];
				sf::Vector2u prev = province.vertices[prevIndex];
				sf::Vector2u next = province.vertices[nextIndex];
				sf::Vector2f A = sf::Vector2f((float)next.x - (float)curr.x, (float)next.y - (float)curr.y);
				sf::Vector2f B = sf::Vector2f((float)prev.x - (float)curr.x, (float)prev.y - (float)curr.y);
				A = A.normalized();
				B = B.normalized();
				float dotProd = (A.x * B.x) + (A.y * B.y);
				if (dotProd > -0.8f) {
					prunedVertices.push_back(curr);
				}
			}
			province.vertices = prunedVertices;
		}

		// (10) Find Center of Mass
		for (SystemProvince& province : systemProvinces) {
			float x_sum = 0.0f;
			float y_sum = 0.0f;
			int count = 0;
			for (sf::Vector2u pixel : province.pixels) {
				x_sum += (float)pixel.x * 2;
				y_sum += (float)pixel.y * 2;
				count++;
			}
			sf::Vector2f centerMass = sf::Vector2f(x_sum / count, y_sum / count);
			province.centerMass = centerMass;
		}

		// (11) Find Friends
		for (SystemProvince& province : systemProvinces) {
			std::vector<sf::Color> friendColors;
			for (sf::Vector2u pixel : province.pixels) {
				std::vector<sf::Vector2i> offsets = {
					sf::Vector2i(-1,  0),
					sf::Vector2i(1,  0),
					sf::Vector2i(0, -1),
					sf::Vector2i(0,  1)
				};
				for (sf::Vector2i offset : offsets) {
					sf::Vector2u offsetPosition = pixel;
					offsetPosition.x += offset.x;
					offsetPosition.y += offset.y;
					sf::Color offsetColor = worldMap.getPixel(offsetPosition);
					if (offsetColor != province.color) {
						friendColors.push_back(offsetColor);
					}
				}
			}
			for (int i = 0; i < systemProvinces.size(); i++) {
				SystemProvince& friendProvince = systemProvinces[i];
				for (sf::Color friendColor : friendColors) {
					if (friendColor == friendProvince.color) {
						province.neighbourIndices.push_back(i);
						continue;
					}
				}
			}
		}

		// (12) Find Coastal Provinces
		for (SystemProvince& province : systemProvinces) {
			for (sf::Vector2u pixel : province.pixels) {
				std::vector<sf::Vector2i> offsets = {
					sf::Vector2i(-1,  0),
					sf::Vector2i(1,  0),
					sf::Vector2i(0, -1),
					sf::Vector2i(0,  1)
				};
				for (sf::Vector2i offset : offsets) {
					sf::Vector2u offsetPosition = pixel;
					offsetPosition.x += offset.x;
					offsetPosition.y += offset.y;
					sf::Color color = worldMap.getPixel(offsetPosition);
					if (color == sf::Color(0, 0, 255)) {
						province.isCoastal = true;
						continue;
					}
				}
			}
		}

		// (13) Get Border sf::Vertices
		for (SystemProvince& province : systemProvinces) {
			for (int i = 0; i < province.vertices.size(); i++) {
				int currIndex = i;
				int nextIndex = i + 1;
				if (nextIndex == province.vertices.size()) {
					nextIndex = 0;
				}
				sf::Vector2u vec_A = province.vertices[currIndex];
				sf::Vector2u vec_B = province.vertices[nextIndex];
				sf::Vertex vertex_A; vertex_A.position = sf::Vector2f((float)vec_A.x, (float)vec_A.y);
				sf::Vertex vertex_B; vertex_B.position = sf::Vector2f((float)vec_B.x, (float)vec_B.y);
				vertex_A.color = sf::Color::Black;
				vertex_B.color = sf::Color::Black;
				province.borderVertices.push_back(vertex_A);
				province.borderVertices.push_back(vertex_B);
			}
		}

		// (14) Clip Ears
		for (SystemProvince& province : systemProvinces) {
			std::vector<sf::Vertex> polygonVertices;
			for (sf::Vector2u& vec : province.vertices) {
				sf::Vertex vertex;
				vertex.position.x = (float)vec.x;
				vertex.position.y = (float)vec.y;
				polygonVertices.push_back(vertex);
			}

			std::vector<sf::Vertex>& triangleVertices = province.triangleVertices;
			int currIndex = 0;

			while (true) {
				int prevIndex = currIndex - 1;
				int nextIndex = currIndex + 1;
				if (prevIndex == -1) {
					prevIndex = polygonVertices.size() - 1;
				}
				if (nextIndex == polygonVertices.size()) {
					nextIndex = 0;
				}

				bool pointInsideTriangleTestPassed = true;
				bool convexVertexTestPassed = true;

				sf::Vertex& prevVert = polygonVertices[prevIndex];
				sf::Vertex& currVert = polygonVertices[currIndex];
				sf::Vertex& nextVert = polygonVertices[nextIndex];
				prevVert.color = sf::Color::Green;
				currVert.color = sf::Color::Green;
				nextVert.color = sf::Color::Green;

				sf::Vector3f A = sf::Vector3f(prevVert.position.x - currVert.position.x, prevVert.position.y - currVert.position.y, 0.0f);
				sf::Vector3f B = sf::Vector3f(nextVert.position.x - currVert.position.x, nextVert.position.y - currVert.position.y, 0.0f);
				A = A.normalized();
				B = B.normalized();
				sf::Vector3f AxB = A.cross(B);
				float AdotB = A.dot(B);

				if (AxB.z <= 0.0001f) {
					convexVertexTestPassed = false;
				}

				if (AdotB < -0.9999) {
					convexVertexTestPassed = false;
				}

				for (int i = 0; i < polygonVertices.size(); i++) {
					if (i == prevIndex)
						continue;
					if (i == currIndex)
						continue;
					if (i == nextIndex)
						continue;
					sf::Vertex vertex = polygonVertices[i];
					sf::Vector2f P = vertex.position;
					sf::Vector2f A = prevVert.position;
					sf::Vector2f B = currVert.position;
					sf::Vector2f C = nextVert.position;

					for (int i = 0; i < 3; i++) {
						float W1 = ((A.x * (C.y - A.y)) + ((P.y - A.y) * (C.x - A.x)) - (P.x * (C.y - A.y))) /
							(((B.y - A.y) * (C.x - A.x)) - ((B.x - A.x) * (C.y - A.y)));
						float W2 = (P.y - A.y - (W1 * (B.y - A.y))) /
							(C.y - A.y);

						if (W1 > -0.0001f) {
							if (W2 > 0.0001f) {
								if ((W1 + W2) < 1.0001f) {
									pointInsideTriangleTestPassed = false;
								}
							}
						}

						sf::Vector2f tempA = A;
						sf::Vector2f tempB = B;
						sf::Vector2f tempC = C;
						A = tempC;
						B = tempA;
						C = tempB;
					}
				}

				if (!pointInsideTriangleTestPassed) {
					currIndex++;
					continue;
				}

				if (!convexVertexTestPassed) {
					currIndex++;
					continue;
				}

				triangleVertices.push_back(prevVert);
				triangleVertices.push_back(currVert);
				triangleVertices.push_back(nextVert);

				polygonVertices.erase(polygonVertices.begin() + currIndex);
				currIndex = 0;

				if (polygonVertices.size() == 3) {
					sf::Vertex vert1 = polygonVertices[0];
					sf::Vertex vert2 = polygonVertices[1];
					sf::Vertex vert3 = polygonVertices[2];
					vert1.color = sf::Color::Green;
					vert2.color = sf::Color::Green;
					vert3.color = sf::Color::Green;
					triangleVertices.push_back(vert1);
					triangleVertices.push_back(vert2);
					triangleVertices.push_back(vert3);
					break;
				}
			}
		}

		// (15) Final Step - Consolidating World Geometry
		int i = 0;
		for (SystemProvince& system_province : systemProvinces) {
			RenderProvince render_province;
			render_province.numTriangleVertices = system_province.triangleVertices.size();
			render_province.numBorderVertices = system_province.borderVertices.size();
			render_province.startIndexTriangleVertices = provinceTriangles.size();
			render_province.startIndexBorderVertices = provinceBorders.size();
			for (const sf::Vertex& triangleVert : system_province.triangleVertices) {
				provinceTriangles.push_back(triangleVert);
			}
			for (const sf::Vertex& borderVert : system_province.borderVertices) {
				provinceBorders.push_back(borderVert);
			}
			render_province.isCoastal = system_province.isCoastal;
			for (int fren : system_province.neighbourIndices) {
				render_province.neighbourIndices.push_back(fren);
			}
			render_province.color = system_province.color;
			render_province.index = i;
			renderProvinces.push_back(render_province);
			i++;
		}

		trasformationMatrix.scale(sf::Vector2f(0.65f, 0.65f));
		for (int i = 0; i < renderProvinces.size(); i++) {
			RenderProvince& render_province = renderProvinces[i];
			if (render_province.isCoastal) {
				setProvinceFillColor(i, sf::Color::Yellow);
			}
		}
	}

	/////////////////////////////////////////////
	//          Rendering Functions            //
	/////////////////////////////////////////////

	float getCurrentScale() {
		const float* matrix = trasformationMatrix.getMatrix();
		float scale = std::sqrt(matrix[0] * matrix[0] + matrix[1] * matrix[1]);
		return scale;
	}

	sf::Vector2i getCurrentTranslation() {
		float tx = trasformationMatrix.getMatrix()[12];
		float ty = trasformationMatrix.getMatrix()[13];
		sf::Vector2i globalPosition = sf::Vector2i((int)tx, (int)ty);
		return globalPosition;
	}

	void zoom(sf::RenderWindow& window, int delta) {
		sf::Vector2i localMousePos = sf::Mouse::getPosition(window) - getCurrentTranslation();
		sf::Vector2f mouseBefore = sf::Vector2f(localMousePos.x / getCurrentScale(), localMousePos.y / getCurrentScale());
		float factor = 1.0f + (float)delta * 0.05f;
		trasformationMatrix.scale(sf::Vector2f(factor, factor));
		sf::Vector2f mouseAfter = sf::Vector2f(localMousePos.x / getCurrentScale(), localMousePos.y / getCurrentScale());
		trasformationMatrix.translate(mouseAfter - mouseBefore);
	}

	void pan(sf::RenderWindow& window) {
		static sf::Vector2i previousMousePosition;
		sf::Vector2i currentMousePosition = sf::Mouse::getPosition(window);
		sf::Vector2f offset;
		offset.x = (float)currentMousePosition.x - (float)previousMousePosition.x;
		offset.y = (float)currentMousePosition.y - (float)previousMousePosition.y;
		offset /= (float)getCurrentScale();
		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
			trasformationMatrix.translate(offset);
		}
		previousMousePosition = currentMousePosition;
	}

	void run(sf::RenderWindow& window) {
		handleEvents(window);
		ImGui::SFML::Update(window, deltaClock.restart());
		pan(window);
		window.clear(sf::Color::Blue);
		window.draw(&provinceTriangles[0], provinceTriangles.size(), sf::PrimitiveType::Triangles, trasformationMatrix);
		window.draw(&provinceBorders[0], provinceBorders.size(), sf::PrimitiveType::Lines, trasformationMatrix);
		gui();
		ImGui::SFML::Render(window);
		window.display();
	}

	void setProvinceFillColor(int provIndex, sf::Color color) {
		RenderProvince& province = renderProvinces[provIndex];
		for (int i = 0; i < province.numTriangleVertices; i++) {
			sf::Vertex& vertex = provinceTriangles[i + province.startIndexTriangleVertices];
			vertex.color = color;
		}
	}

	void setProvinceBorderColor(int provIndex, sf::Color color) {
		RenderProvince& province = renderProvinces[provIndex];
		for (int i = 0; i < province.numBorderVertices; i++) {
			sf::Vertex& vertex = provinceBorders[i + province.startIndexBorderVertices];
			vertex.color = color;
		}
	}

	void selectProvince(sf::Vector2i mousePosition) {
		sf::Vector2i globalPosition = getCurrentTranslation();
		sf::Vector2i localMousePosition = mousePosition - globalPosition;
		sf::Vector2u imageIndex;
		imageIndex.x = 0.5 * (localMousePosition.x / getCurrentScale());
		imageIndex.y = 0.5 * (localMousePosition.y / getCurrentScale());
		sf::Vector2u imageSize = worldMap.getSize();
		if (imageIndex.x >= imageSize.x) {
			return;
		}
		if (imageIndex.y >= imageSize.y) {
			return;
		}
		sf::Color color = worldMap.getPixel(imageIndex);
		int index = -1;
		for (RenderProvince& province : renderProvinces) {
			if (province.color == color) {
				index = province.index;
				break;
			}
		}
		if (index != -1) {
			setProvinceFillColor(index, sf::Color::White);
		}
	}

	/////////////////////////////////////////////
	//             Event Handling              //
	/////////////////////////////////////////////

	void handleEvents(sf::RenderWindow& window) {
		while (const std::optional event = window.pollEvent()) {
			ImGui::SFML::ProcessEvent(window, *event);
			if (event->is<sf::Event::Closed>())
				window.close();
			if (const auto* resized = event->getIf<sf::Event::Resized>()) {
				sf::FloatRect visible_area({ 0.0f, 0.0f }, sf::Vector2f(resized->size));
				window.setView(sf::View(visible_area));
			}
			if (const auto& mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
				zoom(window, mouseWheelScrolled->delta);
			}
			if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
			    if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
			        selectProvince(sf::Mouse::getPosition(window));
			    }
			}
		}
	}

	/////////////////////////////////////////////
	//             User Interface              //
	/////////////////////////////////////////////

	void gui() {
		ImGui::Begin("Window");
		ImGui::Text("Test");
		ImGui::End();
	}
};

