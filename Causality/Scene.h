#pragma once
#include "SceneObject.h"
#include "StepTimer.h"
#include "RenderSystemDecl.h"
#include <Textures.h>
#include <mutex>
#include <vector>

namespace DirectX
{
	class StepTimer;
}

namespace Causality
{
	class Frame;
	class ICamera;
	class IVisual;
	class ILight;
	class AssetDictionary;

	using DirectX::StepTimer;

	interface IScene abstract
	{
	public:
		virtual ~IScene() {};

		virtual void Update() = 0;
		virtual void Render(IRenderContext* context) = 0;
		virtual void Load() = 0;
		virtual void Release() = 0;

		virtual bool IsLoading() const = 0;
		virtual bool IsReleasing() const = 0;

		virtual bool IsLoaded() const = 0;

		virtual void OnNavigatedTo() = 0;
		virtual void OnNavigatedFrom() = 0;

		virtual SceneObject* Content() = 0;
	};

	enum SceneTimeLineType
	{
		PhysicalTimeInterval, // The time of scene is binded to physical time
		FixedTimeInterval,	  // The time of scene is binded to frame count, every two adjacant frame have same time delta
	};

	using std::vector;
	// A Scene is a collection of it's contents, interactive logic, 
	class Scene : public IScene
	{
	public:
		Scene();
		~Scene();

		static uptr<Scene> LoadSceneFromXML(const string& xml_file);

		void LoadFromXML(const string& xml_file);

		static Scene& GetSceneForCurrentView();

		// Inherit from IScene
		virtual void Update() override;
		virtual void Render(IRenderContext* context) override;
		virtual void Load() override;
		virtual void Release() override;

		virtual bool IsLoading() const override;
		virtual bool IsReleasing() const override;

		virtual bool IsLoaded() const override;

		virtual void OnNavigatedTo() override;
		virtual void OnNavigatedFrom() override;

		// Timeline operations
		bool	IsPaused() const { return is_paused; }
		void	Pause() { is_paused = true; }
		void	Resume() { is_paused = false; }

		time_seconds	GetLocalTime() const
		{
			return time_seconds(step_timer.GetTotalSeconds());
		}

		double	GetTimeScale() const { return time_scale; }
		void	SetTimeScale(double time_scale) { this->time_scale = time_scale; }

		// Contens operations
		SceneObject*	Content() override;
		SceneObject*	SetContent(SceneObject* sceneRoot);

		HUDCanvas*		GetHudCanvas();
		void			SetHudCanvas(HUDCanvas* canvas);

		AssetDictionary& Assets() { return *assets; }
		const AssetDictionary& Assets() const { return *assets; }

		ICamera *PrimaryCamera();

		bool SetAsPrimaryCamera(ICamera* camera);

		IRenderDevice*			GetRenderDevice() { return render_device.Get(); }
		IRenderContext*			GetRenderContext() { return render_context.Get(); }
		I2DContext*				Get2DContext() { return m_2dContext.Get(); }
		I2DFactory*				Get2DFactory() { return m_2dFactory.Get(); }
		ITextFactory*			GetTextFactory() { return m_textFactory.Get(); }

		void SetRenderDeviceAndContext(IRenderDevice* device, IRenderContext* context);
		void SetHudRenderDevice(I2DFactory* pD2dFactory, ITextFactory* pTextFactory);

		DirectX::RenderTarget&			Canvas() { return scene_canvas; }
		const DirectX::RenderTarget&	Canvas() const { return scene_canvas; }
		void							SetCanvas(DirectX::RenderTarget& canvas);

		vector<ICamera*>&				GetCameras() { return cameras; }
		vector<ILight*>&				GetLights() { return lights; }
		const vector<const ICamera*>&	GetCameras() const { return reinterpret_cast<const vector<const ICamera*>&>(cameras); }
		const vector<const ILight*>&	GetLights() const { return reinterpret_cast<const vector<const ILight*>&>(lights); }
		vector<DirectX::IEffect*>&		GetEffects();

		void SetupEffectsViewProject(IEffect* pEffect, const DirectX::XMMATRIX &v, const DirectX::XMMATRIX &p);
		void SetupEffectsLights(IEffect* pEffect);

		void UpdateRenderViewCache();

		void SignalCameraCache();

		std::mutex&	ContentMutex();

	private:
		SceneTimeLineType			timeline_type;
		double						time_scale;
		StepTimer					step_timer;

		cptr<IRenderDevice>			render_device;
		cptr<IRenderContext>		render_context;
		cptr<I2DFactory>			m_2dFactory;
		cptr<ITextFactory>			m_textFactory;
		cptr<I2DContext>			m_2dContext;

		uptr<AssetDictionary>
									assets;

		uptr<SceneObject>			m_sceneRoot;
		uptr<HUDCanvas>				m_hudRoot;

		RenderTarget				scene_canvas;
		RenderableTexture2D			back_buffer;

		bool						is_paused;
		bool						is_loaded;
		int							loading_count;

		ICamera						*primary_cameral;

		// Caches
		vector<ICamera*>			cameras;
		vector<ILight*>				lights;
		vector<IVisual*>			renderables;
		vector<IEffect*>			effects;

		std::mutex					content_mutex;

		bool						camera_dirty;
		bool						object_dirty;
	};
}
