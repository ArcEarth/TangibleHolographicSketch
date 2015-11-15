#pragma once
#include "BCL.h"
#include "Textures.h"
#include "Models.h"
#include <boost\filesystem\path.hpp>
#include "Armature.h"
#include "CharacterBehavier.h"
#include "RenderSystemDecl.h"
#include <Effects.h>
#include <ppltasks.h>
#include <typeinfo>
#include "FbxParser.h"
#include <new>
#include <unordered_map>
#include <boost\any.hpp>

namespace Causality
{
	class AssetDictionary;

	using boost::filesystem::path;
	using concurrency::task;

	class asset_dictionary;

	// asset control block
	class asset_control_base
	{
		friend asset_dictionary;

	protected:
		ptrdiff_t					_ref_count;
		asset_dictionary*			_dictionary;
		path						_ref_path;
		bool						_loaded;
		task<void*>					_load_task;
		virtual						~asset_control_base() {}
	public:

		asset_dictionary*			dictionary() { return _dictionary; }
		const asset_dictionary*		dictionary() const { return _dictionary; }

		const path&					source_path() const { return _ref_path; }
		bool						is_loaded() { return _loaded; }

		ptrdiff_t					ref_count() const { return _ref_count; }

		ptrdiff_t					inc_ref() { return ++_ref_count; }
		ptrdiff_t					dec_ref() { return --_ref_count; }

		virtual bool				is_unified_chunck() const { return false; }
	};

	template <class T>
	class asset;

	template <class T>
	class asset : public asset_control_base
	{
	private:
		union
		{
			T	_Tdata;
			char _Bytes[sizeof(T)];
		};

		friend asset_dictionary;

	public:
		virtual ~asset() override
		{
			_Tdata.~T();
		}

		virtual bool				is_unified_chunck() const override
		{ return true; }


		T&						get() { return _Tdata; }
		T*						get_ptr() { return &_Tdata; }

		template<class... _Types> inline
		T*						internal_construct(_Types&&... _Args)
		{	// make a unique_ptr
			return new (&_Tdata) T(_STD forward<_Types>(_Args)...);
		}

	};

	template <class T>
	class asset_ptr
	{
	private:
		asset_control_base* _control;
		T*					_ptr;
	public:
		T* get() { return  _ptr; }
		const T* get() const { return  _ptr; }
		operator T*() { return _ptr; }
		operator const T*() const { return _ptr; }

		explicit asset_ptr(asset_dictionary* dict, const path& source, T* ptr)
			: _ptr(ptr)
		{
			_control = new asset_control_base();
			_control->_dictionary = dict;
			_control->_ref_path = source;
			_control->_loaded = true;
			_control->inc_ref();
		}

		explicit asset_ptr(asset_dictionary* dict, const path& source, task<T*>&& loader)
			: _ptr(nullptr)
		{
			_control = new asset_control_base();
			_control->_dictionary = dict;
			_control->_ref_path = source;
			_control->_loaded = false;
			_control->inc_ref();
			_control->_load_task = loader.then([this](T* result) {
				_ptr = result;
				_control->_loaded = true;
			});
		}

		asset_ptr() : _ptr(nullptr), _control(nullptr) {}
		~asset_ptr()
		{
			reset();
		}

		asset_ptr& operator=(const asset_ptr& rhs)
		{
			_ptr = rhs._ptr;
			_control = rhs._control;
			if (_ptr && _control)
				_control->inc_ref();
			return *this;
		}

		asset_ptr& operator=(asset_ptr&& rhs)
		{
			swap(rhs);
			return *this;
		}

		asset_ptr(const asset_ptr& rhs) { *this = rhs; }
		asset_ptr(asset_ptr&& rhs) { *this = std::move(rhs); }

		void reset() {
			if (_ptr && !_control->dec_ref())
			{
				bool is_same_block = _control->is_unified_chunck();

				delete _control;
				_control = nullptr;

				if (!is_same_block)
				{
					delete _ptr;
					_ptr = nullptr;
				}
			}
		}

