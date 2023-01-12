#include "MyTextureLoader.h"
#include "FileWatcher.h"

#include <fstream>
#include <string>

namespace MyTextureLoader
{
    std::unordered_map<std::string, unsigned int> textures;
    bool reloadTexture = false;
    std::vector<float> times;
    std::vector<std::string> cubeMaps;

    void reload()
    {
        std::ifstream file;
        file.open("assets/background.txt");

        int count;
        file >> count;

        std::vector<std::string> cubeMaps_;
        std::vector<float> times_;
        for (int i = 0; i < count; i++)
        {
            float time;
            file >> time;
            std::string cubemap;
            file >> cubemap;
            cubeMaps_.push_back(cubemap);
            times_.push_back(time);
        }
        times=times_;
        cubeMaps=cubeMaps_;
        file.close();
    }

    void init()
    {
        std::vector<std::string> faces = {
            "assets/day/right.jpeg",
            "assets/day/left.jpeg",
            "assets/day/top.jpeg",
            "assets/day/bottom.jpeg",
            "assets/day/front.jpeg",
            "assets/day/back.jpeg"};
        textures.emplace("day", loadCubemap(faces));
        faces = {
            "assets/night/right.jpeg",
            "assets/night/left.jpeg",
            "assets/night/top.jpeg",
            "assets/night/bottom.jpeg",
            "assets/night/front.jpeg",
            "assets/night/back.jpeg"};
        textures.emplace("night", loadCubemap(faces));
        FileWatcher::add("assets/background.txt", [&]
                         { reloadTexture = true; });
        reload();
    }

    unsigned int getTexture(std::string path)
    {
        unsigned int texture;
        auto entry = textures.find(path);
        if (entry == textures.end())
        {
            glGenTextures(1, &texture);
            // glBindTexture(GL_TEXTURE_2D, texture);
            //  set the texture wrapping/filtering options (on the currently bound texture object)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // load and generate the texture
            int width, height, nrChannels;
            unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else
            {
                std::cerr << "Failed to load texture" << std::endl;
                throw std::exception();
            }
            stbi_image_free(data);
        }
        else
        {
            texture = entry->second;
        }
        return texture;
    }

    unsigned int getCubemap(float timer)
    {
        if (reloadTexture)
        {
            reload();
        }
        int i = 0;
        float count = times[0];
        while (timer > count)
        {
            if (i + 1 == times.size())
            {
                break;
            }
            i += 1;
            count += times[i];
        }
        return textures.at(cubeMaps[i]);
    }

    unsigned int loadCubemap(std::vector<std::string> faces)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return textureID;
    }

}