#include "Scene.h"

void Scene::printMatrix(mat4 m){
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            cout << m[i][j] << " ";
        }
        cout << endl;
    }
}

mat4 Scene::makeScene7Matrix(int _sceneNo){
	if (_sceneNo == BUNNY_SCENE_1){
        mat4 I(1);
		return I;
	}

	auto M = std::make_shared<MatrixStack>();
    M->loadIdentity();
    M->translate(0.3f, -1.5f, 0.0f);
    M->rotate(radians(20.0f), 1, 0, 0);
    M->scale(1.5f, 1.5f, 1.5f);

    return M->topMatrix();
}

void Scene::loadGeometry(string &_meshName, vector<float> &posBuf, vector<float> &norBuf, vector<float> &texBuf){
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> _shapes;
    std::vector<tinyobj::material_t> materials;
    string errStr;
    bool rc = tinyobj::LoadObj(&attrib, &_shapes, &materials, &errStr, _meshName.c_str());
    if(!rc) {
        cerr << errStr << endl;
    } else {
        // Some OBJ files have different indices for vertex positions, normals,
        // and texture coordinates. For example, a cube corner vertex may have
        // three different normals. Here, we are going to duplicate all such
        // vertices.
        // Loop over shapes
        for(size_t s = 0; s < _shapes.size(); s++) {
            // Loop over faces (polygons)
            size_t index_offset = 0;
            for(size_t f = 0; f < _shapes[s].mesh.num_face_vertices.size(); f++) {
                size_t fv = _shapes[s].mesh.num_face_vertices[f];
                // Loop over vertices in the face.
                for(size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = _shapes[s].mesh.indices[index_offset + v];
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
                _shapes[s].mesh.material_ids[f];
            }
        }

    }
}

void Scene::loadScene(int _sceneNo){
    this->sceneNo = _sceneNo;
    if (_sceneNo == 1 || _sceneNo == 2 || _sceneNo == 8){
        Light l(-2.0f, 1.0f, 1.0f, 1.0f);

        // Make three shapes:

        Shape redSphere(vec3(-0.5f, -1.0f, 1.0f), 1.0f);
        redSphere.diffuse = vec3(1.0f, 0.0f, 0.0f);
        redSphere.specular = vec3(1.0f, 1.0f, 0.5f);
        redSphere.ambient = vec3(0.1, 0.1, 0.1);
        redSphere.s = 100.0f;

        Shape greenSphere(vec3(0.5f, -1.0f, -1.0f), 1.0f);
        greenSphere.diffuse = vec3(0.0f, 1.0f, 0.0f);
        greenSphere.specular = vec3(1.0f, 1.0f, 0.5f);
        greenSphere.ambient = vec3(0.1, 0.1, 0.1);
        greenSphere.s = 100.0f;

        Shape blueSphere(vec3(0.0f, 1.0f, 0.0f), 1.0f);
        blueSphere.diffuse = vec3(0.0f, 0.0f, 1.0f);
        blueSphere.specular = vec3(1.0f, 1.0f, 0.5f);
        blueSphere.ambient = vec3(0.1, 0.1, 0.1);
        blueSphere.s = 100.0f;

        shapes.push_back(redSphere);
        shapes.push_back(greenSphere);
        shapes.push_back(blueSphere);
        
        lights.push_back(l);
    }
    else if (_sceneNo == 3){

        Light l1(1.0f, 2.0f, 2.0f, 0.5f);
        Light l2(-1.0f, 2.0f, -1.0f, 0.5f);
        lights.push_back(l1);
        lights.push_back(l2);

        // Red ellipsoid
        auto M = std::make_shared<MatrixStack>();
        M->translate(0.5f, 0.0f, 0.5f);
        M->scale(0.5f, 0.6f, 0.2f);
        glm::mat4 E = M->topMatrix();

        Shape redEllipsoid(E, vec3(0.5f, 0.0f, 0.5f), 1.0f);
        redEllipsoid.diffuse = vec3(1.0f, 0.0f, 0.0f);
        redEllipsoid.specular = vec3(1.0f, 1.0f, 0.5f);
        redEllipsoid.ambient = vec3(0.1f, 0.1f, 0.1f);
        redEllipsoid.s = 100.0f;
        shapes.push_back(redEllipsoid);

        Shape greenSphere(vec3(-0.5f, 0.0f, -0.5f), 1.0f);
        greenSphere.diffuse = vec3(0.0f, 1.0f, 0.0f);
        greenSphere.specular = vec3(1.0f, 1.0f, 0.5f);
        greenSphere.ambient = vec3(0.1f, 0.1f, 0.1f);
        greenSphere.s = 100.0f;
        shapes.push_back(greenSphere);

        Shape plane(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
        plane.diffuse = vec3(1.0f, 1.0f, 1.0f);
        plane.specular = vec3(0.0f, 0.0f, 0.0f);
        plane.ambient = vec3(0.1f, 0.1f, 0.1f);
        plane.s = 0.0f;

        shapes.push_back(plane);
    }
    else if (_sceneNo == 4 || _sceneNo == 5 || _sceneNo == 9){
    	Light l1(-1.0f, 2.0f, 1.0f, 0.5f);
        lights.push_back(l1);
        Light l2(0.5f, -0.5f, 0.0f, 0.5f);
        lights.push_back(l2);

        // Red sphere:
        Shape redSphere(vec3(0.5f, -0.7f, 0.5f), 0.3);
        redSphere.diffuse = vec3(1.0f, 0.0f, 0.0f);
        redSphere.specular = vec3(1.0f, 1.0f, 0.5f);
        redSphere.ambient = vec3(0.1f, 0.1f, 0.1f);
        redSphere.s = 100.0f;
        shapes.push_back(redSphere);

        // Blue sphere:
        Shape blueSphere(vec3(1.0f, -0.7f, 0.0f), 0.3f);
        blueSphere.diffuse = vec3(0.0f, 0.0f, 1.0f);
        blueSphere.specular = vec3(1.0f, 1.0f, 0.5f);
        blueSphere.ambient = vec3(0.1f, 0.1f, 0.1f);
        blueSphere.s = 100.0f;
        shapes.push_back(blueSphere);

        // Floor:
        Shape floor(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
        floor.diffuse = vec3(1.0f, 1.0f, 1.0f);
        floor.specular = vec3(0.0f, 0.0f, 0.0f);
        floor.ambient = vec3(0.1f, 0.1f, 0.1f);
        shapes.push_back(floor);

        // Back wall:
        Shape wall(vec3(0.0f, 0.0f, -3.0f), vec3(0.0f, 0.0f, 1.0f));
        wall.diffuse = vec3(1.0f, 1.0f, 1.0f);
        wall.specular = vec3(0.0f, 0.0f, 0.0f);
        wall.ambient = vec3(0.1f, 0.1f, 0.1f);
        shapes.push_back(wall);

        // Reflective sphere 1:
        Shape reflective1(vec3(-0.5f, 0.0f, -0.5f), 1.0f, true);
        if (sceneNo == 9){
            reflective1.diffuse = vec3(0.0f, 1.0f, 1.0f);
            reflective1.specular = vec3(1.0f, 0.5f, 0.5f);
            reflective1.ambient = vec3(0.1f, 0.1f, 0.1f);
            reflective1.s = 100.0f;
        }
        shapes.push_back(reflective1);

        // Reflective sphere 2:
        Shape reflective2(vec3(1.5f, 0.0f, -1.5f), 1.0f, true);
        if (sceneNo == 9){
            reflective2.diffuse = vec3(0.45f, 0.0f, 1.0f);
            reflective2.specular = vec3(0.5f, 1.0f, 0.5f);
            reflective2.ambient = vec3(0.1f, 0.1f, 0.1f);
            reflective2.s = 100.0f;
        }
        shapes.push_back(reflective2);

    }
    else if (_sceneNo == 6 || _sceneNo == 7){

        //Load geometry
        vector<float> posBuf; // list of vertex positions
        vector<float> norBuf; // list of vertex normals
        vector<float> texBuf; // list of vertex texture coords
        string meshName = "../resources/bunny.obj";
        loadGeometry(meshName, posBuf, norBuf, texBuf);

        E = makeScene7Matrix(_sceneNo);
        sceneNo = _sceneNo;

        if (_sceneNo == BUNNY_SCENE_1){
            Light l(-1.0f, 1.0f, 1.0f, 1.0f);
            lights.push_back(l);	
        }else{
            Light l(1.0f, 1.0f, 2.0f, 1.0f);
            lights.push_back(l);	
        }


        float xMin, xMax, yMin, yMax, zMin, zMax;

        // Add all the triangles
        for (int i = 0; i < posBuf.size(); i = i+9){
            
            if (i == 0){
                xMin = posBuf[0];
                xMax = posBuf[0];

                yMin = posBuf[1];
                yMax = posBuf[1];

                zMin = posBuf[2];
                zMax = posBuf[2];
            }

            Shape tri(posBuf, norBuf, i);
            tri.diffuse = vec3(0.0f, 0.0f, 1.0f);
            tri.specular = vec3(1.0f, 1.0f, 0.5f);
            tri.ambient = vec3(0.1f, 0.1f, 0.1f);
            tri.s = 100.0f;
            shapes.push_back(tri);

            for (int j = 0; j < 3; j++){
                if (posBuf[i * j] < xMin){
                    xMin = posBuf[i * j];
                }else if (posBuf[i * j] > xMax){
                    xMax = posBuf[i * j];
                }

                if (posBuf[(i+1) * j] < yMin){
                    yMin = posBuf[(i+1) * j];
                }else if (posBuf[(i+1) * j] > yMax){
                    yMax = posBuf[(i+1) * j];
                }

                if (posBuf[(i+2) * j] < zMin){
                    zMin = posBuf[(i+2) * j];
                }else if (posBuf[(i+2) * j] > zMax){
                    zMax = posBuf[(i+2) * j];
                }
            }
        }

        vec3 min(xMin, yMin, zMin);
        vec3 max(xMax, yMax, zMax);
        vec3 center = (max + min) * 0.5f;

        this->sphereCenter = inverse(E) * vec4(center, 1.0f);

        float _radius = std::max(abs(zMax-zMin), std::max(abs(xMax-xMin), abs(yMax-yMin)))/2.0f;
        
        if (sceneNo == BUNNY_SCENE_2){
            // The radius should be scaled by 1.5:
            _radius *= 1.5f;
        }

        this->radius = _radius;
    }
}

// v is the NORMALIZED DIRECTION pointing to the source of the ray
// n is the normal of the hit
vec3 Scene::getReflectedRay(vec3 view, vec3 normal){
    return (2.0f * dot(view, normal) * normal) - view;
}

// Return a vector containing all hits given a ray
vector<Hit> Scene::getAllHits(vec3 rayDir, vec3 rayOrigin, float maxDist){

    vector<Hit> allHits;

    double t, u, v, w;
    
    for (int i = 0; i < shapes.size(); i++){

        int type = shapes.at(i).type;

        if (type == TRIANGLE){
            double rayOriginBuff[3] = {(double)rayOrigin.x, (double)rayOrigin.y, (double)rayOrigin.z};
            double rayDirBuff[3] = {(double)rayDir.x, (double)rayDir.y, (double)rayDir.z};

            bool hitTriangle = intersect_triangle3(rayOriginBuff, rayDirBuff, shapes.at(i).vert0, shapes.at(i).vert1, shapes.at(i).vert2, &t, &u, &v);

            if (hitTriangle){
                
                if (t < 0.0){
                    continue;
                }

                w = 1.0 - u - v;

                vec3 pointHit = ((float)w * vec3(shapes.at(i).vert0[0], shapes.at(i).vert0[1], shapes.at(i).vert0[2])) + 
                ((float)u * vec3(shapes.at(i).vert1[0], shapes.at(i).vert1[1], shapes.at(i).vert1[2])) + 
                ((float)v * vec3(shapes.at(i).vert2[0], shapes.at(i).vert2[1], shapes.at(i).vert2[2]));

                vec3 normal = normalize(((float)w * shapes.at(i).nor0) + ((float)u * shapes.at(i).nor1) + ((float)v * shapes.at(i).nor2));
                
                Hit tHit(pointHit, normal, t);
                tHit.shapeHit = &shapes.at(i);
                allHits.push_back(tHit);
                
            }
        }

        else if(type == PLANE){

            Hit pHit = getPlaneIntersection(rayOrigin, rayDir, shapes.at(i).p_point, shapes.at(i).p_normal);
            
            if (pHit.t <= maxDist && pHit.t > 0.0f){

                pHit.shapeHit = &shapes.at(i);

                allHits.push_back(pHit);
                
            } 
        }

        else if (type == SPHERE){
            vector<Hit> sphereHits = getSphereIntersections(rayOrigin, rayDir, shapes.at(i).radius, shapes.at(i).center);
            for (int j = 0; j < sphereHits.size(); j++){

                if (abs(sphereHits.at(j).t) <= maxDist && sphereHits.at(j).t > 0.0f){

                    sphereHits.at(j).shapeHit = &shapes.at(i);

                    allHits.push_back(sphereHits.at(j));
                    
                }   
            }
        }

        else if (type == ELLIPSOID){

            vector<Hit> sphereHits = getEllipsoidIntersections(shapes.at(i).E, rayOrigin, rayDir);
            
            
            for (int j = 0; j < sphereHits.size(); j++){

                if (abs(sphereHits.at(j).t) <= maxDist && sphereHits.at(j).t > 0.0f){

                    sphereHits.at(j).shapeHit = &shapes.at(i);

                    allHits.push_back(sphereHits.at(j));
                    
                }   
            }
        }
    }

    return allHits;
}

// returns true if the xyz values all within 0.01 units of each other. 
bool Scene::samePos(vec3 pos1, vec3 pos2){
    bool sameX = abs(pos1.x - pos2.x) < 0.001f;
    bool sameY = abs(pos1.y - pos2.y) < 0.001f;
    bool sameZ = abs(pos1.z - pos2.z) < 0.001f;

    if (sameX && sameY && sameZ){
        return true;
    }

    return false;
}

// Given an array of hits and the origin of a ray, it will find the hit closest to the origin
Hit Scene::getNearestHit(vector<Hit> &allHits, vec3 rayOrigin){
    Hit h;
    h.t = INFINITY;
    for (int i = 0; i < allHits.size(); i++){
        if (abs(allHits.at(i).t) < abs(h.t) && !samePos(allHits.at(i).x, rayOrigin)){
            h = allHits.at(i);
        }
    }

    return h;
}

// Caculates the specular and diffuse colors given a Hit, camera positioin, and a specific light
vec3 Scene::specularAndDiffuse(Hit hit, vec3 camPos, int lightIndex){
    vec3 n = hit.n;

    vec3 c_D = vec3(0,0,0);
    vec3 c_S = vec3(0,0,0);

    vec3 color = vec3(0,0,0);

    vec3 eye = normalize(camPos - hit.x);

    // Diffuse component:
    vec3 lightVec = normalize(lights.at(lightIndex).pos - hit.x);
    c_D = c_D + hit.shapeHit->diffuse * std::max(0.0f, dot(lightVec, n));

    // Specular Component:
    vec3 h = normalize(lightVec + eye);
    c_S = c_S + hit.shapeHit->specular * pow(std::max(0.0f, dot(h, n)), hit.shapeHit->s);

    color = color + lights.at(lightIndex).col * (c_D + c_S);

    return color;
}

vec3 normalizeColors(vec3 color){
    if ( color.r > 1.0f){
         color.r = 1.0f;
    }

    if ( color.g > 1.0f){
         color.g = 1.0f;
    }
    
    if ( color.b > 1.0f){
         color.b = 1.0f;
    }


    if ( color.r < 0.0f){
         color.r = 0.0f;
    }

    if ( color.g < 0.0f){
         color.g = 0.0f;
    }
    
    if ( color.b < 0.0f){
         color.b = 0.0f;
    }

    return color;
}

vec3 Scene::blinnPhong(Hit nearestHit, vec3 rayOrigin){
    nearestHit.color = nearestHit.shapeHit->ambient;
        
    for (int i = 0; i < lights.size(); i++){
        // Check if we aren't in a shadow!
        
        // Compute light direction:

        vector<Hit> shadowHits;
        Hit nearShadow;
        
        if (sceneNo == BUNNY_SCENE_2){
            // Convert the shadow ray to local coords and find hits:
            vec3 shadowRayDir = vec3(inverse(E) * vec4((lights.at(i).pos - nearestHit.x), 0.0f));
            float maxDist = length(shadowRayDir);
            shadowRayDir = normalize(shadowRayDir);

            shadowHits = getAllHits(shadowRayDir, inverse(E) * vec4(nearestHit.x, 1.0f), maxDist);

            nearShadow = getNearestHit(shadowHits, inverse(E) * vec4(nearestHit.x, 1.0f));
        }
        else{
            vec3 shadowRayDir = lights.at(i).pos - nearestHit.x;
            float maxDist = length(shadowRayDir);
            shadowRayDir = normalize(shadowRayDir);
            shadowHits = getAllHits(shadowRayDir, nearestHit.x, maxDist);

            nearShadow = getNearestHit(shadowHits, nearestHit.x);
        }

        if (nearShadow.t == INFINITY){
            nearestHit.color += specularAndDiffuse(nearestHit, rayOrigin, i);
        }

    }
    
    return normalizeColors(nearestHit.color);
}


void transformToWorld(vector<Hit> &allHits, mat4 &E, vec3 rayOrigin_w, vec3 rayDir_w){
    mat4 invE = inverse(E);
    mat4 invTrE = transpose(invE);

    for (int i = 0; i < allHits.size(); i++){
        allHits.at(i).x = E * vec4(allHits.at(i).x, 1.0f);
        allHits.at(i).n = normalize(vec3(invTrE * vec4(allHits.at(i).n, 0.0f)));
        allHits.at(i).t = length(allHits.at(i).x - rayOrigin_w);

        if (dot(rayDir_w, allHits.at(i).x - rayOrigin_w) < 0.0f){
            allHits.at(i).t *= -1.0f;
        }

        if (allHits.at(i).t < 0.0f){
            // Remove it!
            allHits.erase(allHits.begin() + i);
            i--;
        }
    }
}

vec3 Scene::computePixelColor(vec3 rayDir, vec3 rayOrigin, int depth){

    vec3 rayDir_local, rayOrigin_local;
    vector<Hit> allHits;

    if (this->sceneNo == BUNNY_SCENE_1 || this->sceneNo == BUNNY_SCENE_2){

        mat4 invE = inverse(E);
        rayOrigin_local = invE * vec4(rayOrigin, 1.0f);
        rayDir_local = normalize(vec3(invE * vec4(rayDir, 0.0f)));

        //Check if there is an intersection with the sphere surrounding the bunny
        vector<Hit> sphereHits = getSphereIntersections(rayOrigin_local, rayDir_local, this->radius, this->sphereCenter);

        transformToWorld(sphereHits, E, rayOrigin, rayDir);

        bool hitsInFront = false;
        for (int i = 0; i < sphereHits.size(); i++){
            if (sphereHits.at(i).t > 0.0f){
                hitsInFront = true;
                break;
            }
        }

        if (!hitsInFront || sphereHits.size() == 0){
            return BACKGROUND_COLOR;
        }

        allHits = getAllHits(rayDir_local, rayOrigin_local, INFINITY);
        transformToWorld(allHits, E, rayOrigin, rayDir);

    }else{
        allHits = getAllHits(rayDir, rayOrigin, INFINITY);
    }

    Hit nearestHit = getNearestHit(allHits, rayOrigin);

    if (nearestHit.t != INFINITY){ // We hit something. 

        if (nearestHit.shapeHit->isReflective){ // Check if the object is reflective
            // Get the reflected direction:
            vec3 reflect = getReflectedRay(-1.0f * rayDir, nearestHit.n);

            // We have the reflected ray direction. Now let's check if we hit anything:
            vector<Hit> reflectedHits = getAllHits(reflect, nearestHit.x, INFINITY);
            Hit nearestReflectedHit = getNearestHit(reflectedHits, nearestHit.x);

            if (nearestReflectedHit.t != INFINITY && depth < MAX_DEPTH){
                if (!nearestReflectedHit.shapeHit->isReflective){
                    if (sceneNo == 9){
                        return normalizeColors(blinnPhong(nearestHit, rayOrigin) * 0.7f + blinnPhong(nearestReflectedHit, nearestHit.x) * 0.3f);
                    }else{
                        return blinnPhong(nearestReflectedHit, nearestHit.x);
                    }
                }else{
                    // Recursion time!
                    if (sceneNo == 9){
                        return normalizeColors((blinnPhong(nearestHit, rayOrigin) * 0.7f + computePixelColor(reflect, nearestHit.x, depth + 1) * 0.3f));
                    }else{
                        return computePixelColor(reflect, nearestHit.x, depth + 1);
                    }
                }
            }else{
                return normalizeColors((blinnPhong(nearestHit, rayOrigin) * 0.7f) + BACKGROUND_COLOR);
            }
        }
        return blinnPhong(nearestHit, rayOrigin);

    }

    return BACKGROUND_COLOR;
}