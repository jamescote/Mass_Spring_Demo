#pragma once
#include "MassSpringSystem.h"

/***********\
 * DEFINES *
\***********/
#define DEFAULT_START_POS vec3( 0.0, 10.0f, 0.0f )
#define DELTA_T 0.0001f
#define GRAVITY -9.81
#define DAMPING_COEFF .0010f
#define LOOPS_PER_UPDATE 16

const vec3 vGRAVITY = vec3(0.0f, GRAVITY, 0.0f);

// Default Constructor
MassSpringSystem::MassSpringSystem( float fK, float fRestLength, float fMass )
{
	// Store Private Variables
	m_fK = fK;
	m_fRestLength = fRestLength;
	m_fMass = fMass;
	m_vShdrMngr = ShaderManager::getInstance();

	// Generate Vertex Array
	glGenVertexArrays( 1, &m_iVertexArray );

	// Set up GL_Buffers
	m_iVertexBuffer = m_vShdrMngr->genVertexBuffer(
		m_iVertexArray,
		0, 3,
		m_vPositions.data(),
		m_vPositions.size() * sizeof( vec3 ),
		GL_DYNAMIC_DRAW );

	m_iNormalBuffer = m_vShdrMngr->genVertexBuffer(
		m_iVertexArray,
		1, 3,
		m_vNormals.data(),
		m_vNormals.size() * sizeof( vec3 ),
		GL_STATIC_DRAW );

	m_iIndicesBuffer = m_vShdrMngr->genIndicesBuffer(
		m_iVertexArray,
		m_vIndices.data(),
		m_vIndices.size() * sizeof( unsigned int ),
		GL_STATIC_DRAW );
}

// Destructor: Cleanup any allocated memory
MassSpringSystem::~MassSpringSystem()
{
	// Lose reference to Shader Manager
	m_vShdrMngr = nullptr;

	// Delete GL handles
	glDeleteBuffers( 1, &m_iNormalBuffer );
	glDeleteBuffers( 1, &m_iIndicesBuffer );
	glDeleteBuffers( 1, &m_iVertexBuffer );
	glDeleteVertexArrays( 1, &m_iVertexArray );

	// Delete Masses -> delete each one individually and clear vector of pointers
	if (!m_vMasses.empty())
	{
		for (vector< PointMass* >::iterator iter = m_vMasses.begin();
			iter != m_vMasses.end();
			++iter)
			delete *iter;

		m_vMasses.clear();
	}

	// Free Spring Data Space
	if (!m_vSprings.empty())
	{
		for (vector< Spring* >::iterator iter = m_vSprings.begin();
			iter != m_vSprings.end();
			++iter)
			delete *iter;

		m_vSprings.clear();
	}

	// Clear Vectors
	m_vIndices.clear();
	m_vPositions.clear();
	m_vNormals.clear();
}

