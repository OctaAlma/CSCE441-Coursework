#include <iostream>
#include <string>
#include <float.h> // ADDED

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Image.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

double RANDOM_COLORS[7][3] = {
	{0.0000,    0.4470,    0.7410},
	{0.8500,    0.3250,    0.0980},
	{0.9290,    0.6940,    0.1250},
	{0.4940,    0.1840,    0.5560},
	{0.4660,    0.6740,    0.1880},
	{0.3010,    0.7450,    0.9330},
	{0.6350,    0.0780,    0.1840},
};

// Custom data structures:

// A color struct that stores r,g,b values
struct Color{
	double r, g, b;

	Color():r(0), g(0), b(0){}
	Color(double r, double g, double b):r(r), g(g), b(b){}
};

struct normal
{
	double x, y, z;
	normal():x(0), y(0), z(0){}
	normal(double x, double y, double z):x(x),y(y),z(z){}
};


// A vertex in 3D space
struct Vertex3D{
    double x, y, z;
	double r, g, b;

	normal nor;

    // Constructors:
    Vertex3D(){
        x = y = z = r = g = b = -1;
    }

    Vertex3D(double x, double y, double z, double r, double g, double b): x(x), y(y), z(z), r(r), g(g), b(b){}

	Vertex3D(double x, double y, double z):x(x), y(y), z(z){
		r = g = b = -1;
	}

	void changeColor(double r, double g, double b){ 
		this->r = r;
		this->g = g;
		this->b = b;
	}

	void changeColor(double * col){
		r = col[0] * 255;
		g = col[1] * 255;
		b = col[2] * 255;
	}

	void setNormal(double x, double y, double z){
		nor.x = x;
		nor.y = y; 
		nor.z = z;
	}
};

// The bounding box of the triangle is the box with the extents that will exactly bound that triangle.
struct BoundingBox{
    double xmin, ymin;
	double xmax, ymax;

	BoundingBox(){
		xmin = xmax = ymin = ymax = INT32_MIN;
	}
    
    BoundingBox(double xmin, double ymin, double xmax, double ymax):
        xmin(xmin), ymin(ymin),
		xmax(xmax), ymax(ymax)
    {}

    BoundingBox(vector<Vertex3D> vertices){
        if (vertices.empty()){
            return;
        }

        xmin = vertices.at(0).x;
        xmax = vertices.at(0).x;

        ymin = vertices.at(0).y;
        ymax = vertices.at(0).y;

        double currX, currY;

        for (int i = 1; i < vertices.size(); i++){
            currX = vertices.at(i).x;
            currY = vertices.at(i).y;

            if (currX < xmin){
                xmin = currX;
            }
            
            if (currX > xmax){
                xmax = currX;
            }

            if (currY < ymin){
                ymin = currY;
            }
            
            if (currY > ymax){
                ymax = currY;
            }
        }
    }

	BoundingBox(Vertex3D A, Vertex3D B, Vertex3D C){
		xmin = xmax = A.x;
		ymin = ymax = A.y;
		Vertex3D* ptr;

		for (int i = 1; i < 3; i++){
			if (i == 1){ptr = &B;}
			if (i == 2){ptr = &C;}

			if (ptr->x < xmin){
				xmin = ptr->x;
			} 
			if (ptr->x > xmax){
				xmax = ptr->x;
			}

			if (ptr->y < ymin){
				ymin = ptr->y;
			} 
			
			if (ptr->y > ymax){
				ymax = ptr->y;
			}
		}
	}

    double midPointX(){
        return (double)(xmax+xmin)/(double)2;
    }

    double midPointY(){
        return (double)(ymax + ymin)/(double)2;
    }
};

// A data structure representing a triangle:
struct Triangle{
	Vertex3D A, B, C;

	BoundingBox boundary;

