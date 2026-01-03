#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

using namespace std;

struct AABB {
    glm::vec3 boundsMin;
    glm::vec3 boundsMax;
};

struct CollisionInfo {
    bool IsColliding;
    glm::vec3 Overlaps;
    glm::vec3 Correction;
    int MinOverlap;
};

CollisionInfo checkCollision(AABB BoxA, AABB BoxB) {
    CollisionInfo info = { false, glm::vec3(0.0f) };
    if (BoxA.boundsMax.x < BoxB.boundsMin.x || BoxA.boundsMin.x > BoxB.boundsMax.x)
        return info;
    if (BoxA.boundsMax.y < BoxB.boundsMin.y || BoxA.boundsMin.y > BoxB.boundsMax.y)
        return info;
    if (BoxA.boundsMax.z < BoxB.boundsMin.z || BoxA.boundsMin.z > BoxB.boundsMax.z)
        return info;

    info.IsColliding = true;
    float overlapX = min(abs(BoxA.boundsMax.x - BoxB.boundsMin.x), abs(BoxB.boundsMax.x - BoxA.boundsMin.x));
    float overlapY = min(abs(BoxA.boundsMax.y - BoxB.boundsMin.y), abs(BoxB.boundsMax.y - BoxA.boundsMin.y));
    float overlapZ = min(abs(BoxA.boundsMax.z - BoxB.boundsMin.z), abs(BoxB.boundsMax.z - BoxA.boundsMin.z));
    
    info.Overlaps = glm::vec3(overlapX, overlapY, overlapZ);
    info.MinOverlap = overlapX < overlapY ? (overlapX < overlapZ ? 0 : 2) : (overlapY < overlapZ ? 1 : 2);

    glm::vec3 centerA = (BoxA.boundsMax + BoxA.boundsMin) * 0.5f;
    glm::vec3 centerB = (BoxB.boundsMax + BoxB.boundsMin) * 0.5f;
    glm::vec3 diff = centerA - centerB;

    if(info.MinOverlap == 0) {
        info.Correction.x = diff.x > 0 ? overlapX : -overlapX;
    } else if(info.MinOverlap == 1) {
        info.Correction.y = diff.y > 0 ? overlapY : -overlapY;
    } else if(info.MinOverlap == 2) {
        info.Correction.z = diff.z > 0 ? overlapZ : -overlapZ;
    }
    return info;
}

class Ray
{
public:
    Ray(const glm::vec3 &orig, const glm::vec3 &dir) : orig(orig), dir(dir)
    {
        invdir = glm::vec3(1/dir.x, 1/dir.y, 1/dir.z);
    }
    glm::vec3 orig, dir;
    glm::vec3 invdir;
};

bool ray_intersect(const Ray &r, AABB box) {

    float tmin, tmax, tymin, tymax, tzmin, tzmax;
    
    if (r.invdir.x >= 0) { 
        tmin = (box.boundsMin.x - r.orig.x) * r.invdir.x; 
        tmax = (box.boundsMax.x - r.orig.x) * r.invdir.x; 
    } 
    else { 
        tmin = (box.boundsMax.x - r.orig.x) * r.invdir.x; 
        tmax = (box.boundsMin.x - r.orig.x) * r.invdir.x; 
    }
    if (r.invdir.y >= 0) { 
        tymin = (box.boundsMin.y - r.orig.y) * r.invdir.y; 
        tymax = (box.boundsMax.y - r.orig.y) * r.invdir.y; 
    } 
    else { 
        tymin = (box.boundsMax.y - r.orig.y) * r.invdir.y; 
        tymax = (box.boundsMin.y - r.orig.y) * r.invdir.y; 
    }
    
    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;
    
    if (r.invdir.z >= 0) { 
        tzmin = (box.boundsMin.z - r.orig.z) * r.invdir.z; 
        tzmax = (box.boundsMax.z - r.orig.z) * r.invdir.z; 
    } 
    else { 
        tzmin = (box.boundsMax.z - r.orig.z) * r.invdir.z; 
        tzmax = (box.boundsMin.z - r.orig.z) * r.invdir.z; 
    }
    
    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;

    if (tmax < 0) return false;

    return true;
}