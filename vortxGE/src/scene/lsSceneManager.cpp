#include "../vortx_debug.h"

// OpenGL
#include "../render/lsRender.h"

#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>
#include <functional> // Added for function

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

// Scene
#include "lsScene.h"
#include "lsObject.h"
#include "lsCamera.h"
#include "lsLight_Spot.h"
#include "lsLight_Point.h"
#include "lsLight_Direct.h"
#include "lsLightManager.h"

#include "lsSceneManager.h"


lsSceneManager::lsSceneManager()
{
	// scene_list was removed, m_scenes is default-constructed
}

lsSceneManager::~lsSceneManager()
{
	// vector<unique_ptr<lsScene>> automatically handles deletion of owned scenes.
	// No manual iteration or deletion needed here.
	m_scenes.clear(); // Ensure all unique_ptrs are released and destructors called
#ifdef VORTX_DEBUG_NODES
	cout << "lsSceneManager::~lsSceneManager() - All scenes deleted via unique_ptr." << endl;
#endif
}

// Move all scene objects

void lsSceneManager::Update( lsRender* render, double delta )
{
#ifdef VORTX_DEBUG_NODES
	cout << "lsSceneManager::Update" << endl;
#endif
	
	// Move stats
	if( render->getStats() ) render->getStats()->Move_Start();
	
	// Iterate through the vector of unique_ptrs to scenes
	for (const auto& scene_ptr : m_scenes)
	{
		lsScene* scene = scene_ptr.get();
		if (scene)
		{
			// Init base matrix
			scene->Move( scene, render, delta );

			// Move nodes threaded
			scene->MoveThread( scene, render, delta );
			
			// Clean up deleted nodes after processing
			scene->CleanupDeletedNodes();
		}
	}
	if( render->getStats() ) render->getStats()->Move_Stop();
}

void lsSceneManager::Add_Scene( lsScene* scene )
{
#ifdef VORTX_DEBUG_NODES
	if (scene) {
		cout << "lsSceneManager::Add_Scene : " << scene->getName() << endl;
	} else {
		cout << "lsSceneManager::Add_Scene : Attempted to add a null scene." << endl;
		return;
	}
#endif
	m_scenes.push_back(unique_ptr<lsScene>(scene));
}

lsScene* lsSceneManager::FindScene( const string& name )
{
	for (const auto& scene_ptr : m_scenes)
	{
		if (scene_ptr && scene_ptr->getName() == name)
		{
			return scene_ptr.get();
		}
	}
	return nullptr;
}