		void attach(asset<T>* asset)
		{
			reset();
			_ptr = asset->get_ptr();
			_control = asset;
		}

		void swap(asset_ptr& rhs)
		{
			auto ptr = _ptr;
			_ptr = rhs._ptr;
			rhs._ptr = ptr;
			auto control = _control;
			_control = rhs._control;
			rhs._control = controll
		}
	public:
		asset_dictionary*		dictionary() { return !_control ? nullptr : _control->dictionary(); }
		const asset_dictionary*	dictionary() const { return !_control ? nullptr : _control->dictionary(); }

		bool					experied() const { return _control ? _control->ref_count() == 0 : true; }
		ptrdiff_t				ref_count() const { return !_control ? 0 : _control->ref_count(); }

		const path&				source_path() const { return _control->source_path(); }
		bool					is_loaded() { return !_control ? false : _control->is_loaded(); }
		task<T*>&				load_task() { return _control->_load_task; }
	};

	class asset_dictionary
	{
	public:
		using mesh_type = IModelNode;
		using texture_type = Texture;
		using audio_clip_type = int;
		using animation_clip_type = ArmatureFrameAnimation;
		using behavier_type = BehavierSpace;
		using armature_type = StaticArmature;
		using effect_type = IEffect;
		using material_type = IMaterial;

		typedef std::function<bool(void*, const char*)> creation_function_type;
		map<std::type_index, creation_function_type> creators;

	public:
		// Helper for lazily creating a D3D resource.
		template<typename T, typename TCreateFunc>
		static T* demand_create(asset_ptr<T>& assetPtr, std::mutex& mutex, TCreateFunc createFunc)
		{
			T* result = assetPtr.get();

			// Double-checked lock pattern.
			MemoryBarrier();

			if (!result)
			{
				std::lock_guard<std::mutex> lock(mutex);

				result = comPtr.Get();

				if (!result)
				{
					// Create the new object.
					ThrowIfFailed(
						createFunc(&assetPtr)
						);

					MemoryBarrier();

					assetPtr.attach(result);
				}
			}

			return result;
		}


		template<class _Ty, class... _Types> inline
			asset_ptr<_Ty> make_asset(const string& key, _Types&&... _Args)
		{
			auto ptr = new asset<_Ty>(std::forward(_Args)...);
		}

		template<class _Ty>
		asset_ptr<_Ty> get_asset(const string& key);

		template<>
		asset_ptr<mesh_type> get_asset(const string& key);

	private:
		std::unordered_map<string, asset_control_base*> _resources;
	};

	// Controls the asset name resolve and loading
	class AssetDictionary
	{
	public:
		using path = boost::filesystem::path;
		using mesh_type = IModelNode;
		using texture_type = Texture;
		using audio_clip_type = int;
		using animation_clip_type = ArmatureFrameAnimation;
		using behavier_type = BehavierSpace;
		using armature_type = StaticArmature;
		using effect_type = IEffect;
		using material_type = IMaterial;

		AssetDictionary();
		~AssetDictionary();

		//Synchronize loading methods
		mesh_type*		     LoadObjMesh(const string & key, const string& fileName);
		mesh_type*		     LoadFbxMesh(const string & key, const string& fileName, const std::shared_ptr<material_type> &pMaterial = nullptr);
		mesh_type*		     LoadFbxMesh(const string & key, const string& fileName, bool importMaterial);
		texture_type&	     LoadTexture(const string & key, const string& fileName);
		armature_type&	     LoadArmature(const string & key, const string& fileName);
		animation_clip_type& LoadAnimation(const string& key, const string& fileName);
		behavier_type*		 LoadBehavierFbx(const string & key, const string & fileName);
		behavier_type*		 LoadBehavierFbxs(const string & key, const string& armature, list<std::pair<string, string>>& animations);

		// Async loading methods
		task<mesh_type*>&			LoadMeshAsync(const string & key, const string& fileName);
		task<texture_type*>&		LoadTextureAsync(const string & key, const string& fileName);
		task<audio_clip_type*>&		LoadAudioAsync(const string & key, const string& fileName);