	Triangle(vector<float> &posbuf, vector<float> &norBuf, int start){

		A = Vertex3D(posbuf[start], posbuf[start + 1], posbuf[start + 2]);
		B = Vertex3D(posbuf[start + 3], posbuf[start + 4], posbuf[start + 5]);
		C = Vertex3D(posbuf[start + 6], posbuf[start + 7], posbuf[start + 8]);

		A.setNormal(norBuf[start], norBuf[start + 1], norBuf[start + 2]);
		B.setNormal(norBuf[start + 3], norBuf[start + 4], norBuf[start + 5]);
		C.setNormal(norBuf[start + 6], norBuf[start + 7], norBuf[start + 8]);

		A.changeColor(RANDOM_COLORS[start%7]);
		B.changeColor(RANDOM_COLORS[(start+1)%7]);
		C.changeColor(RANDOM_COLORS[(start+2)%7]);

		boundary = BoundingBox(A, B, C);

	}

	void changeColor(double r, double g, double b){
		A.changeColor(r, g, b);
		B.changeColor(r, g, b);
		C.changeColor(r, g, b);
	}

	// Print statement used for debugging:
	void print(){
		cout << "A | x = " << A.x << " | y = " << A.y << " | z = " << A.z << endl;
		cout << "B | x = " << B.x << " | y = " << B.y << " | z = " << B.z << endl;
		cout << "C | x = " << C.x << " | y = " << C.y << " | z = " << C.z << endl;

		cout << "Bounding Box | xMin = " << boundary.xmin << " | xMax = " << boundary.xmax;
		cout << " | yMin = " << boundary.ymin << " | yMax = " << boundary.ymax << endl << endl;  

	}
	
	// Computes the area of the current triangle:
	double getTotalArea(){
		return 0.5 * ( (B.x-A.x) * (C.y-A.y) - (C.x-A.x) * (B.y-A.y));
	}
};

// Computes the area of the triangle formed between 3 vertices
double area(Vertex3D v0, Vertex3D v1, Vertex3D v2){    
	return 0.5 * ( (v1.x-v0.x) * (v2.y-v0.y) - (v2.x-v0.x) * (v1.y-v0.y));
}

double computeScale(BoundingBox world, int width, int height){   

    // Note: since image coordinates always start at (o,o), we only need the image width and height in terms of pixels
    double scaleX = ((double)width)/((double)(world.xmax - world.xmin));
    double scaleY = ((double)height)/((double)(world.ymax - world.ymin));

    return min(scaleX, scaleY);
}

bool isBetween0and1(double a){
    if (a <= 1 && a >= 0){
        return true;
    }

    return false;
}

int convertToImgCoords(double worldCoord, double scale, double translation){
	return worldCoord * scale + translation;
}

// Task 1: Drawing bounding boxes
void performTask1(shared_ptr<Image> image, int width, int height, double scale, double translationX, double translationY, int i, Triangle* currTriangle, Color** frameBuffer, double** zBuffer){
	int r, g, b;

	for (int x = (currTriangle->boundary.xmin * scale + translationX); x < (currTriangle->boundary.xmax * scale + translationX); x = x + 1){
		for (int y = (currTriangle->boundary.ymin * scale + translationY); y < (currTriangle->boundary.ymax * scale + translationY); y = y + 1){
			
			r = (RANDOM_COLORS[i % 7][0] * 255);
			g = (RANDOM_COLORS[i % 7][1] * 255);
			b = (RANDOM_COLORS[i % 7][2] * 255);

			image->setPixel(x, y, r, g, b);

		}	
	}
}

