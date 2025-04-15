#pragma once

#include <vector>

#include "State.h"

template <class T>
class StateMachine
{
public:
    StateMachine() {};
    ~StateMachine();

    void Update(const float& elapsedTime); // 更新処理

    void DrawDebug();               // ImGui用

    void SetState(const int& newState);        // ステートセット
    void ChangeState(const int& newState);     // ステート変更
    void RegisterState(T* state);       // ステート登録

public:// 取得・設定
    int GetStateIndex();                // 現在のステート番号取得
    //T* GetCurrentState() { return currentState_; } // 現在のステート取得

private:
    T* currentState_;            // 現在のステート
    std::vector<T*> statePool_;  // 各ステートを保持する配列

};

template<class T>
inline StateMachine<T>::~StateMachine()
{
    // 登録したステートを削除する
    for (T* state : statePool_)
    {
        delete state;
    }
    statePool_.clear();
}

template<class T>
inline void StateMachine<T>::Update(const float& elapsedTime)
{
    currentState_->UpdateStateElapsedTime(elapsedTime);
    currentState_->Update(elapsedTime);
}

template<class T>
inline void StateMachine<T>::DrawDebug()
{
    if (ImGui::TreeNode("StateMachine"))
    {
        for (T* state : statePool_)
        {
            state->DrawDebug();
        }
        ImGui::TreePop();
    }

}

template<class T>
inline void StateMachine<T>::SetState(const int& newState)
{
    currentState_ = statePool_.at(newState);
    currentState_->Initialize();
}

template<class T>
inline void StateMachine<T>::ChangeState(const int& newState)
{
    currentState_->Finalize();
    currentState_->ResetStateElapsedTime();
    SetState(newState);
}

template<class T>
inline void StateMachine<T>::RegisterState(T* state)
{
    statePool_.emplace_back(state);
}

template<class T>
inline int StateMachine<T>::GetStateIndex()
{
    int i = 0;
    for (T* state : statePool_)
    {
        if (state == currentState_)
        {
            // i番号目のステートをリターン
            return i;
        }
        ++i;
    }

    // ステートが見つからなかったとき
    return -1;
}
