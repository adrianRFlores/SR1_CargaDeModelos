#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <string>
#include <glm/glm.hpp>

struct Face
{
    std::vector<std::array<int, 3>> vertexIndices;
};

bool loadOBJ(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<Face>& out_faces)
{
    std::ifstream file(path);
    if (!file)
    {
        std::cout << "Failed to open the file: " << path << std::endl;
        return false;
    }

    std::string line;
    std::istringstream iss;
    std::string lineHeader;
    glm::vec3 vertex;
    Face face;

    while (std::getline(file, line))
    {
        iss.clear();
        iss.str(line);
        iss >> lineHeader;

        if (lineHeader == "v")
        {
            iss >> vertex.x >> vertex.y >> vertex.z;
            out_vertices.push_back(vertex);
        }
        else if (lineHeader == "f")
        {
            std::array<int, 3> vertexIndices;
            while (iss >> lineHeader)
            {
              std::istringstream tokenstream(lineHeader);
              std::string token;
              std::array<int, 3> vertexIndices;

              // Read all three values separated by '/'
              for (int i = 0; i < 3; ++i) {
                  std::getline(tokenstream, token, '/');
                  vertexIndices[i] = std::stoi(token) - 1;
              }

              face.vertexIndices.push_back(vertexIndices);
            }
            out_faces.push_back(face);
            face.vertexIndices.clear();
        }
    }

    return true;
}