// Task 2: Drawing triangles
void performTask2(shared_ptr<Image> image, int width, int height, double scale, double translationX, double translationY, int i, Triangle* currTriangle, Color** frameBuffer, double** zBuffer){
	double totalArea = currTriangle->getTotalArea();
	double a, b, c;
	int r, g, bl;

	// Get color:
	r = (int)(RANDOM_COLORS[i % 7][0] * 255);
	g = (int)(RANDOM_COLORS[i % 7][1] * 255);
	bl = (int)(RANDOM_COLORS[i % 7][2] * 255);

	// for all pixels in the triangle bounding box:
	for (int x = (currTriangle->boundary.xmin * scale + translationX); x < (currTriangle->boundary.xmax * scale + translationX); x = x + 1){
		for (int y = (currTriangle->boundary.ymin * scale + translationY); y < (currTriangle->boundary.ymax * scale + translationY); y = y + 1){

			Vertex3D currPoint((x - translationX)/scale, (y - translationY)/scale, INT32_MIN);
			
			// Compute the barycentric coordinates:
			a = area(currPoint, currTriangle->B, currTriangle->C)/totalArea;
			b = area(currTriangle->A, currPoint, currTriangle->C)/totalArea;
			c = area(currTriangle->A, currTriangle->B, currPoint)/totalArea;

			// The point is inside of the triangle if a, b, and c are all between 0 and 1:
			if (isBetween0and1(a) && isBetween0and1(b) && isBetween0and1(c)){
				// if inside:
				currPoint.changeColor(r, g, bl);

				// compute pixel’s z-coordinate 
				currPoint.z = ((currTriangle->A.z * a) + (currTriangle->B.z * b) + (currTriangle->C.z * c));

				//	if z > current z from Zbuff:
				if (currPoint.z > zBuffer[x][y]){
					// write RBG color to Fbuff
					frameBuffer[x][y] = Color(currPoint.r, currPoint.g, currPoint.b);

					// write z-coordinate to Zbuff
					zBuffer[x][y] = currPoint.z;
					image->setPixel(x, y, currPoint.r, currPoint.g, currPoint.b);
				}
			}
		}	
	}
}

// Task 3: Interpolating Per-Vertex Color
void performTask3(shared_ptr<Image> image, int width, int height, double scale, double translationX, double translationY, int i, Triangle* currTriangle, Color** frameBuffer, double** zBuffer){
	double totalArea = currTriangle->getTotalArea();
	double a, b, c;

	// for all pixels in the triangle bounding box:
	for (int x = (currTriangle->boundary.xmin * scale + translationX); x < (currTriangle->boundary.xmax * scale + translationX); x = x + 1){
		for (int y = (currTriangle->boundary.ymin * scale + translationY); y < (currTriangle->boundary.ymax * scale + translationY); y = y + 1){

			Vertex3D currPoint((x - translationX)/scale, (y - translationY)/scale, INT32_MIN);
			
			// Compute the barycentric coordinates:
			a = area(currPoint, currTriangle->B, currTriangle->C)/totalArea;
			b = area(currTriangle->A, currPoint, currTriangle->C)/totalArea;
			c = area(currTriangle->A, currTriangle->B, currPoint)/totalArea;

			// The point is inside of the triangle if a, b, and c are all between 0 and 1:
			if (isBetween0and1(a) && isBetween0and1(b) && isBetween0and1(c)){

				// if inside:
				// compute pixel’s RBG color
				currPoint.r = ((currTriangle->A.r * a) + (currTriangle->B.r * b) + (currTriangle->C.r * c));
				currPoint.g = ((currTriangle->A.g * a) + (currTriangle->B.g * b) + (currTriangle->C.g * c));
				currPoint.b = ((currTriangle->A.b * a) + (currTriangle->B.b * b) + (currTriangle->C.b * c));

				// compute pixel’s z-coordinate 
				currPoint.z = ((currTriangle->A.z * a) + (currTriangle->B.z * b) + (currTriangle->C.z * c));

				//	if z > current z from Zbuff:
				if (currPoint.z > zBuffer[x][y]){
					// write RBG color to Fbuff
					frameBuffer[x][y] = Color(currPoint.r, currPoint.g, currPoint.b);

					// write z-coordinate to Zbuff
					zBuffer[x][y] = currPoint.z;
					image->setPixel(x, y, currPoint.r, currPoint.g, currPoint.b);
				}
			}
		}	
	}
}

