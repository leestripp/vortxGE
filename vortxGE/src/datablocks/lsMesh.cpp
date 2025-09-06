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


lsMesh::lsMesh( lsOpenGL *opengl ) : lsData()
{
    ogl = opengl;
    batch_list = NULL;
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

                batch->diffuse = lsVector4( rx, ry, rz, rw );

            } else if( list[0] == "specular" )
            {
                rx = convert.toFloat( list[1] );
                ry = convert.toFloat( list[2] );
                rz = convert.toFloat( list[3] );
                rw = convert.toFloat( list[4] );

                batch->specular = lsVector4( rx, ry, rz, rw );

            } else if( list[0] == "ambiant" )
            {
                rx = convert.toFloat( list[1] );
                ry = convert.toFloat( list[2] );
                rz = convert.toFloat( list[3] );
                rw = convert.toFloat( list[4] );

                batch->ambiant = lsVector4( rx, ry, rz, rw );

            } else if( list[0] == "shininess" )
            {
                batch->shininess = convert.toFloat( list[1] );

            } else if( list[0] == "transparent" )
            {
                batch->transparent = convert.toFloat( list[1] );

            } else if( list[0] == "glow" )
            {
                batch->glow = convert.toFloat( list[1] );

                // ##########################################3 Textures
            } else if( list[0] == "tex_d" )
            {
                image = (lsAsset_Texture *)am->Find_Asset( list[1] );
                if( image )
                {
                    if( image->getData() )
                    {
                        texture = (lsTexture*)image->getData();
                        batch->tex_diffuse = texture->getTexture();
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
                            batch->tex_diffuse = texture->getTexture();
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
                        batch->tex_specular = texture->getTexture();
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
                            batch->tex_specular = texture->getTexture();
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

