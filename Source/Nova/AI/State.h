#pragma once

#include <vector>

template<class T>
class State
{
public:
	State(T* owner) :owner_(owner) {}
	virtual ~State() {}

	virtual void Initialize() = 0;
	virtual void Update(const float& elapsedTime) = 0;
	virtual void Finalize() = 0;
	virtual void DrawDebug() = 0;

	//	----- ステート経過時間 -----
	virtual void	UpdateStateElapsedTime(const float& elapsedTime) { stateElapsedTime_ += elapsedTime; }
	void			ResetStateElapsedTime() { stateElapsedTime_ = 0.0f; }

protected:
	T* owner_;

	float		stateElapsedTime_ = 0.0f;		//	ステートに入ってからの経過時間

};