// Task 4: Vertical Color - DONE
void performTask4(BoundingBox worldBox, shared_ptr<Image> image, int width, int height, double scale, double translationX, double translationY, int i, Triangle* currTriangle, Color** frameBuffer, double** zBuffer){
	double totalArea = currTriangle->getTotalArea();
	double a, b, c;

	// The color should vary smoothly from top of the object to the bottom of the object, rather than from 
	// the top of the whole image to the bottom of the whole image. In other words, the top tip of the triangle should be fully red.

	int yImgMin = (worldBox.ymin * scale + translationY);
	int yImgMax = (worldBox.ymax * scale + translationY);

	// for all pixels in the triangle bounding box:
	for (int x = (currTriangle->boundary.xmin * scale + translationX); x < (currTriangle->boundary.xmax * scale + translationX); x = x + 1){
		for (int y = (currTriangle->boundary.ymin * scale + translationY); y < (currTriangle->boundary.ymax * scale + translationY); y = y + 1){

			Vertex3D currPoint((x - translationX)/scale, (y - translationY)/scale, INT32_MIN);
			
			// Compute the barycentric coordinates:
			a = area(currPoint, currTriangle->B, currTriangle->C)/totalArea;
			b = area(currTriangle->A, currPoint, currTriangle->C)/totalArea;
			c = area(currTriangle->A, currTriangle->B, currPoint)/totalArea;

			// The point is inside of the triangle if a, b, and c are all between 0 and 1:
			if (isBetween0and1(a) && isBetween0and1(b) && isBetween0and1(c)){

				// if inside:
				// compute pixel’s RBG color
                // Note: In this case, the color is dependent on the vertex's y-axis position
                // Note: Top is red and bottom is blue
				double amountBlue, amountRed;
                // Use the y-value to linearly interpolate two colors: blue (0 0 255) and red (255 0 0).
				amountBlue = (double)(yImgMax - y)/(double)(yImgMax-yImgMin);
				amountRed = (double)(y - yImgMin)/(double)(yImgMax-yImgMin);
				
				currPoint.r = (int)(amountRed * 255);
				currPoint.g = 0;
				currPoint.b = (int)(amountBlue * 255);

				// compute pixel’s z-coordinate 
				currPoint.z = ((currTriangle->A.z * a) + (currTriangle->B.z * b) + (currTriangle->C.z * c));

				//	if z > current z from Zbuff:
				if (currPoint.z > zBuffer[x][y]){
					// write RBG color to Fbuff
					frameBuffer[x][y] = Color(currPoint.r, currPoint.g, currPoint.b);

					// write z-coordinate to Zbuff
					zBuffer[x][y] = currPoint.z;
					image->setPixel(x, y, currPoint.r, currPoint.g, currPoint.b);
				}
			}
		}	
	}
}

// Task 5: Z-Buffering
void performTask5(shared_ptr<Image> image, int width, int height, double scale, double translationX, double translationY, int i, Triangle* currTriangle, Color** frameBuffer, double** zBuffer, double zmin, double zmax){
	double totalArea = currTriangle->getTotalArea();
	double a, b, c;

	currTriangle->A.changeColor(currTriangle->A.z, 0, 0);
	currTriangle->B.changeColor(currTriangle->B.z, 0, 0);
	currTriangle->C.changeColor(currTriangle->C.z, 0, 0);

	// for all pixels in the triangle bounding box:
	for (int x = (currTriangle->boundary.xmin * scale + translationX); x < (currTriangle->boundary.xmax * scale + translationX); x = x + 1){
		for (int y = (currTriangle->boundary.ymin * scale + translationY); y < (currTriangle->boundary.ymax * scale + translationY); y = y + 1){

			Vertex3D currPoint((x - translationX)/scale, (y - translationY)/scale, INT32_MIN);
			
			// Compute the barycentric coordinates:
			a = area(currPoint, currTriangle->B, currTriangle->C)/totalArea;
			b = area(currTriangle->A, currPoint, currTriangle->C)/totalArea;
			c = area(currTriangle->A, currTriangle->B, currPoint)/totalArea;

			// The point is inside of the triangle if a, b, and c are all between 0 and 1:
			if (isBetween0and1(a) && isBetween0and1(b) && isBetween0and1(c)){

				// if inside:
				// compute pixel’s z-coordinate 
				currPoint.z = ((currTriangle->A.z * a) + (currTriangle->B.z * b) + (currTriangle->C.z * c));
				 
				// compute pixel’s RBG color
				currPoint.r = 255 * (double)(currPoint.z - zmin)/(double)(zmax-zmin);
				
				currPoint.g = 0;
				currPoint.b = 0;

				// get the max and min values at the zbuffer, and then scale appropriately to 0-255
				//	if z > current z from Zbuff:
				if (currPoint.z > zBuffer[x][y]){
					// write RBG color to Fbuff
					Color c(currPoint.r, currPoint.g, currPoint.b); 
					frameBuffer[x][y] = c;
                    
					// write z-coordinate to Zbuff
					zBuffer[x][y] = currPoint.z;
					image->setPixel(x, y, currPoint.r, currPoint.g, currPoint.b);
				}
			}
		}	
	}
}

