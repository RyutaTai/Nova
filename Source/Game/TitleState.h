#pragma once

#include "../Nova/AI/State.h"
#include "../Nova/Scenes/SceneTitle.h"

namespace TitleState
{
	class FadeInState :public State<SceneTitle>
	{
	public:
		FadeInState(SceneTitle* owner) :State(owner) {}
		~FadeInState(){}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	private:
		float logoAlpha_ = 0.0f;
		float fadeTime_ = 2.0f;

	};
}

namespace TitleState
{
	class MainState : public State<SceneTitle>
	{
	public:
		MainState(SceneTitle* owner) : State(owner) {}
		~MainState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	};
}

namespace TitleState
{
	class SettingState : public State<SceneTitle>
	{
	public:
		SettingState(SceneTitle* owner) : State(owner) {}
		~SettingState() {}

		void Initialize()override;
		void Update(const float& elapsedTime)override;
		void Finalize()override;
		void DrawDebug()override;

	};
}
