#pragma once
#include "stdafx.h"
#include "ShaderManager.h"

enum SpringType
{
	SPRING = 0,
	CHAIN,
	CUBE,
	CLOTH,
	FLAG,
	MAX_TYPES
};


class MassSpringSystem
{
public:
	MassSpringSystem( float fK, float fRestLength, float fMass,
					  float fDamp_Coeff, float fDelta_T, unsigned int iLoopCount,
					  float fCollision_K, float fCollision_Damp );
	~MassSpringSystem();

	// Overridden intersect function
	void draw( const vec3& vCamLookAt, bool m_bPause );
	void update();

	const vec3& getCenter();

	void initialize( int iLength, int iHeight, int iDepth, SpringType eType );
	
private:
	// Only Accessable by Object Factory
	MassSpringSystem( const MassSpringSystem* pNewMassSpringSystem );  // Protected Copy Constructor
	float m_fK, m_fRestLength, m_fMass;
	float m_fDeltaT, m_fDamping_Coeff, m_fCollisionK, m_fCollisionDamp;
	unsigned int m_iLoopCount;
	ShaderManager* m_vShdrMngr;
	GLuint m_iVertexArray, m_iVertexBuffer, m_iIndicesBuffer, m_iNormalBuffer;
	vec3 m_vCenter;

	// Point Mass Data Structure
	struct PointMass
	{
		// Private PointMass Variables
		float m_fMass;
		vec3 m_vPosition, m_vVelocity, m_vForce;
		bool m_bFixed;

		// Constructors (Default, Copy, equal Operator)
		PointMass(float fMass, const vec3& vPos, bool bFixed)
			: m_fMass( fMass ), m_vPosition( vPos ), m_vVelocity( 0.0f ), m_vForce( 0.0f ), m_bFixed( bFixed )
		{ }
		PointMass( const PointMass* pCopy )
		{
			*this = pCopy;
		}
		PointMass* operator=( const PointMass* pCopy )
		{
			this->m_fMass = pCopy->m_fMass;
			this->m_vPosition = pCopy->m_vPosition;
			this->m_vVelocity = pCopy->m_vVelocity;
			this->m_vForce = pCopy->m_vForce;
			this->m_bFixed = pCopy->m_bFixed;

			return this;
		}

	};

	// Spring Data Structure
	struct Spring
	{
		// Variables
		PointMass *pPoint1, *pPoint2;
		float m_fRestLength;

		// Constructor
		Spring( PointMass* pMass1, PointMass* pMass2 )
		{
			pPoint1 = pMass1;
			pPoint2 = pMass2;
			m_fRestLength = length( pPoint1->m_vPosition - pPoint2->m_vPosition );
		}
		
		// Destructor -> clear pointers
		~Spring()
		{
			pPoint1 = pPoint2 = nullptr;
		}
	};
	
	// Data Vectors
	vector< Spring* > m_vSprings;
	vector< PointMass* > m_vMasses;
	vector< vec3 > m_vPositions, m_vNormals;
	vector< int > m_vIndices;

	// Checks Collision 
	void checkCollision( PointMass& pMass );
};

