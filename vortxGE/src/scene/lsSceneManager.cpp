#include "../vortx_debug.h"
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>
#include <functional>

// OpenGL
#include "../opengl/lsOpenGL.h"

// Scene
#include "lsScene.h"
#include "lsObject.h"
#include "lsCamera.h"
#include "lsLight_Spot.h"
#include "lsLight_Point.h"
#include "lsLightManager.h"

// datablocks
#include "../datablocks/lsStats.h"
#include "../datablocks/lsString.h"
#include "../datablocks/lsBulletShape.h"

// filesystem
#include "../filesystem/lsDir.h"
#include "../filesystem/lsFileIO.h"

// math
#include "../math/lsVector3.h"

// assets
#include "../assets/lsAssetManager.h"
#include "../assets/lsAsset_Mesh.h"
#include "../assets/lsAsset_BulletShape_Box.h"
#include "../assets/lsAsset_BulletShape_Pill.h"
#include "../assets/lsAsset_BulletShape_ConvexHull.h"
#include "../assets/lsAsset_BulletShape_TriMesh.h"

// Physics
#include "../physics/lsBulletObject.h"
#include "../physics/lsBulletWorld.h"

#include "lsSceneManager.h"


lsSceneManager::lsSceneManager()
{
    scene_list = nullptr;
}

lsSceneManager::~lsSceneManager()
{
    // Delete all scenes
    lsScene *scene, *nextscene;

    scene = scene_list;
    while( scene )
    {
        nextscene = (lsScene*)scene->getNext_node();

        // We must clear the scene before deletion
        scene->ClearScene();
        delete scene;

        scene = nextscene;
    }
    scene_list = NULL;
}


// Move all scene objects

void lsSceneManager::Update( lsOpenGL* opengl, double delta )
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsSceneManager::Update" << endl;
#endif
    lsScene *scene = scene_list;
	
    // Move stats
    if( opengl->getStats() ) opengl->getStats()->Move_Start();
    while( scene )
    {
        // Init base matrix
        scene->Move( scene, opengl, delta );

        // Move nodes threaded
        scene->MoveThread( scene, opengl, delta );

        // Next scene
        scene = (lsScene*)scene->getNext_node();
    }
    if( opengl->getStats() ) opengl->getStats()->Move_Stop();
}

void lsSceneManager::Add_Scene( lsScene *scene )
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsSceneManager::Add_Scene : " << scene->getName() << endl;
#endif

    if( scene_list ) scene_list->setPrev_node( scene );
    scene->setNext_node( scene_list );
    scene_list = scene;
}

lsScene* lsSceneManager::FindScene( const string& name )
{
    lsScene *scene = scene_list;
    while( scene )
    {
        if( scene->getName() == name )
        {
            return scene;
        }
        scene = (lsScene*)scene->getNext_node();
    }
    return NULL;
}

// Helper to parse (x,y,z) from string like "(1.0,2.0,3.0)"
static lsVector3 parse_vector3(const std::string& s) {
    lsString converter;
    std::string cleaned = s;
    cleaned.erase(std::remove(cleaned.begin(), cleaned.end(), '('), cleaned.end());
    cleaned.erase(std::remove(cleaned.begin(), cleaned.end(), ')'), cleaned.end());
    
    std::vector<std::string> components;
    lsString::SplitString(cleaned, ',', components);
    
    if (components.size() == 3) {
        return lsVector3(
            converter.toFloat(components[0]),
            converter.toFloat(components[1]),
            converter.toFloat(components[2])
        );
    }
    return lsVector3(0, 0, 0);
}

// Helper to parse `name` from string
static std::string parse_name(const std::string& s) {
    if (s.length() < 2 || s.front() != '`' || s.back() != '`') {
        return s;
    }
    return s.substr(1, s.length() - 2);
}

// A tokenizer for the scene file format that respects quotes and parentheses
static std::vector<std::string> tokenize_vgs(const std::string& input) {
    std::vector<std::string> tokens;
    std::string current_token;
    bool in_quoted_string = false;
    bool in_vector = false;

    for (char c : input) {
        if (c == '`') {
            if (in_quoted_string) {
                // End of quoted string
                tokens.push_back("`" + current_token + "`");
                current_token.clear();
                in_quoted_string = false;
            } else {
                // Start of quoted string
                if (!current_token.empty()) {
                    tokens.push_back(current_token);
                    current_token.clear();
                }
                in_quoted_string = true;
            }
        } else if (c == '(' && !in_quoted_string) {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
            in_vector = true;
            current_token += c;
        } else if (c == ')' && in_vector) {
            current_token += c;
            tokens.push_back(current_token);
            current_token.clear();
            in_vector = false;
        } else if (isspace(c) && !in_quoted_string && !in_vector) {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
        } else {
            current_token += c;
        }
    }

    if (!current_token.empty()) {
        tokens.push_back(current_token);
    }

    return tokens;
}


