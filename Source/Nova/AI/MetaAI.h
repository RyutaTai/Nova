#pragma once
#if 0
#include <set>

#include "Telegram.h"
#include "../../Game/Player.h"
#include "../../Game/EnemyManager.h"

class Meta
{
private:
	Player*			player_			= nullptr;
	EnemyManager*	enemyManager_	= nullptr;

public:
	enum class Identity : int
	{
		Meta	= 0,	//	MetaAI
		Player	= 1,	//	プレイヤーIDは1～4
		Enemy	= 5		//	エネミーIDは5以降を付与
	};
	//	コンストラクタ
	Meta(Player* player, EnemyManager* eManager);
	~Meta() {}
	//	インスタンス取得
	static Meta& Instance();
	//	MetaAIの機能を追加していく
	//	更新処理
	void Update();
	//	メッセージ受信したときの処理
	bool OnMessage(const Telegram& msg);
	//	メッセージ送信関数
	void SendMessaging(int sender, int receiver, MESSAGE_TYPE msg);

};
#endif