// Initialization function for the Mass Spring System
//	Parameters: Length, Height, Depth -> Specifies the dimensions of the Mass Spring System.
//				eType -> fixes certain points given the type:
//						cloth: Fixes the first and iLength - 1 point
//						chain and spring: Fixes the first point
//						cube: No points are fixed.
//	Will create all masses with scaled masses so that the whole system will have the full mass specified.
//	All masses internally will be connected to their neighbours with springs.
void MassSpringSystem::initialize( int iLength, int iHeight, int iDepth, SpringType eType )
{
	// Local Variables
	int iDepthOffset = 0;
	int iHeightOffset = 0;
	int iTotalOffset = 0;
	int iLxH = iLength * iHeight;
	float fScaledMass = m_fMass / (iLxH * iDepth);
	bool bFixed = false;
	vec3 vStartPos = DEFAULT_START_POS;
	
	// Store a center position for Camera to focus on.
	m_vCenter = vec3(DEFAULT_START_POS.x + (((float)iLength / 2.0f) * m_fRestLength),
					 DEFAULT_START_POS.y - (((float)iHeight / 2.0f) * m_fRestLength),
					 DEFAULT_START_POS.z + (((float)iDepth / 2.0f) * m_fRestLength));

	if (eType == SpringType::SPRING || eType == SpringType::CHAIN)
		m_vCenter = DEFAULT_START_POS;

	// Generate PointMasses and Springs
	for ( int z = 0; z < iDepth; ++z )
	{
		iDepthOffset = z * iLxH;
		for ( int y = 0; y < iHeight; ++y )
		{
			iHeightOffset = y * iLength;
			iTotalOffset = iHeightOffset + iDepthOffset;
			for ( int x = 0; x < iLength; ++x )
			{
				// Gen PointMass
				m_vMasses.push_back( new PointMass( fScaledMass, vStartPos, bFixed ) );
				int iLengthOffset = iTotalOffset + x;

				// Create springs attached to this new point
				//* Along the x-Chain
				if (x)
					m_vSprings.push_back(new Spring(m_vMasses[iLengthOffset - 1], m_vMasses[iLengthOffset]));
				//*/

				//* Along the y-chain
				if (y)
				{
					//* Cross along xy-plane
					if (x)
						m_vSprings.push_back(new Spring(m_vMasses[iLengthOffset - iLength - 1], m_vMasses[iLengthOffset]));
					if ((x + 1) < iLength)
						m_vSprings.push_back(new Spring(m_vMasses[iLengthOffset - iLength + 1], m_vMasses[iLengthOffset]));
					//*/

					//* Along Y-Axis
					m_vSprings.push_back(new Spring(m_vMasses[iLengthOffset - iLength], m_vMasses[iLengthOffset]));
					//*/
				}

				// Along the z-chain
				if (z)
				{
					if (y)
					{
						//* Cross along 3D Diagonal front to back
						if( x ) // along xy-diagonal
							m_vSprings.push_back(new Spring(m_vMasses[iLengthOffset - iLxH - iLength - 1], m_vMasses[iLengthOffset]));
						if( iLength > (x + 1) )
							m_vSprings.push_back(new Spring(m_vMasses[iLengthOffset - iLxH - iLength + 1], m_vMasses[iLengthOffset]));
						//*/
					}
		
					//* Along Z
					m_vSprings.push_back(new Spring(m_vMasses[iLengthOffset - iLxH], m_vMasses[iLengthOffset]));
					//*/

					if( iHeight > ( y + 1 ) )
					{
						//* Cross along 3D Diagonal back to front
						if( x )
							m_vSprings.push_back(new Spring(m_vMasses[iLengthOffset - iLxH + iLength - 1], m_vMasses[iLengthOffset]));
						if( iLength > (x + 1) )
							m_vSprings.push_back(new Spring(m_vMasses[iLengthOffset - iLxH + iLength + 1], m_vMasses[iLengthOffset]));
						//*/
					}

					//* Cross along xz-Plane
					if (x) // along x
						m_vSprings.push_back(new Spring(m_vMasses[iLengthOffset - iLxH - 1], m_vMasses[iLengthOffset]));
					if( iLength > (x + 1) )
							m_vSprings.push_back(new Spring(m_vMasses[iLengthOffset - iLxH + 1], m_vMasses[iLengthOffset]));
					//*/

					//* Cross along yz-Plane
					if( iHeight > (y + 1) )
						m_vSprings.push_back( new Spring( m_vMasses[iLengthOffset - iLxH + iLength], m_vMasses[iLengthOffset]));
					if ( y )
						m_vSprings.push_back(new Spring(m_vMasses[iLengthOffset - iLxH - iLength], m_vMasses[iLengthOffset]));
					//*/
				}

				//cout << "{" << vStartPos.x << "|" << vStartPos.y << "|" << vStartPos.z << "} ";
				vStartPos.x += m_fRestLength;
			}
			//cout << endl;
			vStartPos.x = DEFAULT_START_POS.x;
			vStartPos.y -= m_fRestLength;
		}
		vStartPos.y = DEFAULT_START_POS.y;
		vStartPos.z += m_fRestLength;
	}

	// Apply Fixed Specifications
	if( eType != CUBE )
		m_vMasses.front()->m_bFixed = true;
	if (eType == CLOTH)
		m_vMasses[iLength - 1]->m_bFixed = true;
}

// Draws the Mass Spring System
void MassSpringSystem::draw( const vec3& vCamLookAt, bool m_bPause )
{
	// Only update if not paused
	if( !m_bPause )
		update();

	// Set up Shader
	glBindVertexArray( m_iVertexArray );
	glUseProgram( m_vShdrMngr->getProgram( ShaderManager::eShaderType::WORLD_SHDR ) );
	vec3 WHITE(0.75);
	vec3 BLACK(0.0, 0.75, 1.0);

	// Set color and draw Masses
	m_vShdrMngr->setUniformVec3(ShaderManager::eShaderType::WORLD_SHDR, "vColor", &BLACK);
	glBindBuffer( GL_ARRAY_BUFFER, m_iVertexBuffer );
	glBufferData( GL_ARRAY_BUFFER, m_vPositions.size() * sizeof( vec3 ), m_vPositions.data(), GL_DYNAMIC_DRAW );
	glPointSize( 7.5f );
	glDrawArrays( GL_POINTS, 0, m_vPositions.size() );
	glPointSize( 1.0f );

	// Set Color and Draw Lines
	m_vShdrMngr->setUniformVec3(ShaderManager::eShaderType::WORLD_SHDR, "vColor", &WHITE);
	glBufferData(GL_ARRAY_BUFFER, m_vNormals.size() * sizeof(vec3), m_vNormals.data(), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_LINES, 0, m_vNormals.size());

	glUseProgram( 0 );
	glBindVertexArray( 0 );
}

