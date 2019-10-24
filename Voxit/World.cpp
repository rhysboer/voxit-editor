#include "World.h"

Chunk* World::chunks[5][5][5];
Shader* World::shader = nullptr;
ShadowMapping* World::shadow = nullptr;
unsigned int World::totalVoxels = 0;


void World::InitWorld() {
	shader = ShaderManager::GetShader("voxel");
	shadow = new ShadowMapping(1024*2, 1024*2);
	totalVoxels = 0;

	for(int y = 0; y < worldSize; y++) {
		for(int z = 0; z < worldSize; z++) {
			for(int x = 0; x < worldSize; x++) {
				chunks[x][y][z] = new Chunk(glm::vec3(
					Chunk::size * (x - (worldSize / 2)),
					Chunk::size * y,
					Chunk::size * (z - (worldSize / 2))),
					glm::ivec3(x, y, z)
				);
			}
		}
	}
}

void World::DestroyWorld() {
	// DESTROY EVERYTHING
}

void World::AddBlocks(const std::vector<Voxel>& voxels) {
	for(int i = 0; i < voxels.size(); i++) {
		glm::ivec3 chunkIndex = GetChunkIndex(voxels[i].position);
		
		// Out of bounds
		if(chunkIndex.x < 0 || chunkIndex.x >= worldSize || chunkIndex.y < 0 || chunkIndex.y >= worldSize || chunkIndex.z < 0 || chunkIndex.z >= worldSize) {
			continue;
		}

		Chunk* chunk = chunks[chunkIndex.x][chunkIndex.y][chunkIndex.z];
		Voxel* vox = new Voxel(voxels[i]);

		// Check if voxel is a border
		// LEFT
		if((int)vox->position.x == chunk->Position().x - (int)Chunk::size / 2) { 
			if(chunkIndex.x - 1 >= 0) {
				chunks[chunkIndex.x - 1][chunkIndex.y][chunkIndex.z]->SetDirty(true);
			}
		}
		// RIGHT
		if((int)vox->position.x == chunk->Position().x + (int)Chunk::size / 2) {
			if(chunkIndex.x + 1 < worldSize) {
				chunks[chunkIndex.x + 1][chunkIndex.y][chunkIndex.z]->SetDirty(true);
			}
		}
		// FORWARD (positive Z)
		if((int)vox->position.z == chunk->Position().z + (int)Chunk::size / 2) {
			if(chunkIndex.z + 1 < worldSize) {
				chunks[chunkIndex.x][chunkIndex.y][chunkIndex.z + 1]->SetDirty(true);
			}
		}
		// BACKWARDS (negative Z)
		if((int)vox->position.z == chunk->Position().z - (int)Chunk::size / 2) {
			if(chunkIndex.z - 1 >= 0) {
				chunks[chunkIndex.x][chunkIndex.y][chunkIndex.z - 1]->SetDirty(true);
			}
		}
		// UP
		if((int)vox->position.y == chunk->Position().y + (int)Chunk::size / 2) {
			if(chunkIndex.y + 1 < worldSize) {
				chunks[chunkIndex.x][chunkIndex.y + 1][chunkIndex.z]->SetDirty(true);
			}
		}
		// DOWN
		if((int)vox->position.y == chunk->Position().y - (int)Chunk::size / 2) {
			if(chunkIndex.y - 1 >= 0) {
				chunks[chunkIndex.x][chunkIndex.y - 1][chunkIndex.z]->SetDirty(true);
			}
		}

		if(chunk->AddVoxel(vox))
			++totalVoxels;
	}

	for(int y = 0; y < worldSize; y++) {
		for(int z = 0; z < worldSize; z++) {
			for(int x = 0; x < worldSize; x++) {
				if(chunks[x][y][z]->IsDirty()) {
					chunks[x][y][z]->RegenerateMesh();
					chunks[x][y][z]->SetDirty(false);
				}
			}
		}
	}
}

