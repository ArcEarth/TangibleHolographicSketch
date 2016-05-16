#pragma once
#include <typeindex>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <filesystem>
#include <ppltasks.h>
#include <string>
#include "RenderSystemDecl.h"

namespace Causality
{
	namespace filesystem = std::experimental::filesystem;
	using path = filesystem::path;
	using concurrency::task;
	using std::type_index;
	using std::unordered_map;

	class asset_dictionary;

	// asset control block
	class asset_control_base
	{
	private:
		friend asset_dictionary;
	protected:
		type_index					_type;
		ptrdiff_t					_ref_count;
		asset_dictionary*			_dictionary;
		path						_ref_path;
		task<void*>					_load_task;

	public:
		asset_control_base()
		: _ref_count(0), _dictionary(nullptr), _type(typeid(void)) {}

		template <typename T>
		void reset(task<T*>&& loader, asset_dictionary* dict = nullptr, const path& source = path())
		{
			this->_dictionary = dict;
			this->_ref_path = source;
			this->_load_task = reinterpret_cast<task<void*>&&>(loader);
			_type = typeid(T);
		}

		template <typename T>
		void reset(T* _asset, asset_dictionary* dict = nullptr, const path& source = path())
		{
			this->_dictionary = dict;
			this->_ref_path = source;
			this->_load_task = task<void*>();
			_type = typeid(T);
		}

		asset_dictionary*			dictionary() const { return _dictionary; }

		path						source_path() const { return _ref_path; }
		inline bool					is_loaded() const { return !_load_task._GetImpl() || _load_task.is_done(); }

		ptrdiff_t					ref_count() const { return _ref_count; }

		ptrdiff_t					_inc_ref() { return ++_ref_count; }
		ptrdiff_t					_dec_ref() { return --_ref_count; }
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

		explicit asset_ptr(T* ptr, asset_dictionary* dict, const path& source)
			: _ptr(ptr)
		{
			_control = new asset_control_base();
			_control->_dictionary = dict;
			_control->_ref_path = source;
			_control->_inc_ref();
		}

		explicit asset_ptr(task<T*>&& loader, asset_dictionary* dict, const path& source)
			: _ptr(nullptr)
		{
			_control = new asset_control_base();
			_control->_dictionary = dict;
			_control->_ref_path = source;
			_control->_load_task = reinterpret_cast<task<void*>&&>(loader);
			_control->_inc_ref();
		}

		asset_ptr() : _ptr(nullptr), _control(nullptr) {}

		void reset() {
			if (_control && !_control->_dec_ref())
			{
				_internal_destruct();
			}
		}

		void reset(T* ptr, asset_dictionary* dict, const path& source)
		{
			if (this->_control && this->_control->_ref_count == 1)
			{
				if (this->_ptr) { delete this->_ptr; this->_ptr = nullptr; }
			} else {
				this->reset();
				this->_control = new asset_control_base();
			}
			this->_control->_dictionary = dict;
			this->_control->_ref_path = source;
			this->_control->_load_task = task<void*>();

			this->_ptr = ptr;
		}

		~asset_ptr()
		{
			this->reset();
		}

		void _internal_destruct()
		{
			if (this->_control) { delete this->_control; this->_control = nullptr; }
			if (this->_ptr) { delete this->_ptr; this->_ptr = nullptr; }
		}

		asset_ptr& operator=(const asset_ptr& rhs)
		{
			this->reset();
			this->_ptr = rhs._ptr;
			this->_control = rhs._control;
			if (this->_ptr && this->_control)
				this->_control->_inc_ref();
			return *this;
		}

		asset_ptr& operator=(asset_ptr&& rhs)
		{
			swap(rhs);
			return *this;
		}

		asset_ptr(const asset_ptr& rhs) { *this = rhs; }
		asset_ptr(asset_ptr&& rhs) { *this = std::move(rhs); }

		void swap(asset_ptr& rhs)
		{
			std::swap(this->_control, rhs._control);
			std::swap(this->_ptr, rhs._ptr);
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

	struct ArmatureFrameAnimation;
	class BehavierSpace;
	class StaticArmature;

	class asset_dictionary : public std::unordered_map<std::string, asset_ptr<void>>
	{
	public:
		using string = std::string;
		using mesh_type = IModelNode;
		using texture_type = Texture;
		using audio_clip_type = int;
		using animation_clip_type = ArmatureFrameAnimation;
		using behavier_type = BehavierSpace;
		using armature_type = StaticArmature;
		using effect_type = IEffect;
		using material_type = IMaterial;
		using dictionary_type = std::unordered_map<std::string, asset_ptr<void>>;

		typedef std::function<void*(const path&)> creation_function_type;

		path	asset_directory;
		path	texture_directory;
		path	mesh_directory;
		path	animation_directory;

	private:
		unordered_map<type_index, creation_function_type> creators;
		std::mutex m_mutex;

	public:
		~asset_dictionary()
		{
		}

		// Helper for lazily creating a D3D resource.
		template<typename T>
		asset_ptr<T> create_asset(const creation_function_type& createFunc)
		{
			T* result = assetPtr.get();

			// Double-checked lock pattern.
			//MemoryBarrier();

			if (!result)
			{
				std::lock_guard<std::mutex> lock(mutex);

				result = comPtr.Get();

				if (!result)
				{
					// Create the new object.
					createFunc(&assetPtr);

					assetPtr.attach(result);
				}
			}

			return result;
		}

		template<class _Ty>
		asset_ptr<_Ty> get_asset(const string& key) const
		{

		}

	};
}
