// RailManager.cpp



#include <iostream>
#include <string>

#include "../Framework/Utilities.hpp"
#include "../Rendering/Meshes/EntityBatch.hpp"

#include "RailManager.hpp"

namespace as {

//////////////////////////////////////////////////////////////////////////
RailManager::RailManager( Terrain *t )
:	inertia(0.0f),
	targetPosSet(false)
{
	std::list<Entity> railEntities = t->getEntitiesOfType(Entity::RAIL);
	std::list<Entity>::const_iterator it;
	for(it = railEntities.begin(); it != railEntities.end(); ++it) {
		rails[(*it).pos] = RT_XDIR;
	}

	xfactor = 1.0f;
	zfactor = 1.0f;

	initCartMesh();
}

//////////////////////////////////////////////////////////////////////////
void RailManager::determineRowColForPos(BlockPos pos, int *itsRow, int *itsCol) const {
	int row;
	int col = 10;

	RailType type = determineRailTypeForPos(pos);

	switch(type) {
	default:
	case RT_XDIR:
		row = 3;
		break;
	case RT_ZDIR:
		row = 2;
		break;
	case RT_NX_TO_NZ:
		row = 4;
		break;
	case RT_NX_TO_PZ:
		row = 7;
		break;
	case RT_PX_TO_NZ:
		row = 5;
		break;
	case RT_PX_TO_PZ:
		row = 6;
		break;
	}

	*itsRow = row;
	*itsCol = col;
}

RailManager::RailType RailManager::determineRailTypeForPos( BlockPos pos ) const {
	bool env[8];
	env[0] = isRailAtPos(pos.x-1, pos.y, pos.z-1);
	env[1] = isRailAtPos(pos.x, pos.y, pos.z-1);
	env[2] = isRailAtPos(pos.x+1, pos.y, pos.z-1);
	env[3] = isRailAtPos(pos.x-1, pos.y, pos.z);
	env[4] = isRailAtPos(pos.x+1, pos.y, pos.z);
	env[5] = isRailAtPos(pos.x-1, pos.y, pos.z+1);
	env[6] = isRailAtPos(pos.x, pos.y, pos.z+1);
	env[7] = isRailAtPos(pos.x+1, pos.y, pos.z+1);

	if(env[3] && env[4]) return RT_XDIR;
	if(env[1] && env[6]) return RT_ZDIR;

	if(env[3] && env[1]) return RT_PX_TO_NZ;
	if(env[4] && env[1]) return RT_NX_TO_NZ;

	if(env[6] && env[3]) return RT_PX_TO_PZ;
	if(env[6] && env[4]) return RT_NX_TO_PZ;

	if(env[3] ^ env[4]) return RT_XDIR;
	if(env[1] ^ env[6]) return RT_ZDIR;

	return RT_XDIR;
}

bool RailManager::isRailAtPos( int x, int y, int z ) const {
	static BlockPos p;
	p.x = x;
	p.y = y;
	p.z = z;
	return rails.find(p) != rails.end();
}

bool RailManager::isRailAtPos( BlockPos pos ) const
{
	return isRailAtPos(pos.x, pos.y, pos.z);
}

void RailManager::addRail( BlockPos pos ) {
	if(rails.find(pos) != rails.end()) // no dups
		return;

	// Determine rail type
	RailType ntype = RT_XDIR;
	rails[pos] = ntype;
}

bool RailManager::tryToRemoveRail( BlockPos pos )
{
	std::map<BlockPos, RailType>::iterator it = rails.find(pos);
	if(it == rails.end()) return false;
	rails.erase(it);

	if(rails.empty() || pos + BlockPos(0, -1, 0) == BlockPos(cart.pos)) {
		cart.active = false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Cart related
//////////////////////////////////////////////////////////////////////////

/*static float cartPosCoords[] = {
	// front side
	-0.5f, 1.0f, 1.0f,// 0
	0.0f, 0.0f, 0.8f, // 1
	0.0f, 1.0f, 1.0f, // 2
	1.0f, 1.0f, 1.0f, // 3
	1.0f, 0.0f, 0.8f, // 4
	1.5f, 1.0f, 1.0f, // 5
	// back side
	-0.5f, 1.0f, 0.0f,// 6
	0.0f, 0.0f, 0.2f, // 7
	0.0f, 1.0f, 0.0f, // 8
	1.0f, 1.0f, 0.0f, // 9
	1.0f, 0.0f, 0.2f, // 10
	1.5f, 1.0f, 0.0f, // 11
};*/

static float cartPosCoords[] = {
	// front side
	-0.5f, 0.8f, 1.0f,// 0
	0.0f, 0.0f, 1.0f, // 1
	0.0f, 0.8f, 1.0f, // 2
	1.0f, 0.8f, 1.0f, // 3
	1.0f, 0.0f, 1.0f, // 4
	1.5f, 0.8f, 1.0f, // 5
	// back side
	-0.5f, 0.8f, 0.0f,// 6
	0.0f, 0.0f, 0.0f, // 7
	0.0f, 0.8f, 0.0f, // 8
	1.0f, 0.8f, 0.0f, // 9
	1.0f, 0.0f, 0.0f, // 10
	1.5f, 0.8f, 0.0f, // 11
};

static short cartTexCoords[] = {
	// front side (cart side)
	0, 0, // 0
	1, 1, // 1
	1, 0, // 2
	
	0, 1, // 1
	1, 0, // 3
	0, 0, // 2
	
	0, 1, // 1
	1, 1, // 4
	1, 0, // 3
	
	0, 1, // 4
	1, 0, // 5
	0, 0, // 3
	
	// back side (cart side)
	1, 0, // 8
	1, 1, // 7
	0, 0, // 6
	
	0, 0, // 8
	1, 0, // 9
	0, 1, // 7
	
	1, 0, // 9
	1, 1, // 10
	0, 1, // 7
	
	0, 1, // 10
	0, 0, // 9
	1, 0, // 11
	
	// left cartfront
	0,0,
	0,1,
	1,1,
	0,0,
	1,1,
	1,0,
	
	// right cartback
	0,0,
	0,1,
	1,1,
	1,1,
	1,0,
	0,0,
	
	// bottom cartbottom
	0,0,
	1,0,
	1,1,
	1,1,
	0,1,
	0,0,
};

const int NUM_CART_VERTICES = 42;

static short cartPosIndices[NUM_CART_VERTICES] = {
	0,1,2,	//2,1,0,
	1,3,2,	//2,3,1,
	1,4,3,	//3,4,1,
	4,5,3,	//3,5,4,

	8,7,6,	 //6,7,8,
	8,9,7,	 //7,9,8,
	9,10,7,	 //7,10,9,
	10,9,11, //11,9,10,

	6,7,1,	//1,7,6,
	6,1,0,	//0,1,6,

	5,4,10,	 //10,4,5,
	10,11,5, //5,11,10,

	7,1,4,
	4,10,7
};

void RailManager::initCartMesh()
{
	const int nverts = NUM_CART_VERTICES;
	const int l = nverts*(NUM_POS_COORD_COMPS+NUM_TX_COORD_COMPS+NUM_COL_COORD_COMPS);
	float coords[l];

	int k=0;

	const int texRow = 5;
	const int texCol = 7;

	TexCoordRect tcr(TEX_COORD_FACTOR * texCol, TEX_COORD_FACTOR * (texCol + 1),
		TEX_COORD_FACTOR * texRow, TEX_COORD_FACTOR * (texRow + 1));

	for(int i=0; i<nverts; i++) {
		int curIndex = cartPosIndices[i] * 3;
		coords[k++] = cartPosCoords[curIndex++] - 0.5f; // x
		coords[k++] = cartPosCoords[curIndex++]; // y
		coords[k++] = cartPosCoords[curIndex++] - 0.5f; // z

		coords[k++] = tcr.minU + (tcr.maxU - tcr.minU) * (float)cartTexCoords[i*2];// * 0.9f; // u
		coords[k++] = tcr.minV + (tcr.maxV - tcr.minV) * (float)cartTexCoords[i*2+1];// * 0.9f; // v

		coords[k++] = 1.0f; // r
		coords[k++] = 1.0f; // g
		coords[k++] = 1.0f; // b
		coords[k++] = 1.0f; // a
	}

	cartMesh.setVertices(coords, l);	
}

void RailManager::renderCart()
{
	if(!cart.active) return;

	glPushMatrix();
	glTranslatef(cart.pos.x + 0.5f, cart.pos.y, cart.pos.z + 0.5f);
	glRotatef(cart.yaw, 0.0f, 1.0f, 0.0f);

	glDisable(GL_CULL_FACE);
	cartMesh.render();
	glEnable(GL_CULL_FACE);

	glPopMatrix();
}

void RailManager::spawnCartAtPos( BlockPos pos )
{
	cart.pos.setTo((float)pos.x, (float)pos.y+0.2f, (float)pos.z);
	cart.yaw = 0.0f;
	cart.active = true;

	Vec3 *cartPos = &cart.pos;
	BlockPos railBelowCart((int)cartPos->x, (int)(cartPos->y - 0.3f), (int)cartPos->z);

	if(determineRailTypeForPos(railBelowCart) == RT_ZDIR) {
		cart.yaw = 90.0f;
	}
}

bool RailManager::nearCart( Vec3 pos )
{
	if(!cart.active) return false;

	Vec3 delta = pos - cart.pos;
	return delta.length() <= 1.0f;
}

Vec3 RailManager::getCartPos() const
{
	return cart.pos;
}

void RailManager::moveCartForward( ticks_t delta )
{
	inertia += 0.01f * delta;
}

void RailManager::moveCartBackward( ticks_t delta )
{
	inertia -= 0.01f * delta;
}

void RailManager::moveCommon( float factor, float delta )
{
	Vec3 *cartPos = &cart.pos;

	BlockPos railBelowCart((int)cartPos->x, (int)(cartPos->y - 0.3f), (int)cartPos->z);

	std::map<BlockPos, RailType>::iterator it = rails.find(railBelowCart);
	if(it == rails.end()) {
		if(targetPosSet) {
			cart.pos = targetPos;
			xfactor = targetXfactor;
			zfactor = targetZfactor;
			cart.yaw += 90.0f;
			targetPosSet = false;
		} else {
			cart.pos = lastValidPos;
		}
		return;
	}
	
	RailType type = determineRailTypeForPos(railBelowCart);	

	lastValidPos = cart.pos;

	if(type != RT_XDIR && type != RT_ZDIR) {
		if(!targetPosSet) {
			prevTurn = type;

			targetPos.y = cart.pos.y;
		
			switch(prevTurn) {
			case RT_NX_TO_NZ:				
				if(prevRailType == RT_XDIR) {
					targetPos.x = std::truncf(cart.pos.x);
					targetPos.z = cart.pos.z-1;
					targetZfactor = -1.0f;
				} else {				
					targetPos.x = cart.pos.x+1;
					targetPos.z = std::truncf(cart.pos.z);
					targetXfactor = 1.0f;
				}
				break;
			case RT_NX_TO_PZ:
				if(prevRailType == RT_XDIR) {
					targetPos.x = std::truncf(cart.pos.x);
					targetPos.z = cart.pos.z+1;
					targetZfactor = 1.0f;
				} else {
					targetPos.x = cart.pos.x+1;
					targetPos.z = std::truncf(cart.pos.z);
					targetXfactor = 1.0f;
				}
				break;
			case RT_PX_TO_NZ:
				if(prevRailType == RT_XDIR) {
					targetPos.x = std::truncf(cart.pos.x);
					targetPos.z = cart.pos.z-1;
					targetZfactor = -1.0f;

				} else {
					targetPos.x = cart.pos.x-1;
					targetPos.z = std::truncf(cart.pos.z);
					targetXfactor = -1.0f;
				}
				break;
			case RT_PX_TO_PZ:
				if(prevRailType == RT_XDIR) {
					targetPos.x = std::truncf(cart.pos.x);
					targetPos.z = cart.pos.z+1;
					targetZfactor = 1.0f;
				} else {
					targetPos.x = cart.pos.x-1;
					targetPos.z = std::truncf(cart.pos.z);
					targetXfactor = -1.0f;
				}
				break;
			default:
				return;
			}

			targetZfactor *= factor;
			targetXfactor *= factor;
			targetPosSet = true;

			return;
		}

		type = prevRailType;
	}

	switch(type) {
	case RT_XDIR:
		cartPos->x = cartPos->x+0.1f*factor*xfactor*delta;
		break;
	case RT_ZDIR:
		cartPos->z = cartPos->z+0.1f*factor*zfactor*delta;
		break;
	default:
		return;
	}

	prevRailType = type;
}

Vec3 RailManager::cartIntertia(ticks_t delta)
{
	float factor = (inertia > 0) ? 1.0f : -1.0f;
	moveCommon(factor, delta * 0.005f * ABS(inertia));
	inertia *= 0.97f;

	return cart.pos;
}

}

