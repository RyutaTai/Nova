#pragma once

#include <windows.h>
#include <tchar.h>
#include <sstream>
#include <d3d11.h>
#include <wrl.h>

#include <DirectXMath.h>

#include "../Others/Misc.h"
#include "../Core/HighResolutionTimer.h"
#include "../Graphics/Graphics.h"
#include "../../Imgui/ImGuiCtrl.h"
#include "../Input/Input.h"
#include "../Audio/AudioManager.h"

CONST LPCWSTR APPLICATION_NAME{ L"Groove" };

class Framework
{
public:
	Framework(HWND hwnd);
	~Framework();

	Framework(const Framework&) = delete;
	Framework& operator=(const Framework&) = delete;
	Framework(Framework&&) noexcept = delete;
	Framework& operator=(Framework&&) noexcept = delete;

	int Run();

	LRESULT CALLBACK	HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static float		GetDeltaTime() { return tictoc_.GetDeltaTime(); }
	static double		GetDoubleDeltaTime() { return tictoc_.GetDoubleDeltaTime(); }

private:
	bool	Initialize();
	void	Update(const float& elapsedTime/*Elapsed seconds from last frame*/);
	void	Render();
	bool	Uninitialize();
	void	DrawDebug();

	void	CalculateFrameStats();

private:
    static HighResolutionTimer	tictoc_;
	float						elapsedTime_ = 0.0f;

	uint32_t					framesPerSecond_ = 0;
	float						fps_;					//	åªç›ÇÃfpsílÇï€éùÇ∑ÇÈïœêî
	std::vector<float>			fpsBuffer_ = {};		//	fpsêîï€ë∂óp
	const size_t				maxHistorySize_ = 100;	//	fpsÇï€ë∂Ç∑ÇÈç≈ëÂêî

	Graphics					graphics_;
	Input						input_;

};

