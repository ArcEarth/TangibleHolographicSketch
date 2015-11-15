#pragma once
#include "tinyxml2.h"
#include "AssetDictionary.h"
#include "SceneObject.h"

namespace Causality
{
	namespace XML
	{
		extern map<string, function<SceneObject*(tinyxml2::XMLElement* node, AssetDictionary& assets, SceneObject* parent)>> g_XMLSceneGenerators;
	}
}