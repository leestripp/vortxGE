#include <iostream>
// debug
#include "../vortx_debug.h"

// OpenGL
#include "../opengl/lsOpenGL.h"

// filesystem
#include "../filesystem/lsFileIO.h"
#include "../filesystem/lsDir.h"

// shaderss
#include "../shader/lsShaderManager.h"
#include "../shader/lsShader.h"

// assets
#include "../assets/lsAssetManager.h"
#include "../assets/lsAsset_Mesh.h"
#include "../assets/lsAsset_Texture.h"

// scene
#include "../scene/lsNode.h"
#include "../scene/lsObject.h"

// animation
#include "../animation/lsAnimationManager.h"

// shader
#include "../shader/lsShader.h"

// datablocks
#include "lsString.h"
#include "lsBatch.h"
#include "lsBatch_Lines.h"
#include "lsBatch_Mesh.h"
#include "lsBatch_Normals.h"
#include "lsTexture.h"
#include "lslinkedlist.h"
#include "lslistitem_vector2.h"
#include "lslistitem_vector3.h"
#include "lslistitem_index.h"
#include "lslistitem_facetri.h"
#include "lslistitem_facequad.h"
#include "lslistitem_vertexdata.h"
#include "lsMesh.h"

// tinygltf
#define TINYGLTF_NO_STB_IMAGE_WRITE		// Needed to stop linker errors.
#include <tiny_gltf.h>
#include <limits>

// Helper functions for URI decoding, adapted from tinygltf
namespace {
	inline unsigned char from_hex(unsigned char ch)
	{
		if (ch <= '9' && ch >= '0')
			ch -= '0';
		else if (ch <= 'f' && ch >= 'a')
			ch -= 'a' - 10;
		else if (ch <= 'F' && ch >= 'A')
			ch -= 'A' - 10;
		else
			ch = 0;
		return ch;
	}

	static std::string urldecode(const std::string &str)
	{
	  using namespace std;
	  string result;
	  string::size_type i;
	  for (i = 0; i < str.size(); ++i) {
		if (str[i] == '+') {
		  result += ' ';
		} else if (str[i] == '%' && str.size() > i + 2) {
		  const unsigned char ch1 =
			  from_hex(static_cast<unsigned char>(str[i + 1]));
		  const unsigned char ch2 =
			  from_hex(static_cast<unsigned char>(str[i + 2]));
		  const unsigned char ch = static_cast<unsigned char>((ch1 << 4) | ch2);
		  result += static_cast<char>(ch);
		  i += 2;
		} else {
		  result += str[i];
		}
	  }
	  return result;
	}
} // anonymous namespace


lsMesh::lsMesh( lsOpenGL *opengl ) : lsData()
{
	ogl = opengl;
	batch_list = nullptr;
}

lsMesh::~lsMesh()
{
	// lsBatch auto deletes
	if( batch_list ) delete batch_list;
}

void lsMesh::Draw( lsCamera *cam, lsPass *pass, lsObject *object )
{
	if( batch_list ) batch_list->Draw( cam, object, pass );
}

void lsMesh::Add_Batch( lsBatch *batch )
{
#ifdef VORTX_DEBUG_BATCH
	cout << "lsMesh::Add_Batch() - Name: " << batch->getName() << endl;
#endif

	batch->setNext_batch( batch_list );
	if( batch_list ) batch_list->setPrev_batch( batch );
	batch_list = batch;
}

lsOpenGL *lsMesh::getOgl() const
{
	return ogl;
}

void lsMesh::setOgl(lsOpenGL *value)
{
	ogl = value;
}

void lsMesh::Update_Animation()
{
	// if( batch_list ) batch_list->Update();
}


