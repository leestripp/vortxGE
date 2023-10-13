/*
 * Copyright (C) 2015 by Lee A. Stripp <leestripp@gmail.com>
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s):
 *
 * ***** END GPL LICENSE BLOCK *****
 */

// filesystem
#include "../filesystem/lsFileIO.h"

// convert
#include "../convert/lsString.h"

// Math
#include "../math/lsMath.h"
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

#include "lsMesh.h"
#include "lsMesh_Vertex.h"
#include "lsmesh_normal.h"
#include "lsmesh_uv.h"
#include "lsMesh_Face.h"
#include "lsMesh_FaceGroup.h"
#include "lsMaterial.h"


lsMesh::lsMesh()
{
    vertex_list = NULL;
    group_list = NULL;
    uv_list = NULL;
    normals_list = NULL;
    material_list = NULL;

    vertex_count = 0;
    normal_count = 0;
    uv_count = 0;
}

lsMesh::~lsMesh()
{
    // cleanup
    if( material_list ) delete material_list;
    if( group_list ) delete group_list;
    if( normals_list ) delete normals_list;
    if( uv_list ) delete uv_list;
    if( vertex_list ) delete vertex_list;
}

/********************************************************************
 * Load Mesh
 ********************************************************************/

bool lsMesh::Load_Mesh( const string& filename )
{
    // file
    cout << "***** Load Mesh : " << filename << endl;

    lsFileIO meshfile;
    if( ! meshfile.open( filename, LSIO_LINE_READ ) )
    {
        cout << "ERROR: Cant open file..." << endl;
        return false;
    }

    // pointers
    lsMesh_Vertex* vertex = NULL;
    lsMesh_UV* uv = NULL;
    lsMesh_Normal *normal = NULL;
    lsMesh_Face* face = NULL;
    lsMesh_FaceGroup* facegroup = NULL;
    lsMaterial* material = NULL;

    // indexes start at 1 in OBJ
    int vertex_index = 1;
    int normal_index = 1;
    int uv_index = 1;
    // counters
    vertex_count = 0;
    uv_count = 0;
    normal_count = 0;
    // options
    bool smooth_shade = true;

    // Text control
    vector<string> cmdlist;
    vector<string> flist;
    string cmdline;
    lsString convert;
    float rx, ry, rz;

    // read a line
    while ( meshfile.readline() )
    {
        cmdline = meshfile.getLine();
        if ( cmdline != "" )
        {
            cmdlist.clear();
            // split line into parts
            convert.SplitString( cmdline, ' ', cmdlist );

            if ( cmdlist[0] == "v" ) // vertex
            {
                // vertex
                rx = convert.toFloat( cmdlist[1] );
                ry = convert.toFloat( cmdlist[2] );
                rz = convert.toFloat( cmdlist[3] );

                vertex = new lsMesh_Vertex();
                if ( vertex )
                {
                    vertex->setVertexData( rx, ry, rz );
                    Add_Vertex( vertex_index, vertex );
                    vertex_index++;
                }
                vertex_count++;

            } else if ( cmdlist[0] == "vt" ) // UVs
            {
                // vertex
                rx = convert.toFloat( cmdlist[1] );
                ry = convert.toFloat( cmdlist[2] );

                uv = new lsMesh_UV;
                if( uv )
                {
                    uv->Set_UV( rx, ry );
                    Add_UV( uv_index, uv );
                    uv_index++;
                }
                uv_count++;

            }else if ( cmdlist[0] == "vn" ) // normal
            {
                // normal
                rx = convert.toFloat( cmdlist[1] );
                ry = convert.toFloat( cmdlist[2] );
                rz = convert.toFloat( cmdlist[3] );

                normal = new lsMesh_Normal;
                if ( normal )
                {
                    normal->Set_Normal( rx, ry, rz );
                    Add_Normal( normal_index, normal );
                    normal_index++;
                }
                normal_count++;

            } else if ( cmdlist[0] == "mtllib" ) // material file
            {
                // reletive
                string path = "./";
                string mtlfile = path + cmdlist[1];
                Load_Material( mtlfile );

            } else if ( cmdlist[0] == "usemtl" ) // use material
            {
                // workaround bug in Blender obj exporter
                if( facegroup )
                {
                    // Blender exports UV islands as seperate materials even if you only have one material applied.
                } else
                {
                    material = Find_Material( cmdlist[1] );
                    // Add to a new group
                    facegroup = new lsMesh_FaceGroup;
                    if( facegroup )
                    {
                        // smooth shading
                        facegroup->setSmooth_shade( smooth_shade );
                        // set material
                        facegroup->setMaterial( material );
                        Add_FaceGroup( facegroup );
                    }
                } // if( facegroup )

            } else if ( cmdlist[0] == "s" ) // smooth shade
            {
                if( cmdlist[1] == "off" )
                    smooth_shade = false;
                else
                    smooth_shade = true;

            } else if ( cmdlist[0] == "f" ) // add face
            {
                face = new lsMesh_Face;
                if(( face )&&( facegroup ))
                {
                    face->setFace_type( LSFT_TRIANGLE );

                    // V1
                    flist.clear();
                    convert.SplitString( cmdlist[1], '/', flist );
                    face->setV1( Find_Vertex( convert.toInt( flist[0] )));
                    face->setUv1( Find_UV( convert.toInt( flist[1] ) ));
                    face->setNorm1( Find_Normal( convert.toInt( flist[2] )) );

                    // V2
                    flist.clear();
                    convert.SplitString( cmdlist[2], '/', flist );
                    face->setV2( Find_Vertex( convert.toInt( flist[0] )));
                    face->setUv2( Find_UV( convert.toInt( flist[1] ) ));
                    face->setNorm2( Find_Normal( convert.toInt( flist[2] )) );

                    // V3
                    flist.clear();
                    convert.SplitString( cmdlist[3], '/', flist );
                    face->setV3( Find_Vertex( convert.toInt( flist[0] )));
                    face->setUv3( Find_UV( convert.toInt( flist[1] ) ));
                    face->setNorm3( Find_Normal( convert.toInt( flist[2] )) );

                    // Quad
                    if ( cmdlist.size() == 5 )
                    {
                        flist.clear();
                        convert.SplitString( cmdlist[4], '/', flist );
                        face->setV4( Find_Vertex( convert.toInt( flist[0] )));
                        face->setUv4( Find_UV( convert.toInt( flist[1] ) ));
                        face->setNorm4( Find_Normal( convert.toInt( flist[2] )) );

                        // Quad
                        face->setFace_type( LSFT_QUAD );
                    }
                    // Add the face
                    if(facegroup) facegroup->Add_Face( face );
                } else
                {
                    // ERROR
                    cout << "ERROR : No group to add face to" << endl;
                }// if face

            }

        } // if ( cmdline != "" )
    } // while ( meshfile.readline() )

    // debug
    cout << "***** Mesh Stats *****" << endl;
    cout << "Vertex count : " << vertex_count << endl;
    cout << "UV     count : " << uv_count << endl;
    cout << "Normal count : " << normal_count << endl;
    // show facegroup details
    meshStats();

    if( vertex_count )
    {
        // Calc Volume sphere
        bSphere.center = FindCenter();
        bSphere.radius = FindRadius();

        // Build tangents
        buildTangents();
    }
    return true;
}