void World::RemoveBlocks(const std::vector<Voxel>& voxels) {
	for(int i = 0; i < voxels.size(); i++) {
		glm::ivec3 chunkIndex = GetChunkIndex(voxels[i].position);

		// Out of bounds
		if(chunkIndex.x < 0 || chunkIndex.x >= worldSize || chunkIndex.y < 0 || chunkIndex.y >= worldSize || chunkIndex.z < 0 || chunkIndex.z >= worldSize) {
			continue;
		}

		Chunk* chunk = chunks[chunkIndex.x][chunkIndex.y][chunkIndex.z];
		Voxel* voxel = chunk->RemoveVoxel(voxels[i].position);

		if(voxel != nullptr) {
			// Check if voxel is a border
			// LEFT
			if((int)voxels[i].position.x == chunk->Position().x - (int)Chunk::size / 2) {
				if(chunkIndex.x - 1 >= 0) {
					chunks[chunkIndex.x - 1][chunkIndex.y][chunkIndex.z]->SetDirty(true);
				}
			}
			// RIGHT
			if((int)voxels[i].position.x == chunk->Position().x + (int)Chunk::size / 2) {
				if(chunkIndex.x + 1 < worldSize) {
					chunks[chunkIndex.x + 1][chunkIndex.y][chunkIndex.z]->SetDirty(true);
				}
			}
			// FORWARD (positive Z)
			if((int)voxels[i].position.z == chunk->Position().z + (int)Chunk::size / 2) {
				if(chunkIndex.z + 1 < worldSize) {
					chunks[chunkIndex.x][chunkIndex.y][chunkIndex.z + 1]->SetDirty(true);
				}
			}
			// BACKWARDS (negative Z)
			if((int)voxels[i].position.z == chunk->Position().z - (int)Chunk::size / 2) {
				if(chunkIndex.z - 1 >= 0) {
					chunks[chunkIndex.x][chunkIndex.y][chunkIndex.z - 1]->SetDirty(true);
				}
			}
			// UP
			if((int)voxels[i].position.y == chunk->Position().y + (int)Chunk::size / 2) {
				if(chunkIndex.y + 1 < worldSize) {
					chunks[chunkIndex.x][chunkIndex.y + 1][chunkIndex.z]->SetDirty(true);
				}
			}
			// DOWN
			if((int)voxels[i].position.y == chunk->Position().y - (int)Chunk::size / 2) {
				if(chunkIndex.y - 1 >= 0) {
					chunks[chunkIndex.x][chunkIndex.y - 1][chunkIndex.z]->SetDirty(true);
				}
			}

			delete voxel;
			chunk->SetDirty(true);
			--totalVoxels;
		}
	}
}

void World::Draw() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadow->GetDepthMap());

	shader->SetMatrix4("_lightMatrix", shadow->GetDepthPV());
	shader->SetFloat("_showOutline", (float)Settings::useVoxelOutline);
	shader->SetFloat("_showLighting", (float)Settings::useSun);
	//shader->SetFloat("_showShadow", (float)Settings::useShadow);
	//shader->SetVector3("_lightDire", Settings::sunDirection);

	shader->SetVector3("_sunPosition", glm::normalize(shadow->GetPosition()));

	glm::vec3 pos = shadow->GetPosition();
	float radian = glm::radians(Time::DeltaTime() * 15.0f);// % 360;
	printf("%f\n", radian);
	float _z = pos.z * cos(radian) - pos.x * sin(radian);
	float _x = pos.z * sin(radian) + pos.x * cos(radian);



	shadow->SetPosition(glm::vec3(_x, pos.y, _z));

	shader->SetMatrix4("_view", Camera::ActiveCamera->View());
	shader->SetMatrix4("_projection", Camera::ActiveCamera->Projection());

	for(int x = 0; x < worldSize; x++) {
		for(int y = 0; y < worldSize; y++) {
			for(int z = 0; z < worldSize; z++) {
				Chunk* chunk = chunks[x][y][z];
				if(chunk != nullptr) {
					if(chunk->IsDirty()) {
						chunk->RegenerateMesh();
					}
					
					chunks[x][y][z]->DrawMesh(*shader);
				}
			}
		}
	}
}

void World::DepthMapDraw() {
	shadow->FrameBuffer_Start();
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	for(int x = 0; x < worldSize; x++) {
		for(int y = 0; y < worldSize; y++) {
			for(int z = 0; z < worldSize; z++) {
				chunks[x][y][z]->DrawMesh(*shadow->GetShader());
			}
		}
	}
	//glDisable(GL_CULL_FACE);
	shadow->FrameBuffer_End();
}