void lsMesh::LoadVBM(const string& filename, lsAssetManager *am, lsShaderManager *sm )
{
	// Set shader type
	if( filename == "" )
	{
		cout << "ERROR: lsMesh::LoadVBM() - filename empty" << endl;
		return;
	}

#ifdef VORTX_DEBUG_ASSETS
	cout << "***** Load VBM Mesh : " << filename << endl;
#endif
	// VBM file
	string basepath;
	lsDir dir;
	// open the file
	basepath = dir.getexepath();
	lsFileIO meshfile;
	if(! meshfile.open( basepath + filename, LSIO_LINE_READ )  )
	{
		// ERROR
		return;
	}

	// Text processing
	vector<string> list;
	vector<string> sublist;
	string cmdline;
	lsString convert;

	// convert data
	float rx, ry, rz, rw;

	// lists to hold data for processing - file load.
	lsLinkedList triindex_list;
	lsLinkedList quadindex_list;
	// data
	lsLinkedList vertex_list;
	lsLinkedList normal_list;
	lsLinkedList tangent_list;
	lsLinkedList uv_list;
	// data ptrs
	lsListItem_faceTri *ptrFaceTri;
	lsListItem_faceQuad *ptrFaceQuad;
	lsListItem_Vector3 *ptrVertex;
	lsListItem_Vector3 *ptrNormal;
	lsListItem_Vector3 *ptrTangent;
	lsListItem_Vector2 *ptrUV;

	// Batch Mesh
	lsBatch_Mesh *batch = NULL;
	// Assets
	lsAsset_Texture *image;
	lsTexture *texture;
	// shader
	lsShader *shader;

	// read a line
	while( meshfile.readline() )
	{
		cmdline = meshfile.getLine();
		if( cmdline != "" )
		{
			list.clear();
			// split line into parts
			convert.SplitString( cmdline, ' ', list );

			if( list[0] == "batch" )
			{
				if( list[1] == "start" )
				{
					batch = new lsBatch_Mesh( getOgl() );
					if( batch )
					{
						Add_Batch( batch );
					}
				} else if( list[1] == "end" )
				{
					// Batch data loaded
					/* ################ Debug data output */
#ifdef VORTX_DEBUG_BATCH
					cout << "***********************************************" << endl;
					cout << "lsMesh::LoadVBM - Check data loaded into lists" << endl;
					cout << "Vertex count   : " << vertex_list.getItem_count() << endl;
					vertex_list.Debug();;
					cout << "Normal count   : " << normal_list.getItem_count() << endl;
					cout << "Tangent count  : " << tangent_list.getItem_count() << endl;
					cout << "UV count       : " << uv_list.getItem_count() << endl;
					// faces
					cout << "Tri face count  : " << triindex_list.getItem_count() << endl;
					triindex_list.Debug();
					cout << "Quad face count : " << quadindex_list.getItem_count() << endl;
					quadindex_list.Debug();
#endif

					// ############################################# Process data - pass 2

					// Pass 2 data
					lsLinkedList vertexdata_list;
					lsListItem_vertexdata *ptrVertexData;
					lsLinkedList tridata_list;
					lsLinkedList quaddata_list;
					lsListItem_index *ptrTriData;
					lsListItem_index *ptrQuadData;

					// cycle quad faces
					ptrFaceQuad = (lsListItem_faceQuad*)quadindex_list.getItem_list();
					while( ptrFaceQuad )
					{
						// Add quad
						// V1
						ptrVertexData = new lsListItem_vertexdata;
						if( ptrVertexData )
						{
							// Look up and fill out data
							// Find Vertex
							ptrVertex = (lsListItem_Vector3*)vertex_list.Find_item( ptrFaceQuad->v1 );
							ptrVertexData->vertex[0] = ptrVertex->vec[0];
							ptrVertexData->vertex[1] = ptrVertex->vec[1];
							ptrVertexData->vertex[2] = ptrVertex->vec[2];
							// Find Normal
							ptrNormal = (lsListItem_Vector3*)normal_list.Find_item( ptrFaceQuad->n1 );
							ptrVertexData->normal[0] = ptrNormal->vec[0];
							ptrVertexData->normal[1] = ptrNormal->vec[1];
							ptrVertexData->normal[2] = ptrNormal->vec[2];
							// Find Tangent
							ptrTangent = (lsListItem_Vector3*)tangent_list.Find_item( ptrFaceQuad->n1 ); // Tangents use the same index as Normals
							ptrVertexData->tangent[0] = ptrTangent->vec[0];
							ptrVertexData->tangent[1] = ptrTangent->vec[1];
							ptrVertexData->tangent[2] = ptrTangent->vec[2];
							// Fjind UV
							ptrUV = (lsListItem_Vector2*)uv_list.Find_item( ptrFaceQuad->uv1 );
							ptrVertexData->uv[0] = ptrUV->vec[0];
							ptrVertexData->uv[1] = ptrUV->vec[1];

							// Remember this returns the index
							int i = vertexdata_list.Add_ChkDupli( ptrVertexData );
							ptrQuadData = new lsListItem_index;
							if( ptrQuadData )
							{
								// set index
								ptrQuadData->setIndex( i );
								quaddata_list.Add_itematEnd( ptrQuadData );
							}
						} // if( ptrVertexData )

						// V2
						ptrVertexData = new lsListItem_vertexdata;
						if( ptrVertexData )
						{
							// Look up and fill out data
							// Find Vertex
							ptrVertex = (lsListItem_Vector3*)vertex_list.Find_item( ptrFaceQuad->v2 );
							ptrVertexData->vertex[0] = ptrVertex->vec[0];
							ptrVertexData->vertex[1] = ptrVertex->vec[1];
							ptrVertexData->vertex[2] = ptrVertex->vec[2];
							// Find Normal
							ptrNormal = (lsListItem_Vector3*)normal_list.Find_item( ptrFaceQuad->n2 );
							ptrVertexData->normal[0] = ptrNormal->vec[0];
							ptrVertexData->normal[1] = ptrNormal->vec[1];
							ptrVertexData->normal[2] = ptrNormal->vec[2];
							// Find Tangent
							ptrTangent = (lsListItem_Vector3*)tangent_list.Find_item( ptrFaceQuad->n2 ); // Tangents use the same index as Normals
							ptrVertexData->tangent[0] = ptrTangent->vec[0];
							ptrVertexData->tangent[1] = ptrTangent->vec[1];
							ptrVertexData->tangent[2] = ptrTangent->vec[2];
							// Fjind UV
							ptrUV = (lsListItem_Vector2*)uv_list.Find_item( ptrFaceQuad->uv2 );
							ptrVertexData->uv[0] = ptrUV->vec[0];
							ptrVertexData->uv[1] = ptrUV->vec[1];

							// Remember this returns the index
							int i = vertexdata_list.Add_ChkDupli( ptrVertexData );
							ptrQuadData = new lsListItem_index;
							if( ptrQuadData )
							{
								// set index
								ptrQuadData->setIndex( i );
								quaddata_list.Add_itematEnd( ptrQuadData );
							}
						} // if( ptrVertexData )

						// V3
						ptrVertexData = new lsListItem_vertexdata;
						if( ptrVertexData )
						{
							// Look up and fill out data
							// Find Vertex
							ptrVertex = (lsListItem_Vector3*)vertex_list.Find_item( ptrFaceQuad->v3 );
							ptrVertexData->vertex[0] = ptrVertex->vec[0];
							ptrVertexData->vertex[1] = ptrVertex->vec[1];
							ptrVertexData->vertex[2] = ptrVertex->vec[2];
							// Find Normal
							ptrNormal = (lsListItem_Vector3*)normal_list.Find_item( ptrFaceQuad->n3 );
							ptrVertexData->normal[0] = ptrNormal->vec[0];
							ptrVertexData->normal[1] = ptrNormal->vec[1];
							ptrVertexData->normal[2] = ptrNormal->vec[2];
							// Find Tangent
							ptrTangent = (lsListItem_Vector3*)tangent_list.Find_item( ptrFaceQuad->n3 ); // Tangents use the same index as Normals
							ptrVertexData->tangent[0] = ptrTangent->vec[0];
							ptrVertexData->tangent[1] = ptrTangent->vec[1];
							ptrVertexData->tangent[2] = ptrTangent->vec[2];
							// Find UV
							ptrUV = (lsListItem_Vector2*)uv_list.Find_item( ptrFaceQuad->uv3 );
							ptrVertexData->uv[0] = ptrUV->vec[0];
							ptrVertexData->uv[1] = ptrUV->vec[1];

							// Remember this returns the index
							int i = vertexdata_list.Add_ChkDupli( ptrVertexData );
							ptrQuadData = new lsListItem_index;
							if( ptrQuadData )
							{
								// set index
								ptrQuadData->setIndex( i );
								quaddata_list.Add_itematEnd( ptrQuadData );
							}
						} // if( ptrVertexData )

						// V4
						ptrVertexData = new lsListItem_vertexdata;
						if( ptrVertexData )
						{
							// Look up and fill out data
							// Find Vertex
							ptrVertex = (lsListItem_Vector3*)vertex_list.Find_item( ptrFaceQuad->v4 );
							ptrVertexData->vertex[0] = ptrVertex->vec[0];
							ptrVertexData->vertex[1] = ptrVertex->vec[1];
							ptrVertexData->vertex[2] = ptrVertex->vec[2];
							// Find Normal
							ptrNormal = (lsListItem_Vector3*)normal_list.Find_item( ptrFaceQuad->n4 );
							ptrVertexData->normal[0] = ptrNormal->vec[0];
							ptrVertexData->normal[1] = ptrNormal->vec[1];
							ptrVertexData->normal[2] = ptrNormal->vec[2];
							// Find Tangent
							ptrTangent = (lsListItem_Vector3*)tangent_list.Find_item( ptrFaceQuad->n4 ); // Tangents use the same index as Normals
							ptrVertexData->tangent[0] = ptrTangent->vec[0];
							ptrVertexData->tangent[1] = ptrTangent->vec[1];
							ptrVertexData->tangent[2] = ptrTangent->vec[2];
							// Find UV
							ptrUV = (lsListItem_Vector2*)uv_list.Find_item( ptrFaceQuad->uv4 );
							ptrVertexData->uv[0] = ptrUV->vec[0];
							ptrVertexData->uv[1] = ptrUV->vec[1];

							// Remember this returns the index
							int i = vertexdata_list.Add_ChkDupli( ptrVertexData );
							ptrQuadData = new lsListItem_index;
							if( ptrQuadData )
							{
								// set index
								ptrQuadData->setIndex( i );
								quaddata_list.Add_itematEnd( ptrQuadData );
							}
						} // if( ptrVertexData )

						ptrFaceQuad = (lsListItem_faceQuad*)ptrFaceQuad->getNext_item();
					} // while( ptrFaceQuad )

					// #############################################
					// cycle tri faces
					// #############################################

					ptrFaceTri = (lsListItem_faceTri*)triindex_list.getItem_list();
					while( ptrFaceTri )
					{
						// Add Triangle
						// V1
						ptrVertexData = new lsListItem_vertexdata;
						if( ptrVertexData )
						{
							// Look up and fill out data
							// Find Vertex
							ptrVertex = (lsListItem_Vector3*)vertex_list.Find_item( ptrFaceTri->v1 );
							ptrVertexData->vertex[0] = ptrVertex->vec[0];
							ptrVertexData->vertex[1] = ptrVertex->vec[1];
							ptrVertexData->vertex[2] = ptrVertex->vec[2];
							// Find Normal
							ptrNormal = (lsListItem_Vector3*)normal_list.Find_item( ptrFaceTri->n1 );
							ptrVertexData->normal[0] = ptrNormal->vec[0];
							ptrVertexData->normal[1] = ptrNormal->vec[1];
							ptrVertexData->normal[2] = ptrNormal->vec[2];
							// Find Tangent
							ptrTangent = (lsListItem_Vector3*)tangent_list.Find_item( ptrFaceTri->n1 ); // Tangents use the same index as Normals
							ptrVertexData->tangent[0] = ptrTangent->vec[0];
							ptrVertexData->tangent[1] = ptrTangent->vec[1];
							ptrVertexData->tangent[2] = ptrTangent->vec[2];
							// Find UV
							ptrUV = (lsListItem_Vector2*)uv_list.Find_item( ptrFaceTri->uv1 );
							ptrVertexData->uv[0] = ptrUV->vec[0];
							ptrVertexData->uv[1] = ptrUV->vec[1];

							// Remember this returns the index
							int i = vertexdata_list.Add_ChkDupli( ptrVertexData );
							ptrTriData = new lsListItem_index;
							if( ptrTriData )
							{
								// set index
								ptrTriData->setIndex( i );
								tridata_list.Add_itematEnd( ptrTriData );
							}
						}

						// V2
						ptrVertexData = new lsListItem_vertexdata;
						if( ptrVertexData )
						{
							// Look up and fill out data
							// Find Vertex
							ptrVertex = (lsListItem_Vector3*)vertex_list.Find_item( ptrFaceTri->v2 );
							ptrVertexData->vertex[0] = ptrVertex->vec[0];
							ptrVertexData->vertex[1] = ptrVertex->vec[1];
							ptrVertexData->vertex[2] = ptrVertex->vec[2];
							// Find Normal
							ptrNormal = (lsListItem_Vector3*)normal_list.Find_item( ptrFaceTri->n2 );
							ptrVertexData->normal[0] = ptrNormal->vec[0];
							ptrVertexData->normal[1] = ptrNormal->vec[1];
							ptrVertexData->normal[2] = ptrNormal->vec[2];
							// Find Tangent
							ptrTangent = (lsListItem_Vector3*)tangent_list.Find_item( ptrFaceTri->n2 ); // Tangents use the same index as Normals
							ptrVertexData->tangent[0] = ptrTangent->vec[0];
							ptrVertexData->tangent[1] = ptrTangent->vec[1];
							ptrVertexData->tangent[2] = ptrTangent->vec[2];
							// Find UV
							ptrUV = (lsListItem_Vector2*)uv_list.Find_item( ptrFaceTri->uv2 );
							ptrVertexData->uv[0] = ptrUV->vec[0];
							ptrVertexData->uv[1] = ptrUV->vec[1];

							// Remember this returns the index
							int i = vertexdata_list.Add_ChkDupli( ptrVertexData );
							ptrTriData = new lsListItem_index;
							if( ptrTriData )
							{
								// set index
								ptrTriData->setIndex( i );
								tridata_list.Add_itematEnd( ptrTriData );
							}
						}

						// V3
						ptrVertexData = new lsListItem_vertexdata;
						if( ptrVertexData )
						{
							// Look up and fill out data
							// Find Vertex
							ptrVertex = (lsListItem_Vector3*)vertex_list.Find_item( ptrFaceTri->v3 );
							ptrVertexData->vertex[0] = ptrVertex->vec[0];
							ptrVertexData->vertex[1] = ptrVertex->vec[1];
							ptrVertexData->vertex[2] = ptrVertex->vec[2];
							// Find Normal
							ptrNormal = (lsListItem_Vector3*)normal_list.Find_item( ptrFaceTri->n3 );
							ptrVertexData->normal[0] = ptrNormal->vec[0];
							ptrVertexData->normal[1] = ptrNormal->vec[1];
							ptrVertexData->normal[2] = ptrNormal->vec[2];
							// Find Tangent
							ptrTangent = (lsListItem_Vector3*)tangent_list.Find_item( ptrFaceTri->n3 ); // Tangents use the same index as Normals
							ptrVertexData->tangent[0] = ptrTangent->vec[0];
							ptrVertexData->tangent[1] = ptrTangent->vec[1];
							ptrVertexData->tangent[2] = ptrTangent->vec[2];
							// Find UV
							ptrUV = (lsListItem_Vector2*)uv_list.Find_item( ptrFaceTri->uv3 );
							ptrVertexData->uv[0] = ptrUV->vec[0];
							ptrVertexData->uv[1] = ptrUV->vec[1];

							// Remember this returns the index
							int i = vertexdata_list.Add_ChkDupli( ptrVertexData );
							ptrTriData = new lsListItem_index;
							if( ptrTriData )
							{
								// set index
								ptrTriData->setIndex( i );
								tridata_list.Add_itematEnd( ptrTriData );
							}
						}

						ptrFaceTri = (lsListItem_faceTri*)ptrFaceTri->getNext_item();
					} // while( ptrFaceTri )

#ifdef VORTX_DEBUG_BATCH
					// debug pre-batch data
					cout << "***********************************************" << endl;
					cout << "Pre-Batch build check" << endl;
					cout << "Vertex data count : " << vertexdata_list.getItem_count() << endl;
					vertexdata_list.Debug();
					cout << "tridata index count : " << tridata_list.getItem_count() << endl;
					tridata_list.Debug();
					cout << "quaddata index count : " << quaddata_list.getItem_count() << endl;
					quaddata_list.Debug();
#endif

					// build our batch data ##################

					// alloc memory - convert quads into tris
					// debug
#ifdef VORTX_DEBUG_BATCH
					cout << "***********************************************" << endl;
					cout << "Build batch mesh : convert quads to triangles" << endl;
#endif
					batch->Init_data( vertexdata_list.getItem_count(), tridata_list.getItem_count() + (quaddata_list.getItem_count()*1.5) );

					// Add vertex data
					ptrVertexData = (lsListItem_vertexdata *)vertexdata_list.getItem_list();
					while( ptrVertexData )
					{
						batch->Add_VertexData( ptrVertexData );
						ptrVertexData = (lsListItem_vertexdata *)ptrVertexData->getNext_item();
					}
					// Add tri index data
					ptrTriData = (lsListItem_index *)tridata_list.getItem_list();
					while( ptrTriData )
					{
						batch->Add_Index( ptrTriData->getIndex() );
						ptrTriData = (lsListItem_index *)ptrTriData->getNext_item();
					}
					// Add quad index data ### convert to tris
					ptrQuadData = (lsListItem_index *)quaddata_list.getItem_list();
					while( ptrQuadData )
					{
						// get index data
						int q[4];
						q[0] = ptrQuadData->getIndex();
						ptrQuadData = (lsListItem_index *)ptrQuadData->getNext_item();
						q[1] = ptrQuadData->getIndex();
						ptrQuadData = (lsListItem_index *)ptrQuadData->getNext_item();
						q[2] = ptrQuadData->getIndex();
						ptrQuadData = (lsListItem_index *)ptrQuadData->getNext_item();
						q[3] = ptrQuadData->getIndex();
						ptrQuadData = (lsListItem_index *)ptrQuadData->getNext_item();

						// Triangle 1
						batch->Add_Index( q[0] );
						batch->Add_Index( q[1] );
						batch->Add_Index( q[2] );

						// Triangle 2
						batch->Add_Index( q[2] );
						batch->Add_Index( q[3] );
						batch->Add_Index( q[0] );

					} // while( ptrQuadData )

					// Load data into video memory
					batch->Copy_GLdata();
					addMem_used( batch->getMem_used() );
					
					// Clean up - data no longer needed
					vertex_list.Clear();
					normal_list.Clear();
					tangent_list.Clear();
					uv_list.Clear();
					// faces
					triindex_list.Clear();
					quadindex_list.Clear();

					// Clean up - Pass 2 data
					vertexdata_list.Clear();
					tridata_list.Clear();
					quaddata_list.Clear();

				} // if batch end

			} else if( list[0] == "bsphere" )
			{
				// Set the culling sphere
				if( batch )
				{
					batch->bSphere.center.x = convert.toFloat( list[1] );
					batch->bSphere.center.y = convert.toFloat( list[2] );
					batch->bSphere.center.z = convert.toFloat( list[3] );
					batch->bSphere.radius = convert.toFloat( list[4] );
				}

			} else if( list[0] == "vertex" )
			{
				ptrVertex = new lsListItem_Vector3;
				if( ptrVertex )
				{
					// # vertex index x y z
					ptrVertex->setIndex( convert.toFloat( list[1] ) );
					ptrVertex->vec[0] = convert.toFloat( list[2] );
					ptrVertex->vec[1] = convert.toFloat( list[3] );
					ptrVertex->vec[2] = convert.toFloat( list[4] );

					vertex_list.Add_item( ptrVertex );
				}

			} else if( list[0] == "normal" )
			{
				// normal index x y z
				ptrNormal = new lsListItem_Vector3;
				if( ptrNormal )
				{
					// normal index x y z
					ptrNormal->setIndex( convert.toFloat( list[1] ) );
					ptrNormal->vec[0] = convert.toFloat( list[2] );
					ptrNormal->vec[1] = convert.toFloat( list[3] );
					ptrNormal->vec[2] = convert.toFloat( list[4] );

					normal_list.Add_item( ptrNormal );
				}

			} else if( list[0] == "tangent" )
			{
				// tangent index x y z
				ptrTangent = new lsListItem_Vector3;
				if( ptrTangent )
				{
					// tangent index x y z
					ptrTangent->setIndex( convert.toFloat( list[1] ) );
					ptrTangent->vec[0] = convert.toFloat( list[2] );
					ptrTangent->vec[1] = convert.toFloat( list[3] );
					ptrTangent->vec[2] = convert.toFloat( list[4] );

					tangent_list.Add_item( ptrTangent );
				}

			} else if( list[0] == "uv" )
			{
				// uv index u v
				ptrUV = new lsListItem_Vector2;
				if( ptrUV )
				{
					// uv index u v
					ptrUV->setIndex( convert.toFloat( list[1] ) );
					ptrUV->vec[0] = convert.toFloat( list[2] );
					ptrUV->vec[1] = convert.toFloat( list[3] );

					uv_list.Add_item( ptrUV );
				}

			} else if( list[0] == "face_count" )
			{
				// face_count : TriCount QuadCount
				// Not used

			} else if( list[0] == "face" )
			{
				// ########################### Face
				// face FaceIndex VertexIndex/NormalIndex/UvIndex ...
				if( list.size() == 6 )
				{
					// Quads
					ptrFaceQuad = new lsListItem_faceQuad;
					if( ptrFaceQuad )
					{
						// index
						ptrFaceQuad->setIndex( convert.toInt( list[1] ) );

						// V1
						sublist.clear();
						convert.SplitString( list[2], '/', sublist );
						ptrFaceQuad->v1 = convert.toInt( sublist[0] );
						ptrFaceQuad->n1 = convert.toInt( sublist[1] );
						ptrFaceQuad->uv1 = convert.toInt( sublist[2] );

						// V2
						sublist.clear();
						convert.SplitString( list[3], '/', sublist );
						ptrFaceQuad->v2 = convert.toInt( sublist[0] );
						ptrFaceQuad->n2 = convert.toInt( sublist[1] );
						ptrFaceQuad->uv2 = convert.toInt( sublist[2] );

						// V3
						sublist.clear();
						convert.SplitString( list[4], '/', sublist );
						ptrFaceQuad->v3 = convert.toInt( sublist[0] );
						ptrFaceQuad->n3 = convert.toInt( sublist[1] );
						ptrFaceQuad->uv3 = convert.toInt( sublist[2] );

						// V4
						sublist.clear();
						convert.SplitString( list[5], '/', sublist );
						ptrFaceQuad->v4 = convert.toInt( sublist[0] );
						ptrFaceQuad->n4 = convert.toInt( sublist[1] );
						ptrFaceQuad->uv4 = convert.toInt( sublist[2] );

						quadindex_list.Add_item( ptrFaceQuad );
					}

				} else if( list.size() == 5 )
				{
					// Triangle
					ptrFaceTri = new lsListItem_faceTri;
					if( ptrFaceTri )
					{
						// index
						ptrFaceTri->setIndex( convert.toInt( list[1] ) );

						// V1
						sublist.clear();
						convert.SplitString( list[2], '/', sublist );
						ptrFaceTri->v1 = convert.toInt( sublist[0] );
						ptrFaceTri->n1 = convert.toInt( sublist[1] );
						ptrFaceTri->uv1 = convert.toInt( sublist[2] );

						// V2
						sublist.clear();
						convert.SplitString( list[3], '/', sublist );
						ptrFaceTri->v2 = convert.toInt( sublist[0] );
						ptrFaceTri->n2 = convert.toInt( sublist[1] );
						ptrFaceTri->uv2 = convert.toInt( sublist[2] );

						// V3
						sublist.clear();
						convert.SplitString( list[4], '/', sublist );
						ptrFaceTri->v3 = convert.toInt( sublist[0] );
						ptrFaceTri->n3 = convert.toInt( sublist[1] );
						ptrFaceTri->uv3 = convert.toInt( sublist[2] );

						triindex_list.Add_item( ptrFaceTri );
					}
				}

				// ##################################################3 Material
				
			} else if( list[0] == "diffuse" )
			{
				rx = convert.toFloat( list[1] );
				ry = convert.toFloat( list[2] );
				rz = convert.toFloat( list[3] );
				rw = convert.toFloat( list[4] );

				batch->m_albedo = lsVector4( rx, ry, rz, rw );

			} else if( list[0] == "specular" )
			{
				rx = convert.toFloat( list[1] );
				
				batch->m_roughness = rx;

			} else if( list[0] == "shininess" )
			{
				batch->m_metallic = convert.toFloat( list[1] );

			} else if( list[0] == "tex_d" )
			{
				// ##########################################3 Textures
				
				image = (lsAsset_Texture *)am->Find_Asset( list[1] );
				if( image )
				{
					if( image->getData() )
					{
						texture = (lsTexture*)image->getData();
						batch->tex_albedo = texture->getTexture();
					}
				} else
				{
					image = am->CreateAsset_Texture( list[1] );
					if( image )
					{
						// Alloc
						image->AllocAsset();

						if( image->getData() )
						{
							texture = (lsTexture*)image->getData();
							batch->tex_albedo = texture->getTexture();
						}

					}// if image
				}// if find asset

			} else if( list[0] == "tex_n" )
			{
				image = (lsAsset_Texture *)am->Find_Asset( list[1] );
				if( image )
				{
					if( image->getData() )
					{
						texture = (lsTexture*)image->getData();
						batch->tex_normal = texture->getTexture();
					}
				} else
				{
					image = am->CreateAsset_Texture( list[1] );
					if( image )
					{
						// Alloc
						image->AllocAsset();

						if( image->getData() )
						{
							texture = (lsTexture*)image->getData();
							batch->tex_normal = texture->getTexture();
						}

					}// if image
				}// if find asset

			} else if( list[0] == "tex_s" )
			{
				image = (lsAsset_Texture *)am->Find_Asset( list[1] );
				if( image )
				{
					if( image->getData() )
					{
						texture = (lsTexture*)image->getData();
						batch->tex_roughness_metallic = texture->getTexture();
					}
				} else
				{
					image = am->CreateAsset_Texture( list[1] );
					if( image )
					{
						// Alloc
						image->AllocAsset();

						if( image->getData() )
						{
							texture = (lsTexture*)image->getData();
							batch->tex_roughness_metallic = texture->getTexture();
						}

					}// if image
				}// if find asset

			} else if( list[0] == "shader" )
			{
				// shader
				shader = sm->Find_Shader( list[1] );
				if( shader )
				{
					batch->setShader( shader );
				} else
				{
					// Load a shader
					shader = new lsShader( getOgl() );
					if( shader )
					{
						shader->Load_Shader( list[1] );
						batch->setShader( shader );
						sm->Add_Shader( shader );
					}
				} // if find shader
			}

		} // if cmdline != ""
	} // while readline

	// Calc overall bsphere for mesh from each batch.
	if( batch_list )
	{
		// Not correct : only works for one batch FIX later.
		// Cycle through all batch bspheres, find center and max radius.
		bSphere.center.x =  batch_list->bSphere.center.x;
		bSphere.center.y = batch_list->bSphere.center.y;
		bSphere.center.z = batch_list->bSphere.center.z;
		bSphere.radius = batch_list->bSphere.radius;
	}

}