// Update the Mass Spring system by evaluating the force of every spring against its connected masses
void MassSpringSystem::update()
{
	// Calculat Direction both ways and Forces in both Directions
	vec3 vD12, vD21, vF12, vF21;

	// Apply this multiple times per update to give an accurate depiction of movement per frame at a small Delta_T
	for( int i = 0; i < LOOPS_PER_UPDATE; ++i )
	{
		// Iterate over every spring
		for (vector< Spring* >::iterator iter = m_vSprings.begin();
			iter != m_vSprings.end();
			++iter)
		{
			// Get Directions
			vD12 = (*iter)->pPoint2->m_vPosition - (*iter)->pPoint1->m_vPosition;
			vD21 = -vD12;

			// Calculate Spring Force and apply along direction
			float fSpring = m_fK * (length(vD12) - (*iter)->m_fRestLength);
			vF12 = fSpring * normalize(vD12);
			vF21 = fSpring * normalize(vD21);

			// Add forces to points along with damping
			(*iter)->pPoint1->m_vForce += vF12 - ((*iter)->pPoint1->m_vVelocity * DAMPING_COEFF);
			(*iter)->pPoint2->m_vForce += vF21 - ((*iter)->pPoint2->m_vVelocity * DAMPING_COEFF);
		}

		// Apply Forces and update velocities and positions for Masses
		for (vector< PointMass* >::iterator iter = m_vMasses.begin();
			iter != m_vMasses.end();
			++iter)
		{
			if (!(*iter)->m_bFixed)
			{
				// Add Gravity and Collision Forces
				(*iter)->m_vForce += vGRAVITY;
				checkCollision( *(*iter) );

				vec3 vAcceleration = (*iter)->m_vForce / (*iter)->m_fMass;		// Acceleration from Newton's 2nd Law F=ma
				(*iter)->m_vVelocity += vAcceleration * DELTA_T;				// Velocity from definition
				(*iter)->m_vPosition += (*iter)->m_vVelocity * DELTA_T;			// Position from definition
				(*iter)->m_vForce = vec3(0.0f);									// reset Forces
			}
		}
	}

	// Add Points and Lines for Drawing.
	m_vPositions.clear();
	m_vNormals.clear();

	for (vector< PointMass* >::const_iterator iter = m_vMasses.begin();
		iter != m_vMasses.end();
		++iter)
		m_vPositions.push_back((*iter)->m_vPosition);

	for (vector< Spring* >::const_iterator iter = m_vSprings.begin();
		iter != m_vSprings.end();
		++iter)
	{
		m_vNormals.push_back((*iter)->pPoint1->m_vPosition);
		m_vNormals.push_back((*iter)->pPoint2->m_vPosition);
	}

}

// Checks General collision with xz-plane @ y = 0
//	Testing: Was testing a table cloth; masses that fell off the side ended up stretching the cloth to infinity, not sure why.
void MassSpringSystem::checkCollision( PointMass& pMass )
{
	// Project position
	vec3 vNewPos = pMass.m_vPosition + (pMass.m_vVelocity * DELTA_T);

	// Past xy-plane @ y = 0?
	if( vNewPos.y < 0.0 )
	{
		// Check for Table intersection
		//if ( abs( vNewPos.x - 3.0 ) < 2.0f && abs( vNewPos.z - 3.0 ) < 2.0f )
		//{
			// Calculate Spring to add force to plane.
			vec3 vNormal = vec3( 0.0, vNewPos.y, 0.0 );
			vec3 vSpringForce = -m_fK * vNormal - (DAMPING_COEFF * pMass.m_vVelocity);

			pMass.m_vForce += vSpringForce;
		//}
	}
}

// Updates and returns the look at for the camera.
const vec3& MassSpringSystem::getCenter()
{
	m_vCenter = (m_vMasses.empty() ? m_vCenter : vec3(0.0, m_vMasses.front()->m_vPosition.y, 0.0)); 
	return m_vCenter; 
}