// ################################
// Save and Load

void lsSceneManager::SaveScenes( const string& path )
{
	cout << "Save scenes to path : " << path << endl;
	lsScene *scene = scene_list;

    while( scene )
    {
		// open a file per scene

    	// pass file on
    	scene->Save();

        // Next scene
        scene = (lsScene*)scene->getNext_node();
    }
}

bool lsSceneManager::LoadSceneFromFile(const std::string& filename, lsScene* targetScene)
{
    if(! targetScene)
    {
        std::cout << "ERROR: lsSceneManager::LoadSceneFromFile - targetScene is null." << std::endl;
        return false;
    }
	
    lsDir dir;
    std::string fullpath = dir.getexepath() + filename;
	
    lsFileIO file;
    if (!file.open(fullpath, LSIO_LINE_READ))
    {
        std::cout << "ERROR: lsSceneManager::LoadSceneFromFile - Can't open file: " << fullpath << std::endl;
        return false;
    }
	
    std::cout << "INFO: Loading scene file: " << fullpath << std::endl;

    std::map<std::string, lsNode*> nodeMap;
    std::vector<lsLight*> lights_in_scene;
    std::vector<lsObject*> objects_in_scene;
	
    std::string line;
    lsString converter;
	
    int line_num = 0;
    while (file.readline())
    {
        line_num++;
        line = file.getLine();
        if (line.empty() || line[0] == '#') {
            continue;
        }
		
        std::vector<std::string> tokens = tokenize_vgs(line);

        if (tokens.empty()) {
            continue;
        }
		
        const std::string& command = tokens[0];
		
        if (command == "add_scene") {
            if (tokens.size() >= 2) {
                std::string sceneName = parse_name(tokens[1]);
                std::cout << "INFO: Adding scene '" << sceneName << "'" << std::endl;
                targetScene->setName(sceneName);
                nodeMap[sceneName] = targetScene;
            }
        } else if (command == "add_object" && tokens.size() >= 7) {
            // add_object `name` `parent` (pos.x,pos.y,pos.z) (rot.x,rot.y,rot.z) (scale.x,scale.y,scale.z) `asset_path`
            std::string name = parse_name(tokens[1]);
            std::string parentName = parse_name(tokens[2]);
            lsVector3 pos = parse_vector3(tokens[3]);
            lsVector3 rot = parse_vector3(tokens[4]);
            lsVector3 scale = parse_vector3(tokens[5]);
            std::string assetPath = parse_name(tokens[6]);

            std::cout << "INFO: Adding object '" << name << "' with asset '" << assetPath << "'" << std::endl;

            lsNode* parent = nodeMap[parentName];
            if (!parent) {
                std::cout << "WARNING: Parent node '" << parentName << "' not found for object '" << name << "'. Attaching to scene root." << std::endl;
                parent = targetScene;
            }
			
            lsObject* obj = new lsObject(parent);
            obj->setName(name);
            obj->Set_Pos(pos.x, pos.y, pos.z);
            obj->Set_Rot(rot.x, rot.y, rot.z);
            obj->Set_Scale(scale.x, scale.y, scale.z);

            lsAsset_Mesh* meshAsset = static_cast<lsAsset_Mesh*>(targetScene->getAssetManager()->Find_Asset(assetPath));
            if (!meshAsset) {
                std::cout << "INFO: Creating new mesh asset for '" << assetPath << "'" << std::endl;
                meshAsset = targetScene->getAssetManager()->CreateAsset_Mesh(assetPath);
                meshAsset->AllocAsset();
            } else {
                std::cout << "INFO: Re-using existing mesh asset for '" << assetPath << "'" << std::endl;
            }
            obj->Set_Asset(meshAsset);

            parent->Add_Node(obj);
            nodeMap[name] = obj;
            objects_in_scene.push_back(obj);

        } else if (command == "add_light" && tokens.size() >= 7) {
            // add_light type `name` `parent` (pos) (rot) (color)
            std::string type = tokens[1];
            std::string name = parse_name(tokens[2]);
            std::string parentName = parse_name(tokens[3]);
            lsVector3 pos = parse_vector3(tokens[4]);
            lsVector3 rot = parse_vector3(tokens[5]);
            lsVector3 color = parse_vector3(tokens[6]);
            
            std::cout << "INFO: Adding " << type << " light '" << name << "'" << std::endl;
            
            lsNode* parent = nodeMap[parentName];
            if (!parent) {
                std::cout << "WARNING: Parent node '" << parentName << "' not found for light '" << name << "'. Attaching to scene root." << std::endl;
                parent = targetScene;
            }

            lsLight* light = nullptr;
            if (type == "spot") {
                light = new lsLight_Spot(parent);
            } else if (type == "point") {
                light = new lsLight_Point(parent);
            } else {
                std::cout << "WARNING: Unknown light type '" << type << "' for light '" << name << "'." << std::endl;
                continue;
            }
            
            light->setName(name);
            light->Set_Pos(pos.x, pos.y, pos.z);
            light->Set_Rot(rot.x, rot.y, rot.z);
            light->diffuse = lsVector4(color.x, color.y, color.z, 1.0f);
            
            parent->Add_Node(light);
            nodeMap[name] = light;
            lights_in_scene.push_back(light);
            targetScene->getLightManager()->Add_Light(light);

        } else if (command == "add_camera" && tokens.size() >= 6) {
            // add_camera `name` `parent` (pos) (rot) (scale)
            std::string name = parse_name(tokens[1]);
            std::string parentName = parse_name(tokens[2]);
            lsVector3 pos = parse_vector3(tokens[3]);
            lsVector3 rot = parse_vector3(tokens[4]);
            lsVector3 scale = parse_vector3(tokens[5]);

            std::cout << "INFO: Adding camera '" << name << "'" << std::endl;

            lsNode* parent = nodeMap[parentName];
            if (!parent) {
                std::cout << "WARNING: Parent node '" << parentName << "' not found for camera '" << name << "'. Attaching to scene root." << std::endl;
                parent = targetScene;
            }

            lsCamera* cam = new lsCamera(parent);
            cam->setName(name);
            cam->Set_Pos(pos.x, pos.y, pos.z);
            cam->Set_Rot(rot.x, rot.y, rot.z);
            cam->Set_Scale(scale.x, scale.y, scale.z);

            parent->Add_Node(cam);
            nodeMap[name] = cam;
        }
        else if (command == "add_bullet_box" && tokens.size() >= 4) {
            // add_bullet_box `object_name` mass (half_extents)
            std::string objectName = parse_name(tokens[1]);
            float mass = converter.toFloat(tokens[2]);
            lsVector3 halfExtents = parse_vector3(tokens[3]);
            
            lsNode* node = nodeMap[objectName];
            lsObject* obj = dynamic_cast<lsObject*>(node);

            if (obj) {
                lsAsset_BulletShape_Box* shapeAsset = targetScene->getAssetManager()->CreateAsset_BulletShape_Box(halfExtents.x, halfExtents.y, halfExtents.z);
                shapeAsset->AllocAsset();
                lsBulletShape* bulletShape = static_cast<lsBulletShape*>(shapeAsset->getData());
                
                lsBulletObject* bulletObject = new lsBulletObject(obj, bulletShape->getShape(), targetScene->getBulletworld()->getMax_proxies());
                bulletObject->Init(mass);
            } else {
                std::cout << "WARNING: Object '" << objectName << "' not found for bullet box shape attachment." << std::endl;
            }
        }
        else if (command == "add_bullet_pill" && tokens.size() >= 5) {
            // add_bullet_pill `object_name` mass radius height
            std::string objectName = parse_name(tokens[1]);
            float mass = converter.toFloat(tokens[2]);
            float radius = converter.toFloat(tokens[3]);
            float height = converter.toFloat(tokens[4]);
            
            lsNode* node = nodeMap[objectName];
            lsObject* obj = dynamic_cast<lsObject*>(node);

            if (obj) {
                lsAsset_BulletShape_Pill* shapeAsset = targetScene->getAssetManager()->CreateAsset_BulletShape_Pill(radius, height);
                shapeAsset->AllocAsset();
                lsBulletShape* bulletShape = static_cast<lsBulletShape*>(shapeAsset->getData());
                
                lsBulletObject* bulletObject = new lsBulletObject(obj, bulletShape->getShape(), targetScene->getBulletworld()->getMax_proxies());
                bulletObject->Init(mass);
            } else {
                std::cout << "WARNING: Object '" << objectName << "' not found for bullet pill shape attachment." << std::endl;
            }
        }
        else if (command == "add_bullet_convexhull" && tokens.size() >= 3) {
            // add_bullet_convexhull `object_name` mass
            std::string objectName = parse_name(tokens[1]);
            float mass = converter.toFloat(tokens[2]);

            lsNode* node = nodeMap[objectName];
            lsObject* obj = dynamic_cast<lsObject*>(node);

            if (obj && obj->getAsset() && obj->getAsset()->getAsset_type() == LSAT_MESH) {
                lsAsset_Mesh* meshAsset = static_cast<lsAsset_Mesh*>(obj->getAsset());
                lsAsset_BulletShape_ConvexHull* shapeAsset = targetScene->getAssetManager()->CreateAsset_BulletShape_ConvexHull(meshAsset);
                shapeAsset->AllocAsset();
                lsBulletShape* bulletShape = static_cast<lsBulletShape*>(shapeAsset->getData());

                lsBulletObject* bulletObject = new lsBulletObject(obj, bulletShape->getShape(), targetScene->getBulletworld()->getMax_proxies());
                bulletObject->Init(mass);
            } else {
                 std::cout << "WARNING: Object '" << objectName << "' not found or has no mesh for convex hull shape." << std::endl;
            }
        }
        else if (command == "add_bullet_trimesh" && tokens.size() >= 3) {
            // add_bullet_trimesh `object_name` mass
            std::string objectName = parse_name(tokens[1]);
            float mass = converter.toFloat(tokens[2]);

            lsNode* node = nodeMap[objectName];
            lsObject* obj = dynamic_cast<lsObject*>(node);

            if (obj && obj->getAsset() && obj->getAsset()->getAsset_type() == LSAT_MESH) {
                lsAsset_Mesh* meshAsset = static_cast<lsAsset_Mesh*>(obj->getAsset());
                lsAsset_BulletShape_TriMesh* shapeAsset = targetScene->getAssetManager()->CreateAsset_BulletShape_TriMesh(meshAsset);
                shapeAsset->AllocAsset();
                lsBulletShape* bulletShape = static_cast<lsBulletShape*>(shapeAsset->getData());

                lsBulletObject* bulletObject = new lsBulletObject(obj, bulletShape->getShape(), targetScene->getBulletworld()->getMax_proxies());
                bulletObject->Init(mass); // Mass should be 0 for trimesh for static objects
            } else {
                 std::cout << "WARNING: Object '" << objectName << "' not found or has no mesh for trimesh shape." << std::endl;
            }
        } else {
            std::cout << "WARNING: Unrecognized or malformed command on line " << line_num << ": " << line << std::endl;
        }
    }
	
    for (lsObject* obj : objects_in_scene)
    {
        for (lsLight* light : lights_in_scene)
        {
            obj->getLightManager()->Add_Light(light);
        }
    }
	
    std::cout << "INFO: Scene loading finished." << std::endl;
    return true;
}