// Task 6: Normal Coloring
void performTask6(shared_ptr<Image> &image, int width, int height, double scale, double translationX, double translationY, int i, Triangle* currTriangle, Color** frameBuffer, double** zBuffer){
	double totalArea = currTriangle->getTotalArea();
	double a, b, c;

	// for all pixels in the triangle bounding box:
	for (int x = (currTriangle->boundary.xmin * scale + translationX); x < (currTriangle->boundary.xmax * scale + translationX); x = x + 1){
		for (int y = (currTriangle->boundary.ymin * scale + translationY); y < (currTriangle->boundary.ymax * scale + translationY); y = y + 1){

			Vertex3D currPoint((x - translationX)/scale, (y - translationY)/scale, INT32_MIN);
			
			// Compute the barycentric coordinates:
			a = area(currPoint, currTriangle->B, currTriangle->C)/totalArea;
			b = area(currTriangle->A, currPoint, currTriangle->C)/totalArea;
			c = area(currTriangle->A, currTriangle->B, currPoint)/totalArea;

			// The point is inside of the triangle if a, b, and c are all between 0 and 1:
			if (isBetween0and1(a) && isBetween0and1(b) && isBetween0and1(c)){

				// if inside:
				// compute pixel’s RBG color
				// when coloring the pixels, interpolate the normals of the three vertices of the 
				// triangle to compute the normal of the pixel.
				currPoint.nor.x = a * currTriangle->A.nor.x + b * currTriangle->B.nor.x + c * currTriangle->C.nor.x;
				currPoint.nor.y = a * currTriangle->A.nor.y + b * currTriangle->B.nor.y + c * currTriangle->C.nor.y;
				currPoint.nor.z = a * currTriangle->A.nor.z + b * currTriangle->B.nor.z + c * currTriangle->C.nor.z;

				currPoint.r = 255 * (0.5 * currPoint.nor.x + 0.5);
				currPoint.g = 255 * (0.5 * currPoint.nor.y + 0.5);
				currPoint.b = 255 * (0.5 * currPoint.nor.z + 0.5);

				// compute pixel’s z-coordinate 
				currPoint.z = ((currTriangle->A.z * a) + (currTriangle->B.z * b) + (currTriangle->C.z * c));

				//	if z > current z from Zbuff:
				if (currPoint.z > zBuffer[x][y]){
					// write RBG color to Fbuff
					frameBuffer[x][y] = Color(currPoint.r, currPoint.g, currPoint.b);

					// write z-coordinate to Zbuff
					zBuffer[x][y] = currPoint.z;
					image->setPixel(x, y, currPoint.r, currPoint.g, currPoint.b);
				}
			}
		}	
	}
}

const double oneOverSqrt3 = 1.0/sqrt(3);

