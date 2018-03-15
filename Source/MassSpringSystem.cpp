#pragma once
#include "MassSpringSystem.h"

#define DEFAULT_START_POS vec3( 0.0, 20.0f, 0.0f )

MassSpringSystem::MassSpringSystem( float fK, float fRestLength, vec3 vDimensions )
{
	m_fK = fK;
	m_fRestLength = fRestLength;
	m_vEnvMngr = EnvironmentManager::getInstance();
	m_vShdrMngr = ShaderManager::getInstance();

	glGenVertexArrays( 1, &m_iVertexArray );

	m_iVertexBuffer = ShaderManager::getInstance()->genVertexBuffer(
		m_iVertexArray,
		0, 3,
		m_vPositions.data(),
		m_vPositions.size() * sizeof( vec3 ),
		GL_STATIC_DRAW );

	m_iNormalBuffer = ShaderManager::getInstance()->genVertexBuffer(
		m_iVertexArray,
		1, 3,
		m_vNormals.data(),
		m_vNormals.size() * sizeof( vec3 ),
		GL_STATIC_DRAW );

	m_iIndicesBuffer = ShaderManager::getInstance()->genIndicesBuffer(
		m_iVertexArray,
		m_vIndices.data(),
		m_vIndices.size() * sizeof( unsigned int ),
		GL_STATIC_DRAW );
}

MassSpringSystem::~MassSpringSystem()
{
	m_vEnvMngr = nullptr;

	glDeleteBuffers( 1, &m_iNormalBuffer );
	glDeleteBuffers( 1, &m_iIndicesBuffer );
	glDeleteBuffers( 1, &m_iVertexBuffer );
	glDeleteVertexArrays( 1, &m_iVertexArray );

	for ( vector< PointMass* >::iterator iter = m_vMasses.begin();
		 iter != m_vMasses.end();
		 ++iter )
		delete *iter;

	for ( vector< Spring* >::iterator iter = m_vSprings.begin();
		 iter != m_vSprings.end();
		 ++iter )
		delete *iter;
}

void MassSpringSystem::initialize( int iLength, int iHeight, int iDepth, SpringType eType )
{
	PointMass *pNewMass = nullptr;
	bool bFixed = false;
	vec3 vStartPos = DEFAULT_START_POS;
	m_vMasses.push_back( new PointMass( m_fMass, vStartPos, eType != CUBE ) );
	vStartPos.x += m_fRestLength;

	for ( int z = 0; z <= iDepth; ++z )
	{
		for ( int y = 0; y <= iHeight; ++y )
		{
			for ( int x = 0; x <= iLength; ++x )
			{
				bFixed = (x + 1 == iLength && eType == CLOTH);
				m_vMasses.push_back( new PointMass( m_fMass, vStartPos, bFixed ) );
				vStartPos.x += m_fRestLength;
			}
			vStartPos.x = DEFAULT_START_POS.x;
			vStartPos.y -= m_fRestLength;
		}
		vStartPos.y = DEFAULT_START_POS.y;
		vStartPos.z += m_fRestLength;
	}

	for ( vector< PointMass* >::const_iterator iter = m_vMasses.begin();
		 iter != m_vMasses.end();
		 ++iter )
		m_vPositions.push_back( (*iter)->m_vPosition );
}

// Overridden intersect function
void MassSpringSystem::draw( const vec3& vCamLookAt, bool m_bPause )
{
	glBindVertexArray( m_iVertexArray );
	glUseProgram( m_vShdrMngr->getProgram( ShaderManager::eShaderType::MESH_SHDR ) );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_iVertexBuffer );
	glBufferData( m_iVertexBuffer, m_vPositions.size() * sizeof( vec3 ), m_vPositions.data(), GL_DYNAMIC_DRAW );
	glDrawArrays( GL_POINTS, 0, m_vPositions.size() );

	glUseProgram( 0 );
	glBindVertexArray( 0 );
}

void MassSpringSystem::update()
{

}