Chunk* const World::GetVoxelChunk(const glm::ivec3& voxelPos) {
	if(voxelPos.y < 0) return nullptr;
	glm::ivec3 chunkIndex = glm::ivec3(floor(((voxelPos.x + floor(Chunk::size / 2)) / Chunk::size) + worldSize / 2),
									   floor(voxelPos.y / Chunk::size),
									   floor(((voxelPos.z + floor(Chunk::size / 2)) / Chunk::size) + worldSize / 2));

	if(chunkIndex.x < 0 || chunkIndex.x >= worldSize || chunkIndex.y < 0 || chunkIndex.y >= worldSize || chunkIndex.z < 0 || chunkIndex.z >= worldSize)
		return nullptr;

	return chunks[chunkIndex.x][chunkIndex.y][chunkIndex.z];
}

Chunk* const World::GetChunk(const glm::ivec3 chunkIndex) {
	if(chunkIndex.x <= 0 || chunkIndex.x >= worldSize || chunkIndex.y <= 0 || chunkIndex.y >= worldSize || chunkIndex.z <= 0 || chunkIndex.z >= worldSize)
		return nullptr;
	return chunks[chunkIndex.x][chunkIndex.y][chunkIndex.z];
}

Voxel* World::GetVoxel(const glm::ivec3& position) {
	Chunk* const chunk = GetVoxelChunk(position);
	if(!chunk) return nullptr;

	return chunk->GetVoxel(position);;
}

bool World::HasNeighbour(const Voxel& voxel, Chunk::Direction direction) {
	Chunk* const chunk = GetVoxelChunk(voxel.position);
	if(!chunk) return false;

	return chunk->HasNeighbour(voxel.position, direction);
}

Voxel* World::GetAndRemoveVoxel(const glm::ivec3& position) {
	Chunk* const chunk = GetVoxelChunk(position);
	if(!chunk) return nullptr;

	glm::ivec3 chunkIndex = chunk->Index();

	// Check if voxel is a border
	// LEFT
	if(position.x == chunk->Position().x - (int)Chunk::size / 2) {
		if(chunkIndex.x - 1 >= 0) {
			chunks[chunkIndex.x - 1][chunkIndex.y][chunkIndex.z]->SetDirty(true);
		}
	}
	// RIGHT
	if(position.x == chunk->Position().x + (int)Chunk::size / 2) {
		if(chunkIndex.x + 1 < worldSize) {
			chunks[chunkIndex.x + 1][chunkIndex.y][chunkIndex.z]->SetDirty(true);
		}
	}
	// FORWARD (positive Z)
	if(position.z == chunk->Position().z + (int)Chunk::size / 2) {
		if(chunkIndex.z + 1 < worldSize) {
			chunks[chunkIndex.x][chunkIndex.y][chunkIndex.z + 1]->SetDirty(true);
		}
	}
	// BACKWARDS (negative Z)
	if(position.z == chunk->Position().z - (int)Chunk::size / 2) {
		if(chunkIndex.z - 1 >= 0) {
			chunks[chunkIndex.x][chunkIndex.y][chunkIndex.z - 1]->SetDirty(true);
		}
	}
	// UP
	if(position.y == chunk->Position().y + (int)Chunk::size / 2) {
		if(chunkIndex.y + 1 < worldSize) {
			chunks[chunkIndex.x][chunkIndex.y + 1][chunkIndex.z]->SetDirty(true);
		}
	}
	// DOWN
	if(position.y == chunk->Position().y - (int)Chunk::size / 2) {
		if(chunkIndex.y - 1 >= 0) {
			chunks[chunkIndex.x][chunkIndex.y - 1][chunkIndex.z]->SetDirty(true);
		}
	}

	Voxel* voxel = chunk->RemoveVoxel(position);
	if(voxel)
		--totalVoxels;

	return voxel;
}

int World::TotalBlocks() {
	return totalVoxels;
}

void World::GetAllVoxels(std::vector<Voxel*>& voxels) {
	for(int y = 0; y < worldSize; y++) {
		for(int z = 0; z < worldSize; z++) {
			for(int x = 0; x < worldSize; x++) {
				chunks[x][y][z]->GetVoxels(voxels);
			}
		}
	}
}

glm::ivec3 World::GetChunkIndex(glm::vec3 voxelPos) {
	return glm::ivec3(floor(((voxelPos.x + floor(Chunk::size / 2)) / Chunk::size) + worldSize / 2),
					  floor((voxelPos.y / Chunk::size)),
					  floor(((voxelPos.z + floor(Chunk::size / 2)) / Chunk::size) + worldSize / 2));
}