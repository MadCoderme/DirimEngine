#pragma once
#include <vector>
#include <cmath>
#include <queue>
#include <algorithm>
#include <glm/glm.hpp>
#include "levelloader.hpp"

using namespace std;

// Internal node structure for the grid
struct PathNode {
    int lastVisitedRunID = 0; // The optimization magic
    float gScore = FLT_MAX;
    int parentIndex = -1;
};

// PQ Element
struct PQNode {
    int index;
    float fCost;
    bool operator>(const PQNode& other) const { return fCost > other.fCost; }
};

class Pathfinder {
private:
    // Persistent memory to avoid allocations
    vector<PathNode> nodePool;
    // Priority Queue member (cleared, not reallocated)
    priority_queue<PQNode, vector<PQNode>, greater<PQNode>> openList;
    
    int currentRunID = 0;
    int width = 0;
    int height = 0;

public:
    // Call this ONCE after loading the level
    void Init(int mapWidth, int mapHeight) {
        width = mapWidth;
        height = mapHeight;
        nodePool.resize(width * height);
        // Reset pool
        for(auto& n : nodePool) n = PathNode();
        currentRunID = 0;
    }

    // Helper: 1D Index
    int ToIndex(int x, int y) { return y * width + x; }
    glm::vec3 GridToWorld(int x, int y, float tileSize) {
        return glm::vec3(x * tileSize, -0.5f, y * tileSize);
    }
    glm::ivec2 WorldToGrid(glm::vec3 pos, float tileSize) {
        return glm::ivec2(round(pos.x / tileSize), round(pos.z / tileSize));
    }

    // Returns a path vector. Note: This vector is created on stack/RVO, 
    // but the heavy lifting (grid data) is cached.
    vector<glm::vec3> FindPath(glm::vec3 startPos, glm::vec3 targetPos, LevelLoader& level) {
        if (width == 0) return {}; // Not initialized

        glm::ivec2 startNode = WorldToGrid(startPos, level.TILE_SIZE);
        glm::ivec2 targetNode = WorldToGrid(targetPos, level.TILE_SIZE);

        // Bounds checks
        if (startNode.x < 0 || startNode.x >= width || startNode.y < 0 || startNode.y >= height) return {};
        if (targetNode.x < 0 || targetNode.x >= width || targetNode.y < 0 || targetNode.y >= height) return {};
        if (level.mapLayout[targetNode.y][targetNode.x] == 'W') return {};

        int startIndex = ToIndex(startNode.x, startNode.y);
        int targetIndex = ToIndex(targetNode.x, targetNode.y);
        
        if (startIndex == targetIndex) return {};

        // --- OPTIMIZATION: Increment Run ID ---
        // This effectively "clears" the grid without iterating through it.
        currentRunID++; 
        
        // Ensure priority queue is empty from any previous weird state
        while(!openList.empty()) openList.pop();

        // Setup Start Node
        nodePool[startIndex].lastVisitedRunID = currentRunID;
        nodePool[startIndex].gScore = 0.0f;
        nodePool[startIndex].parentIndex = -1;
        
        openList.push({startIndex, 0.0f});

        const int dx[] = {0, 0, -1, 1};
        const int dy[] = {-1, 1, 0, 0};

        while (!openList.empty()) {
            PQNode top = openList.top();
            openList.pop();
            int currentIdx = top.index;

            // If we found the target
            if (currentIdx == targetIndex) break;

            // Lazy cleanup: If this node was added to PQ multiple times, 
            // process only the best one.
            if (top.fCost > nodePool[currentIdx].gScore + GetHeuristic(currentIdx, targetIndex)) {
               // continue; // Optional strict check
            }

            int cx = currentIdx % width;
            int cy = currentIdx / width;

            for (int i = 0; i < 4; i++) {
                int nx = cx + dx[i];
                int ny = cy + dy[i];

                if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
                if (level.mapLayout[ny][nx] == 'W') continue;

                int neighborIdx = ToIndex(nx, ny);
                float newG = nodePool[currentIdx].gScore + 1.0f;

                // Check if neighbor is "new" (from old run) or we found a shorter path
                bool isNew = nodePool[neighborIdx].lastVisitedRunID != currentRunID;
                
                if (isNew || newG < nodePool[neighborIdx].gScore) {
                    nodePool[neighborIdx].gScore = newG;
                    nodePool[neighborIdx].lastVisitedRunID = currentRunID; // Mark as visited in THIS run
                    nodePool[neighborIdx].parentIndex = currentIdx;
                    
                    float f = newG + GetHeuristic(neighborIdx, targetIndex);
                    openList.push({neighborIdx, f});
                }
            }
        }

        // Reconstruct
        if (nodePool[targetIndex].lastVisitedRunID != currentRunID || nodePool[targetIndex].parentIndex == -1) {
            return {}; // No path found
        }

        vector<glm::vec3> path;
        int curr = targetIndex;
        while (curr != startIndex) {
            int cx = curr % width;
            int cy = curr / width;
            path.push_back(GridToWorld(cx, cy, level.TILE_SIZE));
            curr = nodePool[curr].parentIndex;
        }
        reverse(path.begin(), path.end());
        return path;
    }

    float GetHeuristic(int idxA, int idxB) {
        int x1 = idxA % width; int y1 = idxA / width;
        int x2 = idxB % width; int y2 = idxB / width;
        return static_cast<float>(abs(x1 - x2) + abs(y1 - y2));
    }
};