void lsSceneManager::LinkLightsInScene(lsScene* scene, int maxLightsPerObject)
{
    if (!scene) return;
	
    std::cout << "INFO: Linking lights in scene '" << scene->getName() << "'" << std::endl;
	
    const auto& all_lights = scene->getLightManager()->getLights();
    if (all_lights.empty())
    {
        std::cout << "INFO: No lights in scene to link." << std::endl;
        return;
    }
	
    std::vector<lsObject*> all_objects;
    // Helper function to recursively collect objects
    std::function<void(lsNode*)> collectObjects = 
        [&](lsNode* node) {
        if (!node) return;

        // Check if the node is an object (or a subclass)
        if (auto obj = dynamic_cast<lsObject*>(node)) {
            all_objects.push_back(obj);
        }
		
        // Recurse for children
        collectObjects(node->getNode_list());

        // Recurse for siblings
        collectObjects(node->getNext_node());
    };

    // Start collection from the scene's root children
    collectObjects(scene->getNode_list());

    std::cout << "INFO: Found " << all_objects.size() << " objects and " << all_lights.size() << " lights to process for linking." << std::endl;

    for (lsObject* obj : all_objects) {
        obj->getLightManager()->ClearLights();

        std::vector<std::pair<float, lsLight*>> sorted_lights;

        lsVector3 objPos(obj->ModelMatrix.matrix[12], obj->ModelMatrix.matrix[13], obj->ModelMatrix.matrix[14]);

        for (lsLight* light : all_lights) {
            lsVector3 lightPos(light->ModelMatrix.matrix[12], light->ModelMatrix.matrix[13], light->ModelMatrix.matrix[14]);
            float dist = objPos.distance(lightPos);
            sorted_lights.push_back({dist * dist, light}); // Use squared distance for sorting
        }

        // Sort by distance (squared)
        std::sort(sorted_lights.begin(), sorted_lights.end(), 
                  [](const auto& a, const auto& b) {
                      return a.first < b.first;
                  });

        int lights_to_add = std::min((int)sorted_lights.size(), maxLightsPerObject);
        for (int i = 0; i < lights_to_add; ++i) {
            obj->getLightManager()->Add_Light(sorted_lights[i].second);
        }
    }
    
    std::cout << "INFO: Light linking complete." << std::endl;
}