// Helper to parse (x,y,z) from string like "(1.0,2.0,3.0)"
static lsVector3 parse_vector3(const string& s)
{
	lsString converter;
	string cleaned = s;
	cleaned.erase(remove(cleaned.begin(), cleaned.end(), '('), cleaned.end());
	cleaned.erase(remove(cleaned.begin(), cleaned.end(), ')'), cleaned.end());
	
	vector<string> components;
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
static string parse_name(const string& s)
{
	if (s.length() < 2 || s.front() != '`' || s.back() != '`') {
		return s;
	}
	return s.substr(1, s.length() - 2);
}

// A tokenizer for the scene file format that respects quotes and parentheses
static vector<string> tokenize_vgs(const string& input)
{
	vector<string> tokens;
	string current_token;
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
	
	// Iterate through the vector of unique_ptrs to scenes
	for (const auto& scene_ptr : m_scenes)
	{
		lsScene* scene = scene_ptr.get();
		if (scene)
		{
			// open a file per scene
			// pass file on
			scene->Save();
		}
	}
}

bool lsSceneManager::LoadSceneFromFile(const string& filename, lsScene* targetScene)
{
	if(! targetScene)
	{
		cout << "ERROR: lsSceneManager::LoadSceneFromFile - targetScene is null." << endl;
		return false;
	}
	
	lsDir dir;
	string fullpath = dir.getexepath() + filename;
	
	lsFileIO file;
	if(! file.open(fullpath, LSIO_LINE_READ))
	{
		cout << "ERROR: lsSceneManager::LoadSceneFromFile - Can't open file: " << fullpath << endl;
		return false;
	}
	
	cout << "INFO: Loading scene file: " << fullpath << endl;
	
	map<string, lsNode*> nodeMap;
	vector<lsLight*> lights_in_scene;
	// vector<lsObject*> objects_in_scene; // Not strictly needed, as objects are added to nodeMap

	// Set initial scene name and add to node map
	targetScene->setName("DefaultScene"); // Temporary or default name
	nodeMap[targetScene->getName()] = targetScene;
	
	string line;
	lsString converter;
	
	int line_num = 0;
	while (file.readline())
	{
		line_num++;
		line = file.getLine();
		if(line.empty() || line[0] == '#')
		{
			continue;
		}
		
		vector<string> tokens = tokenize_vgs(line);
		if (tokens.empty())
		{
			continue;
		}
		
		const string& command = tokens[0];
		
		if(command == "add_scene")
		{
			if (tokens.size() >= 2)
			{
				string sceneName = parse_name(tokens[1]);
				cout << "INFO: Adding scene '" << sceneName << "'" << endl;
				targetScene->setName(sceneName);
				nodeMap[sceneName] = targetScene; // Update entry for potentially renamed scene
			}
		} else if (command == "add_object" && tokens.size() >= 7)
		{
			// add_object `name` `parent` (pos.x,pos.y,pos.z) (rot.x,rot.y,rot.z) (scale.x,scale.y,scale.z) `asset_path`
			string name = parse_name(tokens[1]);
			string parentName = parse_name(tokens[2]);
			lsVector3 pos = parse_vector3(tokens[3]);
			lsVector3 rot = parse_vector3(tokens[4]);
			lsVector3 scale = parse_vector3(tokens[5]);
			string assetPath = parse_name(tokens[6]);

			cout << "INFO: Adding object '" << name << "' with asset '" << assetPath << "'" << endl;

			lsNode* parent = nodeMap[parentName];
			if(! parent)
			{
				cout << "WARNING: Parent node '" << parentName << "' not found for object '" << name << "'. Attaching to scene root." << endl;
				parent = targetScene;
			}
			
			lsObject* obj = new lsObject(parent);
			obj->setName(name);
			obj->Set_Pos(pos.x, pos.y, pos.z);
			obj->Set_Rot(rot.x, rot.y, rot.z);
			obj->Set_Scale(scale.x, scale.y, scale.z);

			lsAsset_Mesh* meshAsset = static_cast<lsAsset_Mesh*>(targetScene->getAssetManager()->Find_Asset(assetPath));
			if(! meshAsset)
			{
				cout << "INFO: Creating new mesh asset for '" << assetPath << "'" << endl;
				meshAsset = targetScene->getAssetManager()->CreateAsset_Mesh(assetPath);
				meshAsset->AllocAsset();
			} else
			{
				cout << "INFO: Re-using existing mesh asset for '" << assetPath << "'" << endl;
			}
			obj->Set_Asset(meshAsset);

			parent->Add_Node(obj);
			nodeMap[name] = obj;
			// objects_in_scene.push_back(obj); // Not strictly needed
			
		} else if (command == "add_light" && tokens.size() >= 8)
		{
			// add_light type `name` `parent` (pos) (rot) (color) intensity
			string type = tokens[1];
			string name = parse_name(tokens[2]);
			string parentName = parse_name(tokens[3]);
			lsVector3 pos = parse_vector3(tokens[4]);
			lsVector3 rot = parse_vector3(tokens[5]);
			lsVector3 color = parse_vector3(tokens[6]);
			float intensity = converter.toFloat(tokens[7]);

			cout << "INFO: Adding " << type << " light '" << name << "'" << endl;
			
			lsNode* parent = nodeMap[parentName];
			if(! parent)
			{
				cout << "WARNING: Parent node '" << parentName << "' not found for light '" << name << "'. Attaching to scene root." << endl;
				parent = targetScene;
			}
			
			lsLight* light = nullptr;
			if(type == "spot")
			{
				light = new lsLight_Spot(parent);
			} else if(type == "point")
			{
				light = new lsLight_Point(parent);
			} else if(type == "direct")
			{
				light = new lsLight_Direct(parent);
			} else
			{
				cout << "WARNING: Unknown light type '" << type << "' for light '" << name << "'." << endl;
				continue;
			}
			
			light->setName(name);
			light->Set_Pos(pos.x, pos.y, pos.z);
			light->Set_Rot(rot.x, rot.y, rot.z);
			light->diffuse = lsVector4(color.x, color.y, color.z, 1.0f);
			light->setIntensity(intensity);
			
			parent->Add_Node(light);
			nodeMap[name] = light;
			lights_in_scene.push_back(light);
			targetScene->getLightManager()->Add_Light(light);
			
		} else if(command == "add_bullet_box" && tokens.size() >= 4)
		{
			// add_bullet_box `object_name` mass (half_extents)
			string objectName = parse_name(tokens[1]);
			float mass = converter.toFloat(tokens[2]);
			lsVector3 halfExtents = parse_vector3(tokens[3]);
			
			lsNode* node = nodeMap[objectName];
			lsObject* obj = dynamic_cast<lsObject*>(node);
			
			if(obj)
			{
				lsAsset_BulletShape_Box* shapeAsset = targetScene->getAssetManager()->CreateAsset_BulletShape_Box(halfExtents.x, halfExtents.y, halfExtents.z);
				shapeAsset->AllocAsset();
				lsBulletShape* bulletShape = static_cast<lsBulletShape*>(shapeAsset->getData());
				
				lsBulletObject* bulletObject = new lsBulletObject(obj, bulletShape->getShape(), targetScene->getBulletworld()->getMax_proxies());
				bulletObject->Init(mass);
				obj->setBulletObject(bulletObject); // Attach bullet object to the scene object
				
		        // Load friction values if present
		        if (tokens.size() >= 7)
		        {
		            float friction = converter.toFloat(tokens[4]);
		            float rollingFriction = converter.toFloat(tokens[5]);
		            float spinningFriction = converter.toFloat(tokens[6]);
		            
		            bulletObject->getBody()->setFriction(friction);
		            bulletObject->getBody()->setRollingFriction(rollingFriction);
		            bulletObject->getBody()->setSpinningFriction(spinningFriction);
		        }
			} else
			{
				cout << "WARNING: Object '" << objectName << "' not found for bullet box shape attachment." << endl;
			}
		} else if(command == "add_bullet_pill" && tokens.size() >= 5)
		{
			// add_bullet_pill `object_name` mass radius height
			string objectName = parse_name(tokens[1]);
			float mass = converter.toFloat(tokens[2]);
			float radius = converter.toFloat(tokens[3]);
			float height = converter.toFloat(tokens[4]);
			
			lsNode* node = nodeMap[objectName];
			lsObject* obj = dynamic_cast<lsObject*>(node);

			if (obj)
			{
				lsAsset_BulletShape_Pill* shapeAsset = targetScene->getAssetManager()->CreateAsset_BulletShape_Pill(radius, height);
				shapeAsset->AllocAsset();
				lsBulletShape* bulletShape = static_cast<lsBulletShape*>(shapeAsset->getData());
				
				lsBulletObject* bulletObject = new lsBulletObject(obj, bulletShape->getShape(), targetScene->getBulletworld()->getMax_proxies());
				bulletObject->Init(mass);
				obj->setBulletObject(bulletObject); // Set bullet object to the object
				
				// Load friction values if present
				if (tokens.size() >= 8)
				{
					float friction = converter.toFloat(tokens[4]);
					float rollingFriction = converter.toFloat(tokens[5]);
					float spinningFriction = converter.toFloat(tokens[6]);
					
					bulletObject->getBody()->setFriction(friction);
					bulletObject->getBody()->setRollingFriction(rollingFriction);
					bulletObject->getBody()->setSpinningFriction(spinningFriction);
				}
			} else
			{
				cout << "WARNING: Object '" << objectName << "' not found for bullet pill shape attachment." << endl;
			}
		} else if(command == "add_bullet_convexhull" && tokens.size() >= 3)
		{
			// add_bullet_convexhull `object_name` mass
			string objectName = parse_name(tokens[1]);
			float mass = converter.toFloat(tokens[2]);
			
			lsNode* node = nodeMap[objectName];
			lsObject* obj = dynamic_cast<lsObject*>(node);
			
			if(obj && obj->getAsset() && obj->getAsset()->getAsset_type() == LSAT_MESH)
			{
				lsAsset_Mesh* meshAsset = static_cast<lsAsset_Mesh*>(obj->getAsset());
				lsAsset_BulletShape_ConvexHull* shapeAsset = targetScene->getAssetManager()->CreateAsset_BulletShape_ConvexHull(meshAsset);
				shapeAsset->AllocAsset();
				lsBulletShape* bulletShape = static_cast<lsBulletShape*>(shapeAsset->getData());

				lsBulletObject* bulletObject = new lsBulletObject(obj, bulletShape->getShape(), targetScene->getBulletworld()->getMax_proxies());
				bulletObject->Init(mass);
				obj->setBulletObject(bulletObject); // Set bullet object to the object
				
				// Load friction values if present
				if (tokens.size() >= 6)
				{
					float friction = converter.toFloat(tokens[3]);
					float rollingFriction = converter.toFloat(tokens[4]);
					float spinningFriction = converter.toFloat(tokens[5]);
					
					bulletObject->getBody()->setFriction(friction);
					bulletObject->getBody()->setRollingFriction(rollingFriction);
					bulletObject->getBody()->setSpinningFriction(spinningFriction);
				}
			} else
			{
				 cout << "WARNING: Object '" << objectName << "' not found or has no mesh for convex hull shape." << endl;
			}
		} else if(command == "add_bullet_trimesh" && tokens.size() >= 3)
		{
			// add_bullet_trimesh `object_name` mass
			string objectName = parse_name(tokens[1]);
			float mass = converter.toFloat(tokens[2]);

			lsNode* node = nodeMap[objectName];
			lsObject* obj = dynamic_cast<lsObject*>(node);

			if (obj && obj->getAsset() && obj->getAsset()->getAsset_type() == LSAT_MESH)
			{
				lsAsset_Mesh* meshAsset = static_cast<lsAsset_Mesh*>(obj->getAsset());
				lsAsset_BulletShape_TriMesh* shapeAsset = targetScene->getAssetManager()->CreateAsset_BulletShape_TriMesh(meshAsset);
				shapeAsset->AllocAsset();
				lsBulletShape* bulletShape = static_cast<lsBulletShape*>(shapeAsset->getData());

				lsBulletObject* bulletObject = new lsBulletObject(obj, bulletShape->getShape(), targetScene->getBulletworld()->getMax_proxies());
				bulletObject->Init(mass);
				obj->setBulletObject(bulletObject); // Set bullet object to the object
				
				// Load friction values if present
				if (tokens.size() >= 6)
				{
					float friction = converter.toFloat(tokens[3]);
					float rollingFriction = converter.toFloat(tokens[4]);
					float spinningFriction = converter.toFloat(tokens[5]);
					
					bulletObject->getBody()->setFriction(friction);
					bulletObject->getBody()->setRollingFriction(rollingFriction);
					bulletObject->getBody()->setSpinningFriction(spinningFriction);
				}
			} else
			{
				 cout << "WARNING: Object '" << objectName << "' not found or has no mesh for trimesh shape." << endl;
			}
		} else if (command == "add_bullet_only_mesh" && tokens.size() >= 7)
		{
			// add_bullet_only_mesh `name` <type> mass `asset_path` (pos.x,pos.y,pos.z) (rot.x,rot.y,rot.z) (scale.x,scale.y,scale.z)
			string name = parse_name(tokens[1]);
			string physicsType = tokens[2]; // "convexhull" or "trimesh"
			float mass = converter.toFloat(tokens[3]);
			string assetPath = parse_name(tokens[4]);
			lsVector3 pos = parse_vector3(tokens[5]);
			lsVector3 rot = parse_vector3(tokens[6]);
			lsVector3 scale = parse_vector3(tokens[7]);
			
			cout << "INFO: Adding physics-only mesh '" << name << "' of type '" << physicsType << "' with mass " << mass << endl;
			
			// Load or create mesh asset
			lsAsset_Mesh* meshAsset = static_cast<lsAsset_Mesh*>(targetScene->getAssetManager()->Find_Asset(assetPath));
			if (!meshAsset)
			{
				cout << "INFO: Creating new mesh asset for physics-only mesh '" << assetPath << "'." << endl;
				meshAsset = targetScene->getAssetManager()->CreateAsset_Mesh(assetPath);
				meshAsset->AllocAsset();
			}
			if (!meshAsset || !meshAsset->getData())
			{
				cout << "ERROR: Failed to load mesh asset for physics-only mesh '" << assetPath << "'." << endl;
				continue;
			}
			
			lsBulletShape* bulletShape = nullptr;
			if (physicsType == "convexhull")
			{
				lsAsset_BulletShape_ConvexHull* shapeAsset = targetScene->getAssetManager()->CreateAsset_BulletShape_ConvexHull(meshAsset);
				shapeAsset->AllocAsset();
				bulletShape = static_cast<lsBulletShape*>(shapeAsset->getData());
				cout << "INFO: Convex Hull physics shape asset created and registered for '" << name << "'." << endl;
			} else if (physicsType == "trimesh")
			{
				lsAsset_BulletShape_TriMesh* shapeAsset = targetScene->getAssetManager()->CreateAsset_BulletShape_TriMesh(meshAsset);
				shapeAsset->AllocAsset();
				bulletShape = static_cast<lsBulletShape*>(shapeAsset->getData());
				cout << "INFO: Triangle Mesh physics shape asset created and registered for '" << name << "'." << endl;
			} else
			{
				cout << "WARNING: Unknown physics type '" << physicsType << "' for physics-only mesh '" << name << "'." << endl;
				continue;
			}

			if (bulletShape) {
				lsObject* physicsOnlyObj = new lsObject(targetScene);
				physicsOnlyObj->setName("PhysicsOnly_" + name);
				physicsOnlyObj->Set_Pos(pos.x, pos.y, pos.z);
				physicsOnlyObj->Set_Rot(rot.x, rot.y, rot.z);
				physicsOnlyObj->Set_Scale(scale.x, scale.y, scale.z);
				
				targetScene->Add_Node(physicsOnlyObj);
				nodeMap[physicsOnlyObj->getName()] = physicsOnlyObj;

				lsBulletObject* bulletObject = new lsBulletObject(physicsOnlyObj, bulletShape->getShape(), targetScene->getBulletworld()->getMax_proxies());
				bulletObject->Init(mass);
				physicsOnlyObj->setBulletObject(bulletObject);
				
				// Load friction values if present
				if (tokens.size() >= 10)
				{
					float friction = converter.toFloat(tokens[8]);
					float rollingFriction = converter.toFloat(tokens[9]);
					float spinningFriction = converter.toFloat(tokens[10]);
					
					bulletObject->getBody()->setFriction(friction);
					bulletObject->getBody()->setRollingFriction(rollingFriction);
					bulletObject->getBody()->setSpinningFriction(spinningFriction);
				}
				
				cout << "INFO: Physics-only object '" << physicsOnlyObj->getName() << "' created and added to Bullet world." << endl;
			}
			
		} else
		{
			cout << "WARNING: Unrecognized or malformed command on line " << line_num << ": " << line << endl;
		}
	}
	
	cout << "INFO: Scene loading finished." << endl;
	return true;
}

void lsSceneManager::LinkLightsInScene(lsScene* scene, int maxLightsPerObject)
{
	if (!scene) return;
	
	cout << "INFO: Linking lights in scene '" << scene->getName() << "'" << endl;
	
	const auto& all_lights = scene->getLightManager()->getLights();
	if (all_lights.empty())
	{
		cout << "INFO: No lights in scene to link." << endl;
		return;
	}
	
	vector<lsObject*> all_objects;
	// Helper function to recursively collect objects
	function<void(lsNode*)> collectObjects = 
		[&](lsNode* node) {
		if (!node) return;

		// Check if the node is an object (or a subclass)
		if (auto obj = dynamic_cast<lsObject*>(node)) {
			all_objects.push_back(obj);
		}
		
		// Recurse for children using the correct getChildren() method
		for (const auto& child_ptr : node->getChildren()) {
			collectObjects(child_ptr.get());
		}
	};
	
	// Start collection from the scene's children (scene itself is a lsNode)
	for (const auto& child_ptr : scene->getChildren()) {
		collectObjects(child_ptr.get());
	}
	
	cout << "INFO: Found " << all_objects.size() << " objects and " << all_lights.size() << " lights to process for linking." << endl;
	
	for(lsObject* obj : all_objects)
	{
			// Make sure light list is empty.
		obj->getLightManager()->ClearLights();
		
		// Store lights with distance.
		vector<pair<float, lsLight*>> sorted_lights;
		// Get object oosition.
		lsVector3 objPos(obj->ModelMatrix.matrix[12], obj->ModelMatrix.matrix[13], obj->ModelMatrix.matrix[14]);

		for (lsLight* light : all_lights)
		{
				// Get light position.
			lsVector3 lightPos(light->ModelMatrix.matrix[12], light->ModelMatrix.matrix[13], light->ModelMatrix.matrix[14]);
			float dist = objPos.distance(lightPos);
			
			// Direct lights get priority (distance = 0)
			if (light->getLight_type() == LSLT_DIRECT) {
				sorted_lights.push_back({0.0f, light});
			} else {
				sorted_lights.push_back({dist * dist, light}); // Use squared distance for sorting
			}
		}
		
		// Sort by distance (squared) - direct lights will be at the beginning due to 0.0 distance
		sort(sorted_lights.begin(), sorted_lights.end(), 
				  [](const auto& a, const auto& b) {
					  return a.first < b.first;
				  });
		
		// Add lights to object.
		int lights_to_add = min((int)sorted_lights.size(), maxLightsPerObject);
		for(int i = 0; i < lights_to_add; ++i)
		{
			obj->getLightManager()->Add_Light(sorted_lights[i].second);
		}
	}
	
	cout << "INFO: Light linking complete." << endl;
}

