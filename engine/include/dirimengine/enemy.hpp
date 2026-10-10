#pragma once
#include "gameobject.hpp"
#include "pathfinding.hpp"
#include "physics.hpp"
#include "levelloader.hpp"
#include <limits> // Required for infinity

// Forward declaration
class Scene; 

class Enemy : public GameObject {
public:
    // AI Variables
    vector<glm::vec3> currentPath;
    int pathIndex = 0;
    float pathTimer = 0.0f;
    float lastAttackTime = 0.0f;
    Pathfinder* pathfinderSystem = nullptr;
    
    // Logic State
    bool hasAggro = false;
    float aggroRange = 12.0f; // Increased range slightly since we now have wall checks
    float attackRange = 2.8f;
    float stopDistance = 1.5f;
    float pathUpdateRate = 0.5f;

    // References
    Scene* scene = nullptr;
    glm::vec3* targetPos = nullptr; 
    LevelLoader* loaderRef = nullptr;
    vector<GameObject*>* sceneObjects = nullptr; 
    int* targetHealth = nullptr; 

    Enemy(Model* m, Scene* sc) : GameObject(m) {
        this->tag = "enemy";
        this->type = TEXTURE_GEOMETRY;
        this->health = 100; 
        this->isAlive = true;
        this->scene = sc;
        pathTimer = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * pathUpdateRate;
    }

    void SetupAI(glm::vec3* playerPos, int* playerHP, LevelLoader* loader, vector<GameObject*>* objects, Pathfinder* pf) {
        targetPos = playerPos;
        targetHealth = playerHP;
        loaderRef = loader;
        sceneObjects = objects;
        pathfinderSystem = pf;
    }

    void Update(float deltaTime) override {
        if (!isAlive || targetPos == nullptr) return;

        float distToPlayer = glm::distance(this->position, *targetPos);
        if (distToPlayer > 40.0f) return;
        // 1. CHECK AGGRO
        // If we don't have aggro, we check if we can see the player.
        // Even if we DO have aggro, we keep checking LOS so the enemy 
        // doesn't try to shoot/attack through a wall if the player runs behind one.
        if (!hasAggro) {
            CheckForPlayer();
            return; 
        }

        // 2. ATTACK BEHAVIOR
        if (distToPlayer <= attackRange) {
            FaceTarget(*targetPos);
            
            if (glfwGetTime() - lastAttackTime > 1.0f) {
                // STRLICT CHECK: Only damage if we are facing player AND have Line of Sight
                if (HasLineOfSight()) { 
                     if (targetHealth) *targetHealth -= 10;
                     scene->audioSystem.Play2D(DIRIMENGINE_ASSET_ROOT "/audio/Melee1.wav", false);
                     // Trigger attack animation/sound here
                }
                lastAttackTime = glfwGetTime();
            }
        }
        // 3. CHASE BEHAVIOR
        else {
            pathTimer += deltaTime;
            
            // Recalculate path occasionally
            if ((pathTimer > pathUpdateRate || currentPath.empty()) && scene->pathfindRequestsThisFrame < scene->MAX_PATH_REQUESTS) {
                pathTimer = 0.0f;
                if (pathfinderSystem) {
                    scene->pathfindRequestsThisFrame++;
                    currentPath = pathfinderSystem->FindPath(this->position, *targetPos, *loaderRef);
                    
                    pathIndex = 0;
                    // Jitter Fix: Skip current tile
                    if (currentPath.size() > 1) {
                        float distToFirstNode = glm::distance(
                            glm::vec3(position.x, 0, position.z), 
                            glm::vec3(currentPath[0].x, 0, currentPath[0].z)
                        );
                        if (distToFirstNode < 1.0f) pathIndex = 1; 
                    }
                }
            }

            if (!currentPath.empty() && pathIndex < currentPath.size()) {
                glm::vec3 waypoint = currentPath[pathIndex];
                
                float distToWaypoint = glm::distance(
                    glm::vec3(position.x, 0, position.z), 
                    glm::vec3(waypoint.x, 0, waypoint.z)
                );

                if (distToWaypoint < 0.2f) {
                    pathIndex++;
                } else {
                    MoveTowards(waypoint, 0.8f, deltaTime); 
                }
            }
        }

        ResolveCollisions();
    }

    GameObject* Clone() override {
        Enemy* clone = new Enemy(this->model, this->scene);
        clone->type = this->type;
        clone->tag = this->tag;
        clone->scale = this->scale;
        clone->health = this->health;
        clone->isCollisionEnabled = this->isCollisionEnabled;
        clone->hasAggro = false; 
        return clone;
    }

private:
    void CheckForPlayer() {
        if (!targetPos) return;

        float dist = glm::distance(this->position, *targetPos);
        
        // Optimization: Only cast rays if player is close enough
        if (dist < aggroRange) {
            if (HasLineOfSight()) {
                hasAggro = true;
            }
        }
    }

