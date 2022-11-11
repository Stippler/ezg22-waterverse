#include "Sphere.h"


#define _USE_MATH_DEFINES
#include <cmath>
#define PI M_PI
#include <vector>

#include "TextureLoader.h"

Sphere::Sphere(float radius, int sectorCount, int stackCount, glm::vec3 position)
{
	model = glm::translate(model, position);

	textureDiffuse = TextureLoader::getTexture("assets/silver.jpg");
	// textureSpecular = TextureLoader::getTexture("assets/bricks_specular.dds");

	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> texCoords;

	float x, y, z, xy;							 // vertex position
	float nx, ny, nz, lengthInv = 1.0f / radius; // vertex normal
	float s, t;									 // vertex texCoord

	float sectorStep = 2 * PI / sectorCount;
	float stackStep = PI / stackCount;
	float sectorAngle, stackAngle;

	for (int i = 0; i <= stackCount; ++i)
	{
		stackAngle = PI / 2 - i * stackStep; // starting from pi/2 to -pi/2
		xy = radius * cosf(stackAngle);		 // r * cos(u)
		z = radius * sinf(stackAngle);		 // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for (int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep; // starting from 0 to 2pi

			// vertex position (x, y, z)
			x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);

			// normalized vertex normal (nx, ny, nz)
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			normals.push_back(nx);
			normals.push_back(ny);
			normals.push_back(nz);

			// vertex tex coord (s, t) range between [0, 1]
			s = (float)j / sectorCount;
			t = (float)i / stackCount;
			texCoords.push_back(s);
			texCoords.push_back(t);
		}
	}

	int k1, k2;
	for (int i = 0; i < stackCount; ++i)
	{
		k1 = i * (sectorCount + 1); // beginning of current stack
		k2 = k1 + sectorCount + 1;	// beginning of next stack

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if (i != (stackCount - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}

			// store indices for lines
			// vertical lines for all stacks, k1 => k2
			lineIndices.push_back(k1);
			lineIndices.push_back(k2);
			if (i != 0) // horizontal lines except 1st stack, k1 => k+1
			{
				lineIndices.push_back(k1);
				lineIndices.push_back(k1 + 1);
			}
		}
	}


	std::size_t i, j;
	std::size_t count = vertices.size();
	for (i = 0, j = 0; i < count; i += 3, j += 2)
	{
		interleavedVertices.push_back(vertices[i]);
		interleavedVertices.push_back(vertices[i + 1]);
		interleavedVertices.push_back(vertices[i + 2]);

		interleavedVertices.push_back(normals[i]);
		interleavedVertices.push_back(normals[i + 1]);
		interleavedVertices.push_back(normals[i + 2]);

		interleavedVertices.push_back(texCoords[j]);
		interleavedVertices.push_back(texCoords[j + 1]);
	}

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, (unsigned int)interleavedVertices.size() * sizeof(float), interleavedVertices.data(), GL_STATIC_DRAW);

	// copy index data to VBO
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // for index data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,		  // target
				 (unsigned int)indices.size()*sizeof(unsigned int),			  // data size, # of bytes
				 indices.data(),			  // ptr to index data
				 GL_STATIC_DRAW);				  // usage


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

void Sphere::draw(Shader *shader)
{
	shader->setMat4("model", model);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shader->ID, "texture_diffuse1"), 1);
	glBindTexture(GL_TEXTURE_2D, textureDiffuse);


	// glActiveTexture(GL_TEXTURE0);
	// glUniform1i(glGetUniformLocation(shader->ID, "texture_specular1"), 1);
	// glBindTexture(GL_TEXTURE_2D, textureDiffuse);

    // glActiveTexture(GL_TEXTURE0); // active proper texture unit before binding
    // glUniform1i(glGetUniformLocation(shader->ID, "texture_diffuse1");
    // glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES,                // primitive type
               indices.size(),          		// # of indices
               GL_UNSIGNED_INT,                 // data type
               0);
	glBindVertexArray(0);
}