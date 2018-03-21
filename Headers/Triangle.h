#pragma once
#include "Object3D.h"

#define P1 0
#define P2 1
#define P3 2

class Triangle :
	public Object3D
{
public:
	~Triangle();

	// Overridden intersect function
	void draw( const vec3& vCamLookAt, float fMinThreshold, float fMaxThreshold, bool m_bPause );
	
	// Overridden Type Output
	string getType() { return "Triangle"; }

	// Overridden Debug Output
	string getDebugOutput();

	bool isCollision(const vec3& vStart, const vec3& vRay, float& fT, vec3& vIntersectingNormal) { return false; /*Not Implemented*/ }

private:
	// Only Accessable by Object Factory
	Triangle( const vec3* pPosition,
			  const vector<vec3>* pVerts,			  
			  long lID, const string* sTexName, const Anim_Track* pAnimTrack );
	Triangle( const Triangle* pNewTriangle );  // Protected Copy Constructor

	// Inherited from Parent
	void calculateUVs();

	// Declare Object_Factory as a Friend to be sole progenitor of Triangles
	friend class Object_Factory;
};

