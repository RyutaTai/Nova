#pragma once

#include "Scene.h"

#define UseOldAudioManager 0

class SceneManager
{
private:
	SceneManager() {}
	~SceneManager();

public: 
	static SceneManager& Instance()
	{
		static SceneManager instance;
		return instance;
	}

	void Update(const float& elapsedTime);

	void ShadowRender();
	void Render();
	void DrawDebug();

	void Clear();
	void ChangeScene(Scene* scene);

	float GetCurrentSceneTime() { return currentSceneTime_; }

private:
	Scene* currentScene_ = nullptr;		//	現在のシーン
	Scene* nextScene_ = nullptr;		//	次のシーン

	float currentSceneTime_ = 0.0f;	//	現在のシーンの経過時間

};