void lsMesh::Load_Material(const string &filename)
{
    // file
    cout << "Load Material : " << filename << endl;

    lsFileIO file;
    if( ! file.open( filename, LSIO_LINE_READ ) )
    {
        cout << "ERROR: Cant open file : " << filename << endl;
        return;
    }

    // Material
    lsMaterial *material = NULL;

    // Text control
    vector<string> cmdlist;
    vector<string> flist;
    string cmdline;
    lsString convert;
    float rx, ry, rz;

    // read a line
    while ( file.readline() )
    {
        cmdline = file.getLine();
        if ( cmdline != "" )
        {
            cmdlist.clear();
            // split line into parts
            convert.SplitString( cmdline, ' ', cmdlist );

            if ( cmdlist[0] == "newmtl" ) // material
            {
                material = new lsMaterial();
                if ( material )
                {
                    material->setName( cmdlist[1] );
                    // Add it
                    Add_Material( material );
                }

            } else if ( cmdlist[0] == "Ka" ) // ambient
            {
                if( material )
                {
                    // values
                    rx = convert.toFloat( cmdlist[1] );
                    ry = convert.toFloat( cmdlist[2] );
                    rz = convert.toFloat( cmdlist[3] );

                    material->setAmbient( rx, ry, rz );
                }
            } else if ( cmdlist[0] == "Kd" ) // diffuse
            {
                if( material )
                {
                    // values
                    rx = convert.toFloat( cmdlist[1] );
                    ry = convert.toFloat( cmdlist[2] );
                    rz = convert.toFloat( cmdlist[3] );

                    material->setDiffuse( rx, ry, rz );
                }
            } else if ( cmdlist[0] == "Ks" ) // specular
            {
                if( material )
                {
                    // values
                    rx = convert.toFloat( cmdlist[1] );
                    ry = convert.toFloat( cmdlist[2] );
                    rz = convert.toFloat( cmdlist[3] );

                    material->setSpecular( rx, ry, rz );
                }
            } else if ( cmdlist[0] == "d" ) // transparent
            {
                if( material )
                {
                    // values
                    rx = convert.toFloat( cmdlist[1] );

                    material->setTransparent( rx );
                }
            } else if ( cmdlist[0] == "Ns" ) // shininess exponent
            {
                if( material )
                {
                    rx = convert.toFloat( cmdlist[1] );

                    material->setShininess( rx );
                }

            } else if ( cmdlist[0] == "map_Kd" ) // tex_d
            {
                if( material )
                {
                    // values
                    material->setTex_d( cmdlist[1] );
                }
            } else if ( cmdlist[0] == "map_Bump" ) // bump
            {
                if( material )
                {
                    // values
                    material->setTex_n( cmdlist[1] );
                }
            } else if ( cmdlist[0] == "map_Ks" ) // specular
            {
                if( material )
                {
                    // values
                    material->setTex_s( cmdlist[1] );
                }
            }

        } // if ( cmdline != "" )
    } // while ( file.readline() )

    // debug
    material = material_list;
    while( material )
    {
        cout << "Materials found" << endl;
        cout << "Material : " << material->getName() << endl;

        material = material->getNext_material();
    }
}