// Task 7: Simple lighting
void performTask7(shared_ptr<Image> &image, int width, int height, double scale, double translationX, double translationY, int i, Triangle* currTriangle, Color** frameBuffer, double** zBuffer){
	double totalArea = currTriangle->getTotalArea();
	double a, b, c;

	// for all pixels in the triangle bounding box:
	for (int x = (currTriangle->boundary.xmin * scale + translationX); x < (currTriangle->boundary.xmax * scale + translationX); x = x + 1){
		for (int y = (currTriangle->boundary.ymin * scale + translationY); y < (currTriangle->boundary.ymax * scale + translationY); y = y + 1){

			Vertex3D currPoint((x - translationX)/scale, (y - translationY)/scale, INT32_MIN);
			
			// Compute the barycentric coordinates:
			a = area(currPoint, currTriangle->B, currTriangle->C)/totalArea;
			b = area(currTriangle->A, currPoint, currTriangle->C)/totalArea;
			c = area(currTriangle->A, currTriangle->B, currPoint)/totalArea;

			// The point is inside of the triangle if a, b, and c are all between 0 and 1:
			if (isBetween0and1(a) && isBetween0and1(b) && isBetween0and1(c)){

				// if inside:
				// compute pixel’s RBG color

				// when coloring the pixels, interpolate the normals of the three vertices of the 
				// triangle to compute the normal of the pixel.
				currPoint.nor.x = a * currTriangle->A.nor.x + b * currTriangle->B.nor.x + c * currTriangle->C.nor.x;
				currPoint.nor.y = a * currTriangle->A.nor.y + b * currTriangle->B.nor.y + c * currTriangle->C.nor.y;
				currPoint.nor.z = a * currTriangle->A.nor.z + b * currTriangle->B.nor.z + c * currTriangle->C.nor.z;

				double dotProduct = max(oneOverSqrt3 * (currPoint.nor.x + currPoint.nor.y + currPoint.nor.z), 0.0);
				currPoint.changeColor(255 * dotProduct, 255 * dotProduct, 255 * dotProduct);

				// compute pixel’s z-coordinate 
				currPoint.z = ((currTriangle->A.z * a) + (currTriangle->B.z * b) + (currTriangle->C.z * c));

				//	if z > current z from Zbuff:
				if (currPoint.z > zBuffer[x][y]){
					// write RBG color to Fbuff
					frameBuffer[x][y] = Color(currPoint.r, currPoint.g, currPoint.b);

					// write z-coordinate to Zbuff
					zBuffer[x][y] = currPoint.z;
					image->setPixel(x, y, currPoint.r, currPoint.g, currPoint.b);
				}
			}
		}	
	}
}



/*********** STARTER CODE ***********/


