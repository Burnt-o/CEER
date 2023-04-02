#pragma once
#include "multilevel_pointer.h"

struct matrix
{
	float data[4][4];

	matrix& operator*(matrix other)
	{

		matrix mOut;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				double sum = 0.0;
				for (int k = 0; k < 4; k++)
					sum += data[i][k] * other.data[k][j];
				mOut.data[i][j] = sum;
			}
		}

		return mOut;
	}


	matrix(float m11, float m12, float m13, float m14,
		float m21, float m22, float m23, float m24,
		float m31, float m32, float m33, float m34,
		float m41, float m42, float m43, float m44)
	{
		data[0][0] = m11; data[0][1] = m12; data[0][2] = m13; data[0][3] = m14;
		data[1][0] = m21; data[1][1] = m22; data[1][2] = m23; data[1][3] = m24;
		data[2][0] = m31; data[2][1] = m32; data[2][2] = m33; data[2][3] = m34;
		data[3][0] = m41; data[3][1] = m42; data[3][2] = m43; data[3][3] = m44;
	}

	matrix() = default;
};

class MirrorMode
{
private:
	// constructor and destructor private
	explicit MirrorMode() = default;
	~MirrorMode() = default;

	
	// hook objects
	safetyhook::MidHook mHookCamera;
	static inline std::mutex mDestructionGuard; // Protects against D3D11Hook singleton destruction while hooks are executing

	void* p_cameraData;
	static void mirrorCamera(SafetyHookContext& ctx);
	matrix reflectionMatrix = matrix(1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);


public:
	static MirrorMode& get() {
		static MirrorMode instance;
		return instance;
	}


	static void initialize()
	{
		//auto mlp_cameraData = MultilevelPointer::make(L"halo1.dll", { 0x01AC7840, 0xF0 });
		auto mlp_cameraData = MultilevelPointer::make(L"halo1.dll", { 0x01AC7840, 0x130 });
		mlp_cameraData->resolve(&get().p_cameraData);


		// this really should be a modulehook
		auto mlp_cameraHook = MultilevelPointer::make(L"halo1.dll", { 0x2E91FD });
		void* p_cameraHook;
		if (!mlp_cameraHook->resolve(&p_cameraHook))
		{
			PLOG_FATAL << "blah mirror thing didn't resolve hook ptr";
		}
		get().mHookCamera = safetyhook::create_mid(p_cameraHook, mirrorCamera);
		PLOG_VERBOSE << "reflection matrix for editing: " << &get().reflectionMatrix;

	}
	static void enable()
	{

	
	}

	static void destroy()
	{
		std::scoped_lock<std::mutex> lock(mDestructionGuard);
		get().mHookCamera.reset();
		get().~MirrorMode();
	}
};

