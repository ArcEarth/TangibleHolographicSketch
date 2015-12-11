#include "pch_bcl.h"
#include "Vicon.h"
#include <ViconReader\ViconRTClient.h>
#pragma comment(lib,"ViconReader.lib")

using namespace Causality::Devices;
using namespace Causality;

XM_ALIGNATTR
class ViconReaderImpl : public IViconClient, public ViconRTClient, public AlignedNew<XMVECTOR>
{
protected:
	void BodyDataToRigid(const BodyData & body, RigidTransform & rigid)
	{
		Quaternion q(body.QX, body.QY, body.QZ, body.QW);
		Vector3 t(body.TX, body.TY, body.TZ);

		rigid.Translation = t * m_world;
		rigid.Rotation = m_world.Rotation * q;
	}

public:
	virtual ~ViconReaderImpl()
	{
		stop();
	}

	ViconReaderImpl()
	{

	}

	// Inherited via IViconClient
	virtual const IsometricTransform & GetWorldTransform() override
	{
		return m_world;
	}
	virtual void SetWorldTransform(const IsometricTransform & transform) override
	{
		m_world = transform;
	}
	virtual RigidTransform GetRigid(int index) override
	{
		RigidTransform rigid;
		auto body = getBodyDataByID(index);
		BodyDataToRigid(body,rigid);
		return rigid;
	}
	virtual RigidTransform GetRigid(const string & name) override
	{
		RigidTransform rigid;
		auto body = this->getBodyData(name);
		BodyDataToRigid(body, rigid);
		return rigid;
	}
	virtual int GetRigidID(const string & name) override
	{
		return this->getBodyChannelID(name);
	}

	// Inherited via IViconClient
	virtual bool Start() override
	{
		return this->start();
	}

	virtual void Stop() override
	{
		this->stop();
	}

	virtual bool Update() override
	{
		this->update();
		return true;
	}
	// Inherited via IViconClient
	virtual bool Initialize(const ParamArchive * archive) override
	{
		string severIP;
		GetParam(archive, "vicon", severIP);
		GetParam(archive, "translation", m_world.Translation);
		GetParam(archive, "scale", m_world.Scale);
		Vector3 eular;
		GetParam(archive, "rotation", eular);
		m_world.Rotation = Quaternion::CreateFromYawPitchRoll(eular.y, eular.x, eular.z);
		this->init(severIP);
		return true;
	}
	virtual bool IsStreaming() override
	{
		return true;
	}

private:
	IsometricTransform m_world;

};

wptr<ViconReaderImpl> g_wpVicon;

sptr<IViconClient> IViconClient::GetFroCurrentView()
{
	if (!g_wpVicon.expired())
		return g_wpVicon.lock();
	return nullptr;
}

sptr<IViconClient> IViconClient::Create(const std::string &serverIP)
{
	sptr<ViconReaderImpl> pClient;
	if (g_wpVicon.expired())
	{
		pClient.reset(new ViconReaderImpl());
		g_wpVicon = pClient;
	}
	else
	{
		auto pClient = g_wpVicon.lock();
		pClient->stop();
	}
	pClient->init(serverIP, "ARSketch");
	return pClient;
}

IViconClient::~IViconClient()
{
}