		bool IsAssetLoaded(const string& key) const;

		template<typename TAsset>
		TAsset&						GetAsset(const string& key)
		{
			return *boost::any_cast<TAsset*>(assets[key]);
		}

		mesh_type*					GetMesh(const string& key)
		{
			auto itr = meshes.find(key);
			if (itr != meshes.end())
				return itr->second;
			return nullptr;
		}

		texture_type&				GetTexture(const string& key)
		{
			auto itr = textures.find(key);
			if (itr != textures.end())
				return *itr->second;
			return *itr->second;
		}

		animation_clip_type&		GetAnimation(const string& key)
		{
			return animations[key];
		}

		behavier_type&				GetBehavier(const string& key)
		{
			return *behaviers[key];
		}

		audio_clip_type&			GetAudio(const string& key);

		effect_type*				GetEffect(const string& key)
		{
			return effects[key];
		}

		const sptr<material_type>&	GetMaterial(const string& key) const
		{
			return materials[key];
		}

		template<typename VertexType>
		const cptr<ID3D11InputLayout>& GetInputLayout(IEffect* pEffct = nullptr);

		IEffectFactory&		EffctFactory() { return *effect_factory; }

		template<typename TAsset>
		TAsset&						AddAsset(const string&key, TAsset* pAsset)
		{
			assets[key] = pAsset;
			return *pAsset;
		}

		material_type*				AddMaterial(const string& key, const sptr<material_type>& pMaterial)
		{
			materials[key] = pMaterial;
			return pMaterial.get();
		}

		template<typename TAsset>
		TAsset&						LoadAsset(const string& fileName, function<TAsset(const string& fileName, std::istream& file)> deserializer)
		{
		}

		template<typename TAsset>
		task<TAsset&>&				LoadAssetAsync(const string& fileName, function<TAsset(const string& fileName, std::istream& file)> deserializer)
		{
		}

		IRenderDevice* GetRenderDevice() { return render_device.Get(); }
		const IRenderDevice* GetRenderDevice() const { return render_device.Get(); }
		void SetRenderDevice(IRenderDevice* device);
		void SetParentDictionary(AssetDictionary* dict);
		void SetTextureDirectory(const path& dir);
		const path& GetTextureDirectory() const { return texture_directory; }
		void SetMeshDirectory(const path& dir);
		void SetAssetDirectory(const path& dir);

	private:
		cptr<IRenderDevice>					render_device;

		AssetDictionary*					parent_dictionary;

		path								asset_directory;
		path								texture_directory;
		path								mesh_directory;
		path								animation_directory;

		map<string, task<mesh_type*>>		loading_meshes;
		map<string, task<texture_type*>>	loading_textures;

		map<string, mesh_type*>				meshes;
		map<string, texture_type*>			textures;
		map<string, animation_clip_type>	animations;
		map<string, audio_clip_type>		audios;
		map<string, effect_type*>			effects;
		map<string, behavier_type*>			behaviers;
		mutable map<string, sptr<material_type>>	materials;

		sptr<IEffect>				default_effect;
		sptr<IEffect>				default_skinned_effect;
		sptr<IEffect>				default_envirument_effect;
		sptr<PhongMaterial>			default_material;
		uptr<IEffectFactory>		effect_factory;

		map<std::type_index, cptr<ID3D11InputLayout>> layouts;

		// other assets
		map<string, boost::any>					assets;

	public:

		auto GetEffects()
		{
			return adaptors::values(effects);
		}
	};

	template <typename VertexType>
	inline const cptr<ID3D11InputLayout>& AssetDictionary::GetInputLayout(IEffect * pEffct)
	{
		std::type_index type = typeid(VertexType);
		auto& pLayout = layouts[type];
		if (pLayout == nullptr)
		{
			void const* shaderByteCode;
			size_t byteCodeLength;
			pEffct->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
			pLayout = CreateInputLayout<VertexType>(render_device.Get(), shaderByteCode, byteCodeLength);
		}
		return pLayout;
	}

}