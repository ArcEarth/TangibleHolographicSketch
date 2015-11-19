#include "pch_bcl.h"
#include "Serialization.h"
#include <tinyxml2.h>
#include <sstream>

using namespace Causality;
using namespace Causality::Serialization;
using namespace tinyxml2;
using namespace std;

namespace Causality
{
	namespace Serialization
	{
		bool GetString(const ParamArchive * store, const char * name, const char *(&param))
		{
			auto str = store->Attribute(name);
			if (str != nullptr)
				param = str;
			else
				return false;
		}

		bool GetString(const ParamArchive * store, const char * name, string & param)
		{
			auto str = store->Attribute(name);
			if (str != nullptr)
				param = str;
			else
				return false;
		}

		bool GetBool(const ParamArchive * store, const char * name, bool & param)
		{
			return !store->QueryBoolAttribute(name, &param);
		}

		bool GetFloat(const ParamArchive * store, const char * name, float & param)
		{
			return !store->QueryFloatAttribute(name, &param);
		}

		bool GetDouble(const ParamArchive * store, const char * name, double & param)
		{
			return !store->QueryDoubleAttribute(name, &param);
		}

		bool GetInt(const ParamArchive * store, const char * name, int & param)
		{
			return !store->QueryIntAttribute(name, &param);
		}

		bool GetUint(const ParamArchive * store, const char * name, unsigned & param)
		{
			return !store->QueryUnsignedAttribute(name, &param);
		}

		bool GetVector2(const ParamArchive * store, const char * name, Vector2 & param)
		{
			auto attrval = store->Attribute(name);
			if (attrval != nullptr)
			{
				string str(attrval);
				if (str.find_first_of(',') != string::npos)
				{
					stringstream ss(str);
					char ch;
					ss >> param.x >> ch >> param.y;
					return true;
				}
				else
				{
					param.x = param.y = (float)atof(attrval); // replicate
					return true;
				}
			}
			return false;
		}

		bool GetVector3(const ParamArchive * store, const char * name, Vector3 & param)
		{
			auto attrval = store->Attribute(name);
			if (attrval != nullptr)
			{
				string str(attrval);
				if (str.find_first_of(',') != string::npos)
				{
					stringstream ss(str);
					char ch;
					ss >> param.x >> ch >> param.y >> ch >> param.z;
					return true;
				}
				else
				{
					param.x = param.y = param.z = (float)atof(attrval); // replicate
					return true;
				}
			}
			return false;
		}

		bool GetVector4(const ParamArchive * store, const char * name, Vector4 & param)
		{
			auto attrval = store->Attribute(name);
			if (attrval != nullptr)
			{
				string str(attrval);
				if (str.find_first_of(',') != string::npos)
				{
					stringstream ss(str);
					char ch;
					ss >> param.x >> ch >> param.y >> ch >> param.z >> ch >> param.w;
					return true;
				}
				else
				{
					param.x = param.y = param.z = param.w = (float)atof(attrval); // replicate
					return true;
				}
			}
			return false;
		}

		bool GetColor(const ParamArchive * store, const char * name, Color & param)
		{
			auto attrval = store->Attribute(name);
			if (attrval != nullptr && attrval[0] == '#') // for hex code literal
			{
				char* end;
				auto val = strtoul(attrval + 1, &end, 16);
				param.w = ((float)((val & 0xff000000U) >> 24)) / (float)0xff;
				param.x = ((float)((val & 0x00ff0000U) >> 16)) / (float)0xff;
				param.y = ((float)((val & 0x0000ff00U) >> 8)) / (float)0xff;
				param.z = ((float)((val & 0x000000ffU) >> 0)) / (float)0xff;
				return true;
			}
			else if (attrval != nullptr && (std::isdigit(attrval[0]) || attrval[0] == '.')) // for Vector4 literal
			{
				Serialization::GetVector4(store, name, reinterpret_cast<Vector4&>(param));
				return true;
			}
			return false;
		}
	}

	const ParamArchive * GetFirstChildArchive(const ParamArchive * store, const char * name)
	{
		return store->FirstChildElement(name);
	}

	const ParamArchive * GetPrevSiblingArchive(const ParamArchive * store, const char * name)
	{
		return store->PreviousSiblingElement(name);
	}

	const ParamArchive * GetNextSiblingArchive(const ParamArchive * store, const char * name)
	{
		return store->NextSiblingElement(name);
	}

	const ParamArchive * GetParentArchive(const ParamArchive * store)
	{
		return dynamic_cast<const XMLElement*>(store->Parent());
	}

	const char * GetArchiveName(const ParamArchive * store)
	{
		return store->Name();
	}

}

