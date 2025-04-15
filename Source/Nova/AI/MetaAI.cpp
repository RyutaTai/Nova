#if 0
#include "MetaAI.h"

#include <random>

static Meta* instance = nullptr;
Meta& Meta::Instance()
{
	return *instance;
}

Meta::Meta(Player* player, EnemyManager* eManager) : player_(player), enemyManager_(eManager)
{
	instance = this;
}

void Meta::Update()
{
	//	ここで自発的にメタAIが監視している処理があれば記載する
	//	今課題ではメタAIのイベントトリガーはメッセージを受信したときのため記述する処理は無し
}

//	メッセージ受信したときの処理
bool Meta::OnMessage(const Telegram& telegram)
{
	std::vector<Enemy*> enmVec;
	enmVec.clear();
	int enemyCount = enemyManager_->GetEnemyCount();
	switch (telegram.msg)
	{
	case MESSAGE_TYPE::MsgCallHelp:				//	誰かがプレイヤーを発見したので他の敵を呼ぶ
		for (int i = 0; i < enemyCount; ++i)	//	全ての敵のリスト
		{
			int enemyId = enemyManager_->GetEnemy(i)->GetId();
			//	送信者を除くエネミーにメッセージを送る
			if (telegram.sender != enemyId)
			{
				//	仲間を呼ぶメッセージの送信
				//	誰かがプレイヤーを発見したので送信者を除く敵にメッセージを送る
				//	メタAIから各敵に送るメッセージも MsgCallHelp です。

				SendMessaging((int)Meta::Identity::Meta, enemyId, MESSAGE_TYPE::MsgCallHelp);
			}
		}
		return true;
		break;
	case MESSAGE_TYPE::MsgChangeAttackRight:
		//	エネミーマネージャからAttackRange以内のエネミーだけの配列を作成し
		for (int i = 0; i < enemyCount; ++i)
		{
			Enemy* enemy = enemyManager_->GetEnemy(i);
			DirectX::XMFLOAT3 position = enemy->GetTransform()->GetPosition();
			DirectX::XMFLOAT3 targetPosition = player_->GetTransform()->GetPosition();
			float vx = targetPosition.x - position.x;
			float vy = targetPosition.y - position.y;
			float vz = targetPosition.z - position.z;
			float dist = sqrtf(vx * vx + vy * vy + vz * vz);

			if (dist < enemy->GetAttackRange())
			{
				enmVec.push_back(enemy);
			}
		}
		if (enmVec.size() > 0)
		{
			//	攻撃権の付与
			//	enmVecのサイズで乱数を取得し、取得した値をインデックス値として
			//	enmVecに登録された敵の一人に攻撃権を与える
			//	攻撃権を与えるメッセージはMsgGiveAttackRightです。
			//	random には、1 ～ enmVec.size() までの乱数が欲しい
			int random = 0;												//	ランダム値を格納
			int randomMax = static_cast<int>(enmVec.size()) - 1;		//	ランダム生成範囲(最大値)
			int receiver = 0;											//	受け手のIDを格納
			std::random_device rnd;										//	非決定的な乱数生成器を生成
			std::mt19937 mt(rnd());										//  メルセンヌ・ツイスタの32ビット版、引数は初期シード値
			std::uniform_int_distribution<> randEnemy(0, randomMax);	//	[0, randomMax-1] 範囲の一様乱数
			random	= randEnemy(mt) + 1;								//	ランダム値生成()　
			receiver = random * (int)Meta::Identity::Enemy;				//	受け手のID設定　EnemyのIDは5から、5ずつ増えていくため合わせる
			SendMessaging((int)Meta::Identity::Meta, receiver, MESSAGE_TYPE::MsgGiveAttackRight);
		}
		return true;
		break;
	case MESSAGE_TYPE::MsgAskAttackRight:	//	敵から、最初の攻撃権を要求された
		int count = 0;						//	全ての敵について、攻撃権を持っていないか調べる。
		for (int i = 0; i < enemyCount; ++i)
		{
			Enemy* enemy = enemyManager_->GetEnemy(i);
			if (enemy->GetAttackFlg()) count++;
		}
		//	他に攻撃権持っている敵がいないなら、要求してきた敵に攻撃権を与えるメッセージを送る。
		//	要求してきた敵のidはtelegramから取得出来ます。
		if (count < 1)
		{
			//	攻撃権の付与
			//	誰も攻撃権を持っていなければMetaAIから送信者に攻撃権付与のメッセージを送る
			//	メッセージはMsgGiveAttackRight
			SendMessaging((int)Meta::Identity::Meta, telegram.sender, MESSAGE_TYPE::MsgGiveAttackRight);
		}
		return true;
		break;
	}
	return false;
}

void Meta::SendMessaging(int sender, int receiver, MESSAGE_TYPE msg)
{
	if (receiver == static_cast<int>(Meta::Identity::Meta))	// MetaAI宛の時
	{
		//	メッセージデータを作成
		Telegram telegram(sender, receiver, msg);
		//	メッセージ送信
		OnMessage(telegram);
	}
	else
	{	
		//	エネミーが受信者のとき、どのエネミーに送信するか
		//	受信者のポインタを取得
		Enemy* receiveEnemy = enemyManager_->GetEnemyFromId(receiver);
		//	レシーバー居ないとき関数を終了する
		if (receiveEnemy == nullptr) return;
		//	メッセージデータを作成
		Telegram telegram(sender, receiver, msg);
		//	ディレイ無しメッセージ（即時配送メッセージ）
		receiveEnemy->OnMessage(telegram);
	}
}
#endif