lsVector3 lsMesh::FindCenter()
{
    lsVector3 res;
    lsVector3 vec;
    lsMesh_Vertex* vert = vertex_list;
    float* v;

    while ( vert )
    {
        v = vert->getVertexData();
        vec = lsVector3( v[0], v[1], v[2] );
        // Add
        res = res + vec;
        vert = vert->getNext_vertex();
    }
    return res / (float)vertex_count;
}

float lsMesh::FindRadius()
{
    lsVector3 vec;
    float* v;
    float res = 0.0f;
    float dist;
    lsMesh_Vertex* vert = vertex_list;

    while ( vert )
    {
        v = vert->getVertexData();
        vec = lsVector3( v[0], v[1], v[2] );

        dist = bSphere.distance( vec );
        if ( dist > res )
            res = dist;

        vert = vert->getNext_vertex();
    }
    return res;
}



/***************************************************************************************
 * Data
 ***************************************************************************************/

void lsMesh::Add_Vertex( int index, lsMesh_Vertex* vertex )
{
    vertex->setNext_vertex( vertex_list );
    vertex->setIndex( index );
    vertex_list = vertex;
}

void lsMesh::Add_Normal(int index, lsMesh_Normal *normal)
{
    normal->setNext_normal( normals_list );
    normal->setNindex( index );
    normals_list = normal;
}

void lsMesh::Add_UV(int index, lsMesh_UV *uv)
{
    uv->setNext_uv( uv_list );
    uv->setUv_index( index );
    uv_list = uv;
}

void lsMesh::Add_FaceGroup( lsMesh_FaceGroup* group )
{
    group->setNext_group( group_list );
    group_list = group;
}