    // --- NEW: RAYCASTING LOGIC ---
    bool HasLineOfSight() {
        if (!sceneObjects || !targetPos) return false;

        // Lift origin/target up by 0.5f so we don't trace rays along the floor
        // (Floor rays often get blocked by uneven geometry or self-collision)
        glm::vec3 origin = this->position; 
        origin.y += 0.5f; 
        
        glm::vec3 target = *targetPos;
        target.y += 0.5f;

        glm::vec3 dir = glm::normalize(target - origin);
        float distToPlayer = glm::distance(origin, target);

        // Check every object in the scene
        for (GameObject* obj : *sceneObjects) {
            // Only check walls/obstacles (ignore self, gun, and other enemies)
            if (obj->isCollisionEnabled && obj != this && obj->tag != "enemy" && obj->id != "gun") {
                
                float t = 0.0f;
                // Perform Ray-AABB intersection
                if (IntersectRayAABB(origin, dir, obj->getWorldAABB(), t)) {
                    // If we hit something, and it's CLOSER than the player
                    if (t < distToPlayer) {
                        return false; // Vision Blocked
                    }
                }
            }
        }
        return true; // Nothing blocked the view
    }

    // Standard Slab Method for Ray-AABB intersection
    // Returns true if hit, and stores distance in 't'
    bool IntersectRayAABB(glm::vec3 origin, glm::vec3 dir, AABB box, float &t) {
        float tmin = (box.boundsMin.x - origin.x) / dir.x;
        float tmax = (box.boundsMax.x - origin.x) / dir.x;

        if (tmin > tmax) swap(tmin, tmax);

        float tymin = (box.boundsMin.y - origin.y) / dir.y;
        float tymax = (box.boundsMax.y - origin.y) / dir.y;

        if (tymin > tymax) swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax)) return false;

        if (tymin > tmin) tmin = tymin;
        if (tymax < tmax) tmax = tymax;

        float tzmin = (box.boundsMin.z - origin.z) / dir.z;
        float tzmax = (box.boundsMax.z - origin.z) / dir.z;

        if (tzmin > tzmax) swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax)) return false;

        if (tzmin > tmin) tmin = tzmin;
        if (tzmax < tmax) tmax = tzmax;

        if (tmax < 0) return false; // Behind ray

        t = tmin; // Return distance to entry point
        return true;
    }

    bool CanSeePlayer(float viewAngleDeg) {
        glm::vec3 toPlayer = glm::normalize(*targetPos - this->position);
        toPlayer.y = 0; 
        
        // Calculate forward vector from rotation angle
        glm::vec3 forward;
        forward.x = cos(glm::radians(rotationAngle - 90.0f)); 
        forward.z = sin(glm::radians(rotationAngle - 90.0f));
        forward.y = 0;
        forward = glm::normalize(forward);

        float dotProd = glm::dot(forward, toPlayer);
        return dotProd > cos(glm::radians(viewAngleDeg));
    }

    void MoveTowards(glm::vec3 target, float speed, float dt) {
        glm::vec3 dir = glm::normalize(target - this->position);
        dir.y = 0; 
        
        if (glm::distance(glm::vec3(position.x, 0, position.z), glm::vec3(target.x, 0, target.z)) > 0.05f) {
            SetPosition(this->position + (dir * speed * dt));
            FaceTarget(target);
        }
    }

    void FaceTarget(glm::vec3 target) {
        glm::vec3 dir = target - this->position;
        dir.y = 0;
        if (glm::length(dir) > 0.01f) {
            float angle = -glm::degrees(atan2(dir.z, dir.x)) + 90.0f;
            
            if (abs(this->rotationAngle - angle) > 0.1f) {
                SetRotation(angle, glm::vec3(0.0f, 1.0f, 0.0f));
            }
        }
    }

    void ResolveCollisions() {
        if (sceneObjects) {
            for (GameObject* other : *sceneObjects) {
                if (other->isCollisionEnabled && other != this) {
                    
                    CollisionInfo col = checkCollision(this->getWorldAABB(), other->getWorldAABB());
                    if (col.IsColliding) {
                        glm::vec3 newPos = this->position;
                        newPos.x += col.Correction.x;
                        newPos.z += col.Correction.z;
                        SetPosition(newPos);
                    }
                }
            }
        }
    }
};