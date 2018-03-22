#include "Plane.h"

#define NUM_CORNERS 4
#define PI					3.14159265f

// Constructor
Plane::Plane( const vec3* pPosition,
			  const vector<glm::vec3>* pCorners,
			  long lID,
			  const string* sTexName,
			  bool bUseEB, const Anim_Track* pAnimTrack ) : Object3D( pPosition, lID, sTexName, pAnimTrack )
{
	if ( NUM_CORNERS != pCorners->size() )	// Set up a default Plane
	{
		m_pVertices.push_back( glm::vec3( -1.f, 0.f, -1.f ) );
		m_pVertices.push_back( glm::vec3( -1.f, 0.f, 1.f ) );
		m_pVertices.push_back( glm::vec3( 1.f, 0.f, -1.f ) );
		m_pVertices.push_back( glm::vec3( 1.f, 0.f, 1.f ) );
	}
	else
		m_pVertices.insert( m_pVertices.begin(), pCorners->begin(), pCorners->end() );

	vec3 vTranslateVector = m_pPosition - vec3(0.0);
	mat4 mTranslationMatrix = translate(mat4(1.0), vTranslateVector);
	m_fD = length(vTranslateVector);

	// Translate Plane to Specified Position in 3D space.
	for (vector< vec3 >::iterator iter = m_pVertices.begin();
		iter != m_pVertices.end();
		++iter)
		(*iter) = vec3( mTranslationMatrix * vec4((*iter), 1.0) );

	// Set up Normal and Vertex Normals
	m_pNormal = normalize( cross( m_pVertices[ 1 ] - m_pVertices[ 0 ], m_pVertices[ 2 ] - m_pVertices[ 0 ] ) );
	m_pNormals.resize( m_pVertices.size(), m_pNormal );

	m_fScale = 1.f;

	glGenVertexArrays( 1, &m_iVertexArray );

	m_iVertexBuffer = ShaderManager::getInstance()->genVertexBuffer( m_iVertexArray,
									    							 0, 3, m_pVertices.data(),
																	 m_pVertices.size() * sizeof( glm::vec3 ), GL_STATIC_DRAW );
	m_iNormalBuffer = ShaderManager::getInstance()->genVertexBuffer( m_iVertexArray,
																	 1, 3, m_pNormals.data(),
																	 m_pNormals.size() * sizeof( glm::vec3 ), GL_STATIC_DRAW );
	calculateUVs();

	if ( bUseEB )
	{
		m_pEdgeBuffer = new EdgeBuffer( m_iVertexArray );
		m_pEdgeBuffer->GenerateAdjListStrip( m_pVertices, m_pNormals, m_pVertices.size() );
	}
	else
		m_pEdgeBuffer = nullptr;

	if ( nullptr == m_pTexture )
		ShaderManager::getInstance()->setUniformBool( ShaderManager::eShaderType::PLANE_SHDR, "bTextureLoaded", false );
}

// Destructor
Plane::~Plane()
{
	if ( nullptr != m_pEdgeBuffer )
		delete m_pEdgeBuffer;
}

// Overridden Debug Output
string Plane::getDebugOutput()
{
	string sOutput = "";

	sOutput += getType();
	sOutput += "/ID:" + to_string( m_lID );
	sOutput += "/Normal:" + glm::to_string( m_pNormal );

	return sOutput;
}

// Checks collision of ray from start point against this plane.
bool Plane::isCollision(const vec3& vStart, const vec3& vRay, float& fT, vec3& vIntersectingNormal)
{
	// Local Variables
	vec3 vDest = vStart + vRay;
	vec3 vRayNormalized, vIntersection;
	fT = dot(vDest, m_pNormal) - m_fD;
	bool bReturn = false;

	// Lies on or behind the plane but started before intersecting.
	if ( fT <= 0.0f && (dot(vStart, m_pNormal) - m_fD) >= 0.0f )
	{
		// Calculate whether the ray crosses through the plane
		vRayNormalized = normalize(vRay);
		fT = -(dot(m_pNormal, vStart) - m_fD) / dot(m_pNormal, vRayNormalized);
		vIntersection = vStart + (vRayNormalized * fT);
		vec3 v1, v2, v3, v4;

		v1 = normalize(vIntersection - m_pVertices[0]);
		v2 = normalize(vIntersection - m_pVertices[1]);
		v3 = normalize(vIntersection - m_pVertices[2]);

		float fTheta1;

		// Angles around intersection should total 360 (2Pi)
		/*
			glm::vec3( -1.f, 0.f, -1.f )
			glm::vec3( -1.f, 0.f, 1.f )
			glm::vec3( 1.f, 0.f, -1.f )
			glm::vec3( 1.f, 0.f, 1.f ) )

			v2-v4
			| \ |
			v1-v3
		*/
		fTheta1 = acos(dot(v1, v2))
				+ acos(dot(v2, v3))
				+ acos(dot(v3, v1));

		// Check Tri v1,v2,v3
		// Intersected through first Triangle
		if (fabs(fTheta1 - (2 * PI) < 0.1))
			bReturn = true;
		else
		{
			v4 = normalize(vIntersection - m_pVertices[3]);

			// Check Tri v2,v4,v3
			fTheta1 = acos(dot(v2, v4))
					+ acos(dot(v4, v3))
					+ acos(dot(v3, v2));

			// Intersected through other triangle instead.
			if (fabs(fTheta1 - (2 * PI) < 0.1))
				bReturn = true;
		}
	}

	// Return the Normal
	if (bReturn)
		vIntersectingNormal = m_pNormal;

	// Return true if possible intersection
	return bReturn;
}

// Setup OpenGl to draw the Plane using the Plane Shader.
void Plane::draw( const vec3& vCamLookAt, float fMinThreshold, float fMaxThreshold, bool m_bPause )
{
	glBindVertexArray( m_iVertexArray );

	if ( nullptr != m_pTexture )
	{
		m_pTexture->bindTexture( ShaderManager::eShaderType::PLANE_SHDR, "gSampler" );
	}

	glUseProgram( ShaderManager::getInstance()->getProgram( ShaderManager::eShaderType::PLANE_SHDR ) );

	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

	// Render EdgeBuffer
	if ( nullptr != m_pEdgeBuffer )
	{
		m_pEdgeBuffer->CalculateEdgeBufferStrip( m_pNormals, &vCamLookAt );
		m_pEdgeBuffer->drawEdgeBuffer( m_fScale, m_pPosition, fMinThreshold, fMaxThreshold );
	}

	if ( nullptr != m_pTexture )
		m_pTexture->unbindTexture();

	glUseProgram(0);
	glBindVertexArray( 0 );
}

void Plane::calculateUVs()
{
	m_pUVs.push_back( vec2( 0.0, 0.0 ) );
	m_pUVs.push_back( vec2( 0.0, 1.0 ) );
	m_pUVs.push_back( vec2( 1.0, 0.0 ) );
	m_pUVs.push_back( vec2( 1.0, 1.0 ) );

	m_iTextureBuffer = ShaderManager::getInstance()->genVertexBuffer( m_iVertexArray,
																	  2, 2, m_pUVs.data(),
																	  m_pUVs.size() * sizeof( vec2 ), GL_STATIC_DRAW );
}