void lsMesh::LoadGLTF(const string& filename, lsAssetManager *am, lsShaderManager *sm)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	// Get executable path.
	lsDir dir;
	string basepath = dir.getexepath();
	
	std::string ext;
	size_t pos = filename.find_last_of(".");
	if (pos != std::string::npos) {
		ext = filename.substr(pos + 1);
	}
	
	bool ret = (ext == "glb") ? loader.LoadBinaryFromFile(&model, &err, &warn, basepath + filename)
							  : loader.LoadASCIIFromFile(&model, &err, &warn, basepath + filename);

	if (!warn.empty()) { std::cout << "glTF WARN: " << warn << std::endl; }
	if (!err.empty()) { std::cerr << "glTF ERR: " << err << std::endl; }
	if (!ret) { std::cerr << "Failed to load glTF: " << filename << std::endl; return; }

	lsVector3 min_v(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	lsVector3 max_v(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
	std::vector<lsVector3> all_vertices;

	for (const auto& mesh : model.meshes) {
		for (const auto& primitive : mesh.primitives) {
			lsBatch_Mesh* batch = new lsBatch_Mesh(getOgl());
			if (!batch) continue;

			// Attributes
			std::vector<lsListItem_vertexdata> vertexData;
			size_t vertexCount = 0;

			if (primitive.attributes.find("POSITION") == primitive.attributes.end()) {
				delete batch;
				continue;
			}
			
			const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
			vertexCount = posAccessor.count;
			vertexData.resize(vertexCount);
			
			{ // Position
				const tinygltf::BufferView& bufferView = model.bufferViews[posAccessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
				const unsigned char* data_ptr = &buffer.data[bufferView.byteOffset + posAccessor.byteOffset];
				int stride = posAccessor.ByteStride(bufferView);
				if (stride == 0) stride = 3 * sizeof(float);

				for (size_t i = 0; i < vertexCount; ++i) {
					const float* p = reinterpret_cast<const float*>(data_ptr + i * stride);
					vertexData[i].vertex[0] = p[0];
					vertexData[i].vertex[1] = p[1];
					vertexData[i].vertex[2] = p[2];

					min_v.x = std::min(min_v.x, p[0]);
					min_v.y = std::min(min_v.y, p[1]);
					min_v.z = std::min(min_v.z, p[2]);
					max_v.x = std::max(max_v.x, p[0]);
					max_v.y = std::max(max_v.y, p[1]);
					max_v.z = std::max(max_v.z, p[2]);
					all_vertices.emplace_back(p[0], p[1], p[2]);
				}
			}
			
			if (primitive.attributes.count("NORMAL")) {
				const auto& accessor = model.accessors[primitive.attributes.at("NORMAL")];
				const auto& bufferView = model.bufferViews[accessor.bufferView];
				const auto& buffer = model.buffers[bufferView.buffer];
				const unsigned char* data_ptr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
				int stride = accessor.ByteStride(bufferView);
				if (stride == 0) stride = 3 * sizeof(float);
				for (size_t i = 0; i < vertexCount; ++i) {
					const float* p = reinterpret_cast<const float*>(data_ptr + i * stride);
					vertexData[i].normal[0] = p[0];
					vertexData[i].normal[1] = p[1];
					vertexData[i].normal[2] = p[2];
				}
			}
			
			if (primitive.attributes.count("TEXCOORD_0")) {
				const auto& accessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
				const auto& bufferView = model.bufferViews[accessor.bufferView];
				const auto& buffer = model.buffers[bufferView.buffer];
				const unsigned char* data_ptr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
				int stride = accessor.ByteStride(bufferView);
				if (stride == 0) stride = 2 * sizeof(float);
				for (size_t i = 0; i < vertexCount; ++i) {
					const float* p = reinterpret_cast<const float*>(data_ptr + i * stride);
					vertexData[i].uv[0] = p[0];
					vertexData[i].uv[1] = p[1];
				}
			}

			if (primitive.attributes.count("TANGENT")) {
				const auto& accessor = model.accessors[primitive.attributes.at("TANGENT")];
				const auto& bufferView = model.bufferViews[accessor.bufferView];
				const auto& buffer = model.buffers[bufferView.buffer];
				const unsigned char* data_ptr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
				int stride = accessor.ByteStride(bufferView);
				if (stride == 0) stride = 4 * sizeof(float);
				for (size_t i = 0; i < vertexCount; ++i) {
					const float* p = reinterpret_cast<const float*>(data_ptr + i * stride);
					vertexData[i].tangent[0] = p[0];
					vertexData[i].tangent[1] = p[1];
					vertexData[i].tangent[2] = p[2];
				}
			}

			// Indices
			const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
			const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
			const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];
			const unsigned char* indexData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];
			
			batch->Init_data(vertexCount, indexAccessor.count);
			for(const auto& vd : vertexData) {
				lsListItem_vertexdata temp_vd = vd;
				batch->Add_VertexData(&temp_vd);
			}
			
			if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
				const uint32_t* p = reinterpret_cast<const uint32_t*>(indexData);
				for (size_t i = 0; i < indexAccessor.count; ++i) batch->Add_Index(p[i] + 1);
			} else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
				const uint16_t* p = reinterpret_cast<const uint16_t*>(indexData);
				for (size_t i = 0; i < indexAccessor.count; ++i) batch->Add_Index(p[i] + 1);
			} else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
				const uint8_t* p = reinterpret_cast<const uint8_t*>(indexData);
				for (size_t i = 0; i < indexAccessor.count; ++i) batch->Add_Index(p[i] + 1);
			}

			batch->Copy_GLdata();
			addMem_used(batch->getMem_used());

			if (primitive.material >= 0) {
				const tinygltf::Material& mat = model.materials[primitive.material];
				
				const auto& pbr = mat.pbrMetallicRoughness;
				batch->m_albedo = lsVector4(pbr.baseColorFactor[0], pbr.baseColorFactor[1], pbr.baseColorFactor[2], pbr.baseColorFactor[3]);
				batch->m_roughness = pbr.roughnessFactor;
				batch->m_metallic = pbr.metallicFactor;
				
				std::string shader_name_suffix = "";

				auto get_texture_id = [&](int tex_idx) -> GLuint {
					if (tex_idx < 0 || static_cast<size_t>(tex_idx) >= model.textures.size()) return 0;
					const auto& tex = model.textures[tex_idx];
					if (tex.source < 0 || static_cast<size_t>(tex.source) >= model.images.size()) return 0;
					const auto& img = model.images[tex.source];
					
					if (!img.uri.empty() && !tinygltf::IsDataURI(img.uri)) {
						std::string base_dir = filename.substr(0, filename.find_last_of("/\\"));
						std::string tex_path = base_dir + "/" + urldecode(img.uri);

						lsAsset_Texture *image_asset = (lsAsset_Texture *)am->Find_Asset(tex_path);
						if (!image_asset) {
							image_asset = am->CreateAsset_Texture(tex_path);
							image_asset->AllocAsset();
						}
						if (image_asset && image_asset->getData()) {
							return static_cast<lsTexture*>(image_asset->getData())->getTexture();
						}
					} else if (!img.image.empty()) {
						GLuint tex_id = 0;
						glGenTextures(1, &tex_id);
						glBindTexture(GL_TEXTURE_2D, tex_id);
						GLenum format = (img.component == 4) ? GL_RGBA : GL_RGB;
						
						if (img.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
							GLenum internal_format = (img.component == 4) ? GL_RGBA16 : GL_RGB16;
							glTexImage2D(GL_TEXTURE_2D, 0, internal_format, img.width, img.height, 0, format, GL_UNSIGNED_SHORT, img.image.data());
						} else {
							GLenum internal_format = (img.component == 4) ? GL_SRGB8_ALPHA8 : GL_SRGB8;
							glTexImage2D(GL_TEXTURE_2D, 0, internal_format, img.width, img.height, 0, format, GL_UNSIGNED_BYTE, img.image.data());
						}
						
						glGenerateMipmap(GL_TEXTURE_2D);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glBindTexture(GL_TEXTURE_2D, 0);
						return tex_id;
					}
					return 0;
				};

				if (pbr.baseColorTexture.index > -1) {
					batch->tex_albedo = get_texture_id(pbr.baseColorTexture.index);
					if (batch->tex_albedo != 0) shader_name_suffix += "d";
				}
				if (mat.normalTexture.index > -1) {
					batch->tex_normal = get_texture_id(mat.normalTexture.index);
					if (batch->tex_normal != 0) shader_name_suffix += "n";
				}
				
				std::string shader_name = "light_tex_";
				if (shader_name_suffix.empty()) {
					shader_name += "none";
				} else {
					shader_name += shader_name_suffix;
				}
				
				lsShader* shader = sm->Find_Shader(shader_name);
				if (!shader) {
					shader = new lsShader(getOgl());
					shader->Load_Shader(shader_name);
					if(shader->getKind() != ShaderKind::None)
					{
						sm->Add_Shader(shader);
					} else
					{
						delete shader;
						shader = sm->Find_Shader("light_tex_none");
					}
				}
				batch->setShader(shader);
			} else
			{
				 lsShader* shader = sm->Find_Shader("light_tex_none");
				 batch->setShader(shader);
			}

			Add_Batch(batch);
		}
	}

	// Set bounding sphere.
	if (! all_vertices.empty())
	{
		bSphere.center = (min_v + max_v) * 0.5f;
		bSphere.radius = 0.0f;
		for(const auto& vert : all_vertices)
		{
			float dist = (vert - bSphere.center).length();
			if(dist > bSphere.radius)
			{
				bSphere.radius = dist;
			}
		}
	} else
	{
		bSphere.center = lsVector3(0,0,0);
		bSphere.radius = 0.0f;
	}
}