int main(int argc, char **argv)
{
	if(argc < 2) {
		std::cout << "Usage: A1 meshfile" << endl;
		return 0;
	}
	string meshName(argv[1]);

	// Load geometry
	vector<float> posBuf; // list of vertex positions
	vector<float> norBuf; // list of vertex normals
	vector<float> texBuf; // list of vertex texture coords
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	string errStr;
	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if(!rc) {
		cerr << errStr << endl;
	} else {
		// Some OBJ files have different indices for vertex positions, normals,
		// and texture coordinates. For example, a cube corner vertex may have
		// three different normals. Here, we are going to duplicate all such
		// vertices.
		// Loop over shapes
		for(size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces (polygons)
			size_t index_offset = 0;
			for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = shapes[s].mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for(size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+0]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+1]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+2]);
					if(!attrib.normals.empty()) {
						norBuf.push_back(attrib.normals[3*idx.normal_index+0]);
						norBuf.push_back(attrib.normals[3*idx.normal_index+1]);
						norBuf.push_back(attrib.normals[3*idx.normal_index+2]);
					}
					if(!attrib.texcoords.empty()) {
						texBuf.push_back(attrib.texcoords[2*idx.texcoord_index+0]);
						texBuf.push_back(attrib.texcoords[2*idx.texcoord_index+1]);
					}
				}
				index_offset += fv;
				// per-face material (IGNORE)
				shapes[s].mesh.material_ids[f];
			}
		}
	}

	/****************** NEW CODE ******************/

	// Add a command line argument to accept the following command line arguments:

	// 	Input filename of the .obj file to rasterize. ALREADY done with the meshName in the starter code
	// 	Output image filename (should be png)
	string outputFile = argv[2];
	
	// 	Image width and Image height and open the image itself
	int width = stoi(argv[3]);
	int height = stoi(argv[4]);
	// Create the image. We're using a `shared_ptr`, a C++11 feature.
	auto image = make_shared<Image>(width, height);

	// 	Task number (1 through 7)
	int taskNo = stoi(argv[5]);

	// Make a bounding box for the ENTIRE IMAGE
	//  - This will allow us to get a universale scale and translation that applies for every triangle!
	double xmin, ymin, zmin, xmax, ymax, zmax, xcurr, ycurr, zcurr;

	for (int i = 0; i < posBuf.size(); i += 3){
		if (i == 0){
			xmin = xmax = posBuf[0];
			ymin = ymax = posBuf[1];
			zmin = zmax = posBuf[2];
		}else{
			xcurr = posBuf[i];
			ycurr = posBuf[i+1];
			zcurr = posBuf[i+2];

			if (xcurr < xmin){
				xmin = xcurr; 	
			}
			
			if (xcurr > xmax){
				xmax = xcurr;
			}

			if (ycurr < ymin){
				ymin = ycurr; 	
			}
			
			if (ycurr > ymax){
				ymax = ycurr;
			}

			if (zcurr < zmin){
				zmin = zcurr; 	
			}
			
			if (zcurr > zmax){
				zmax = zcurr;
			}
		}
	}

	// Knowing the minimum and maximum coordinates, create the bounding box:
	BoundingBox worldBox(xmin, ymin, xmax, ymax);

	// Now that we have the boundaries, we can compute:
	double scale = computeScale(worldBox, width, height); 
	double translationX = ((double)width)/2 - scale * worldBox.midPointX();
    double translationY = ((double)height)/2 - scale * worldBox.midPointY();

	vector<Triangle> triangles;
	int numTriangles = 0;
	for (int i = 0; i < posBuf.size(); i += 9){
		Triangle curr(posBuf, norBuf, i);
		
		numTriangles++;

		triangles.push_back(curr);
	}

	// The frame buffer stores the color of each pixel (rgb values) in the new image
	Color** frameBuffer;
	frameBuffer = new Color* [width];

	// The Z buffer stores the "current depth" or "Z value" of each pixel
	double** zBuffer = new double* [width];

	for (int i = 0; i < width; i++){
		// Note: the default constructor for color represents black
		frameBuffer[i] = new Color [height];
		zBuffer[i] = new double[height];

		// Set each element of the zbuffer array to the smallest possible integer
		for (int j = 0; j < height; j++){
			zBuffer[i][j] = -DBL_MAX;
		}
	}

	Triangle* currTriangle;
	
	// for all triangles: 
	for (int i = 0; i < triangles.size(); i++){
		currTriangle = &(triangles.at(i));

		switch (taskNo){
			case 1:
				performTask1(image, width, height, scale, translationX, translationY, i, currTriangle, frameBuffer, zBuffer);
				break;
			case 2:
				performTask2(image, width, height, scale, translationX, translationY, i, currTriangle, frameBuffer, zBuffer);
				break;
			case 3:
				performTask3(image, width, height, scale, translationX, translationY, i, currTriangle, frameBuffer, zBuffer);
				break;
			case 4:
				performTask4(worldBox, image, width, height, scale, translationX, translationY, i, currTriangle, frameBuffer, zBuffer);
				break;
			case 5:
				performTask5(image, width, height, scale, translationX, translationY, i, currTriangle, frameBuffer, zBuffer, zmin, zmax);
				break;
			case 6:
				performTask6(image, width, height, scale, translationX, translationY, i, currTriangle, frameBuffer, zBuffer);
				break;
			case 7:
				performTask7(image, width, height, scale, translationX, translationY, i, currTriangle, frameBuffer, zBuffer);
				break;
			default:
				break;
		}
	}

	// Write image to file
	image->writeToFile(outputFile);

	// Deleting frame and Z buffers:
	for (int i = 0; i < width; i++){
		delete [] frameBuffer[i];
		delete [] zBuffer[i];
	}

	delete [] frameBuffer;
	delete [] zBuffer;

	return 0;
}
