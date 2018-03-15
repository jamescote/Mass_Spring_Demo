#pragma once
#include "stdafx.h"
#include "EnvironmentManager.h"

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
	MassSpringSystem( float fK, float fRestLength, vec3 vDimensions );
	~MassSpringSystem();

	// Overridden intersect function
	void draw( const vec3& vCamLookAt, bool m_bPause );
	void update();

	void initialize( int iLength, int iHeight, int iDepth, SpringType eType );
	
private:
	// Only Accessable by Object Factory
	MassSpringSystem( const MassSpringSystem* pNewMassSpringSystem );  // Protected Copy Constructor
	float m_fK, m_fRestLength, m_fMass;
	EnvironmentManager* m_vEnvMngr;
	ShaderManager* m_vShdrMngr;
	GLuint m_iVertexArray, m_iVertexBuffer, m_iIndicesBuffer, m_iNormalBuffer;

	struct PointMass
	{
		// Private PointMass Variables
		float m_fMass;
		vec3 m_vPosition, m_vVelocity, m_vForce;
		bool m_bFixed;

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
	struct Spring
	{
		PointMass *pPoint1, *pPoint2;

		Spring( PointMass* pMass1, PointMass* pMass2 )
		{
			pPoint1 = pMass1;
			pPoint2 = pMass2;
		}
		~Spring()
		{
			pPoint1 = pPoint2 = nullptr;
		}
	};
	vector< Spring* > m_vSprings;
	vector< PointMass* > m_vMasses;
	vector< vec3 > m_vPositions, m_vNormals;
	vector< int > m_vIndices;
	
};