void lsMesh::Add_Material(lsMaterial *material)
{
    material->setNext_material( material_list );
    material_list = material;
}

lsMesh_Vertex* lsMesh::getVertex_list()
{
    return vertex_list;
}


/**********************************************************************************
 * Tangents
 **********************************************************************************/

void lsMesh::buildTangents()
{
    cout << "Build Tangent vectors - ";
    cout << "Vertex Count : " << vertex_count << endl;

    lsMath math;
    lsMesh_FaceGroup* group;
    lsMesh_Face* face;

    lsVector3 T, B, N;
    lsVector3 InvTangent, InvBiTangent, InvNormal;

    group = group_list;
    while( group )
    {
        face = group->getFace_list();
        while( face )
        {
            // vertex 1
            computeTangentBasis( face->getNorm1(), face->getV1(), face->getV2(), face->getV3(), face->getUv1(), face->getUv2(), face->getUv3() );
            // vertex 2
            computeTangentBasis( face->getNorm2(), face->getV2(), face->getV3(), face->getV1(), face->getUv2(), face->getUv3(), face->getUv1() );
            // vertex 3
            computeTangentBasis( face->getNorm3(), face->getV3(), face->getV1(), face->getV2(), face->getUv3(), face->getUv1(), face->getUv2() );

            if( face->getFace_type() == LSFT_QUAD )
            {
                // vertex 4
                computeTangentBasis( face->getNorm4(), face->getV4(), face->getV3(), face->getV2(), face->getUv4(), face->getUv3(), face->getUv2() );
            }
            face = face->getNext_face();
        } // while face

        group = group->getNext_group();
    } // while group

    cout << "Build Tangents done..." << endl;
}

void lsMesh::computeTangentBasis( lsMesh_Normal *normal, lsMesh_Vertex* vertex1, lsMesh_Vertex* vertex2, lsMesh_Vertex* vertex3, lsMesh_UV* UV1, lsMesh_UV* UV2, lsMesh_UV* UV3 )
{
    lsMath math;
    float *V1, *V2, *V3;
    float *U1, *U2, *U3;
    lsVector3 T, B, N;

    V1 = vertex1->getVertexData();
    V2 = vertex2->getVertexData();
    V3 = vertex3->getVertexData();
    // UVs
    U1 = UV1->Get_UV();
    U2 = UV2->Get_UV();
    U3 = UV3->Get_UV();

    lsVector3 v1 = lsVector3( V1[0], V1[1], V1[2] );
    lsVector3 v2 = lsVector3( V2[0], V2[1], V2[2] );
    lsVector3 v3 = lsVector3( V3[0], V3[1], V3[2] );

    lsVector2 w1 = lsVector2( U1[0], U1[1] );
    lsVector2 w2 = lsVector2( U2[0], U2[1] );
    lsVector2 w3 = lsVector2( U3[0], U3[1] );

    // Vectors
    float x1 = v2.x - v1.x;
    float x2 = v3.x - v1.x;
    float y1 = v2.y - v1.y;
    float y2 = v3.y - v1.y;
    float z1 = v2.z - v1.z;
    float z2 = v3.z - v1.z;
    // UV vectors
    float s1 = w2.x - w1.x;
    float s2 = w3.x - w1.x;
    float t1 = w2.y - w1.y;
    float t2 = w3.y - w1.y;

    float r = 1.0 / (s1 * t2 - s2 * t1);
    if( isnan(r) ) cout << "Warnning : Bad scale value (r = NaN)" << endl;
    T = lsVector3((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                  (t2 * z1 - t1 * z2) * r);
    B = lsVector3((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                  (s1 * z2 - s2 * z1) * r);
    N = math.Cross( T, B ); //Cross product! for testing

    // Set Normal data
    normal->Set_Tangent( T.x, T.y, T.z );
    normal->Set_Binormal( B.x, B.y, B.z );
}


/******************************************************
 * Find data
 *******************************************************/

lsMesh_Vertex* lsMesh::Find_Vertex( int index )
{
    lsMesh_Vertex* ptr;

    // Debug
    cout << "Looking for vertex : " << index << endl;

    ptr = vertex_list;
    while ( ptr )
    {
        if ( ptr->getIndex() == index ) break;
        ptr = ptr->getNext_vertex();
    }
    // Debug
    if ( ptr )  cout << "Found vertex : " << ptr->getIndex() << endl;

    return ptr;
}

lsMesh_Normal *lsMesh::Find_Normal(int index)
{
    lsMesh_Normal* ptr;

    // Debug
    cout << "Looking for normal : " << index << endl;

    ptr = normals_list;
    while ( ptr )
    {
        if ( ptr->getNindex() == index ) break;
        ptr = ptr->getNext_normal();
    }
    // Debug
    if ( ptr )  cout << "Found normal : " << ptr->getNindex() << endl;

    return ptr;
}

lsMesh_UV *lsMesh::Find_UV(int index)
{
    lsMesh_UV* ptr;

    // Debug
    cout << "Looking for UV : " << index << endl;

    ptr = uv_list;
    while ( ptr )
    {
        if ( ptr->getUv_index() == index ) break;
        ptr = ptr->getNext_uv();
    }
    // Debug
    if ( ptr )  cout << "Found UV : " << ptr->getUv_index() << endl;

    return ptr;
}


lsMesh_Face* lsMesh::Find_Face( int index )
{
    lsMesh_Face* face = 0;
    lsMesh_FaceGroup* group = group_list;

    while ( group )
    {
        face = group->getFace_list();
        while ( face )
        {
            if ( face->getIndex() == index ) break;
            face = face->getNext_face();
        }
        if ( face ) break;
        group = group->getNext_group();
    }

    return face;
}

lsMaterial *lsMesh::Find_Material(const string &name)
{
    lsMaterial *mat = material_list;
    while( mat )
    {
        if( mat->getName() == name ) break;
        mat = mat->getNext_material();
    }
    return mat;
}


/*!
 * \brief lsMesh::exportVBM
 * \param filename
 */
void lsMesh::exportVBM( const string& filename )
{
    cout << "Expport vortx batch mesh VBM ..." << endl;

    lsMesh_FaceGroup *facegroup;
    lsMesh_Face *face;
    lsMesh_Vertex *verts;
    lsMesh_Normal *normals;
    lsMesh_UV *UVs;

    string line;
    string rawfile;
    lsString convert;
    vector<string> list;
    stringstream ss;

    list.clear();
    rawfile = convert.RemoveExtension( filename );
    // Open file
    cout << "Write to file : " << rawfile + ".vbm" << endl;

    lsFileIO vbmfile;
    if( vbmfile.open( rawfile+".vbm", LSIO_LINE_CLEAR ) )
    {
        // header
        vbmfile.writeline( "# vortxGE VBM data by Lee A. Stripp" );
        vbmfile.writeline( "" );

        // batch
        ss.str("");
        ss << "batch start";
        vbmfile.writeline( ss.str() );

        vbmfile.writeline( "# bsphere : x y z radius" );
        ss.str("");
        ss << "bsphere ";
        ss << bSphere.center.x << " ";
        ss << bSphere.center.y << " ";
        ss << bSphere.center.z << " ";
        ss << bSphere.radius;
        vbmfile.writeline( ss.str() );
        vbmfile.writeline( "" );

        // Verticies
        vbmfile.writeline( "# vertex : index x y z" );
        verts = vertex_list;
        while( verts )
        {
            ss.str("");
            ss << "vertex ";
            ss << verts->toString();
            vbmfile.writeline( ss.str() );

            verts = verts->getNext_vertex();
        }

        // Normals
        vbmfile.writeline( "# normals and tangents : index x y z" );
        normals = normals_list;
        while( normals )
        {
            ss.str("");
            ss << "normal ";
            ss << normals->toString();
            vbmfile.writeline( ss.str() );
            // Tangent
            ss.str("");
            ss << "tangent ";
            ss << normals->toStringTangent();
            vbmfile.writeline( ss.str() );

            normals = normals->getNext_normal();
        }

        // UVs
        vbmfile.writeline( "# uv's' : index u v" );
        UVs = uv_list;
        while( UVs )
        {
            ss.str("");
            ss << "uv ";
            ss << UVs->toString();
            vbmfile.writeline( ss.str() );

            UVs = UVs->getNext_uv();
        }

        // Face Indicies
        vbmfile.writeline( "" );
        facegroup = group_list;
        while( facegroup )
        {
            // face_count
            vbmfile.writeline( "# face_count : TriCount QuadCount" );
            ss.str("");
            ss << "face_count ";
            ss << facegroup->getTriangle_count() << " " << facegroup->getQuad_count();
            vbmfile.writeline( ss.str() );

            vbmfile.writeline( "# face : FaceIndex VertexIndex/NormalIndex/UvIndex ..." );
            face = facegroup->getFace_list();
            while( face )
            {
                ss.str("");
                ss << "face ";
                ss << face->toString();
                vbmfile.writeline( ss.str() );

                face = face->getNext_face();
            }

            // Materials
            vbmfile.writeline( "" );
            vbmfile.writeline( "# Material" );
            // Ambiant
            ss.str("");
            ss << "ambiant ";
            ss << facegroup->getMaterial()->toStringAmbiant();
            vbmfile.writeline( ss.str() );
            // Diffuse
            ss.str("");
            ss << "diffuse ";
            ss << facegroup->getMaterial()->toStringDiffuse();
            vbmfile.writeline( ss.str() );
            // Specular
            ss.str("");
            ss << "specular ";
            ss << facegroup->getMaterial()->toStringSpecular();
            vbmfile.writeline( ss.str() );
            // Shininess
            ss.str("");
            ss << "shininess ";
            ss << facegroup->getMaterial()->getMshininess();
            vbmfile.writeline( ss.str() );
            // Transparent
            ss.str("");
            ss << "transparent ";
            ss << facegroup->getMaterial()->getTransparent();
            vbmfile.writeline( ss.str() );
            // Glow
            ss.str("");
            ss << "glow ";
            ss << facegroup->getMaterial()->getMglow();
            vbmfile.writeline( ss.str() );

            // Textures
            line = "";
            if(! facegroup->getMaterial()->getTex_d().empty() )
            {
                ss.str("");
                ss << "tex_d ";
                ss << facegroup->getMaterial()->getTex_d();
                vbmfile.writeline( ss.str() );
                line += "d";
            }
            if(! facegroup->getMaterial()->getTex_n().empty() )
            {
                ss.str("");
                ss << "tex_n ";
                ss << facegroup->getMaterial()->getTex_n();
                vbmfile.writeline( ss.str() );
                line += "n";
            }
            if(! facegroup->getMaterial()->getTex_s().empty() )
            {
                ss.str("");
                ss << "tex_s ";
                ss << facegroup->getMaterial()->getTex_s();
                vbmfile.writeline( ss.str() );
                line += "s";
            }

            // shader type
            if( line.empty() )
                line = "shader light_tex_none";
            else
                line = "shader light_tex_" + line;
            // write shader
            vbmfile.writeline( line );

            facegroup = facegroup->getNext_group();
        } // while facegroup

        // All done
        vbmfile.writeline( "batch end" );

    } else
    {
        // ERROR
        cout << "ERROR : Could not open file..." << endl;
    }
    cout << "Expport done..." << endl;
}


/* *********************************************************
 * Stats
 * */
void lsMesh::meshStats()
{
    lsMesh_FaceGroup * group;
    int i = 1;

    group = group_list;
    while( group )
    {
        cout << "Group : " << i << " ";
        cout << "Face count : " << group->getTriangle_count() + group->getQuad_count() << endl;
        cout << "Triangle count : " << group->getTriangle_count() << endl;
        cout << "Quad count : " << group->getQuad_count() << endl;
        cout << "Smooth shade : " << group->getSmooth_shade() << endl;
        if( group->getMaterial() )
            cout << "Material : " << group->getMaterial()->getName() << endl;
        cout << endl;

        i++;
        group = group->getNext_group();
    }
}

