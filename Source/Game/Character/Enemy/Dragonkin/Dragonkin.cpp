#include "Dragonkin.h"

#include "DragonkinAction.h"
#include "DragonkinJudgment.h"
#include "../../Player/Player.h"
#include "../../../../Nova/AI/BehaviorData.h"
#include "../../../../Nova/Graphics/Graphics.h"
#include "../../../../Nova/Collision/Collision.h"
#include "../../../../Nova/Others/MemoryUtility.h"

//	コンストラクタ
Dragonkin::Dragonkin()
	:Enemy("./Resources/Model/Dragonkin/Dragonkin.gltf")
{
	//	----- 自身の種類設定 -----
	myType_ = EnemyType::Dragonkin;

	//	----- モデルのルート設定 -----
	int rootNodeIndex = GetNodeIndex("root");
	SetRootJointIndex(rootNodeIndex);

	//	----- 当たり判定データ登録 -----
	RegisterCollisionData("./Resources/Json/DragonkinCollisionData.json");

	//	----- 索敵範囲設定 -----
	searchRange_ = 13.5f;

	//	当たり判定用高さ、半径設定
	radius_ = 3.0f;
	height_ = 10.0f;

	//	HP設定
	hp_ = MaxHp_;

	//	ビヘイビアツリー設定
	behaviorData_ = new BehaviorData();
	behaviorTree_ = new BehaviorTree(this);

	//	ノード追加
	//	(優先度0) Death  : 死亡したら選ばれる
	//	(優先度1) Damage : ダメージを受けたら選ばれる
	//	(優先度2) Search : プレイヤーを見つけていなければ選ばれる
	//	(優先度3) Battle : プレイヤーを見つけていたら選ばれる
	//			(ランダム1) AttackPunch : ランダム
	//			(ランダム2) AttackKick  : ランダム
	//			(ランダム3)	AttackWing  : ランダム
	//	(優先度4) Idle   : 上記のどれでもなければ選ばれる
	behaviorTree_->AddNode("", "Root", 0, BehaviorTree::SelectRule::Priority, nullptr, nullptr);
	{
		behaviorTree_->AddNode("Root", "Death", 0, BehaviorTree::SelectRule::Non, new DragonkinJudgment::DeathJudgment(this), new DragonkinAction::DeathAction(this));	//	死亡ノード(末端)
		behaviorTree_->AddNode("Root", "Damage", 1, BehaviorTree::SelectRule::Non, new DragonkinJudgment::DamageJudgment(this), new DragonkinAction::DamageAction(this));	//	ダメージノード(末端)
		behaviorTree_->AddNode("Root", "Search", 2, BehaviorTree::SelectRule::Non, new DragonkinJudgment::SearchJudgment(this), new DragonkinAction::SearchAction(this));		//	索敵ノード(末端)
		behaviorTree_->AddNode("Root", "Battle", 3, BehaviorTree::SelectRule::Random, new DragonkinJudgment::BattleJudgment(this), nullptr);									//	戦闘ノード(中間)
		{
			//	AttackPunchSequence
			behaviorTree_->AddNode("Battle", "AttackPunchSequence", 0, BehaviorTree::SelectRule::Sequence, nullptr, nullptr); // Sequenceノード
			{
				behaviorTree_->AddNode("AttackPunchSequence", "AttackPunch", 0, BehaviorTree::SelectRule::Non, nullptr, new DragonkinAction::AttackPunchAction(this)); // 実際の攻撃アクション
				behaviorTree_->AddNode("AttackPunchSequence", "AttackWait", 1, BehaviorTree::SelectRule::Non, nullptr, new DragonkinAction::AttackWaitAction(this));   // 攻撃後待機＆旋回
			}

			//	AttackKickSequence
			behaviorTree_->AddNode("Battle", "AttackKickSequence", 0, BehaviorTree::SelectRule::Sequence, nullptr, nullptr); // Sequenceノード
			{
				behaviorTree_->AddNode("AttackKickSequence", "AttackKick", 0, BehaviorTree::SelectRule::Non, nullptr, new DragonkinAction::AttackKickAction(this));     // 実際の攻撃アクション
				behaviorTree_->AddNode("AttackKickSequence", "AttackWait", 1, BehaviorTree::SelectRule::Non, nullptr, new DragonkinAction::AttackWaitAction(this));     // 攻撃後待機＆旋回
			}

			//	AttackWingSequence
			behaviorTree_->AddNode("Battle", "AttackWingSequence", 0, BehaviorTree::SelectRule::Sequence, nullptr, nullptr); // Sequenceノード
			{
				behaviorTree_->AddNode("AttackWingSequence", "AttackWing", 0, BehaviorTree::SelectRule::Non, nullptr, new DragonkinAction::AttackWingAction(this));     // 実際の攻撃アクション
				behaviorTree_->AddNode("AttackWingSequence", "AttackWait", 1, BehaviorTree::SelectRule::Non, nullptr, new DragonkinAction::AttackWaitAction(this));     // 攻撃後待機＆旋回
			}
		}
		behaviorTree_->AddNode("Root", "Idle", 4, BehaviorTree::SelectRule::Non, new DragonkinJudgment::IdleJudgment(this), new DragonkinAction::IdleAction(this));				//	待機ノード(末端)
	}
	activeNode_ = behaviorTree_->ActiveNodeInference(behaviorData_);
}

Dragonkin::~Dragonkin()
{
	activeNode_ = nullptr;		//	二重解放しないため
	SafeDelete(activeNode_);
	SafeDelete(behaviorTree_);
	SafeDelete(behaviorData_);
}

//	初期化
void Dragonkin::Initialize()
{
	//	位置設定
	GetTransform()->SetPosition({ 23.0f, 0.0f,3.0f });

	//	回転値設定
	GetTransform()->SetRotationY(DirectX::XMConvertToRadians(-182.499f));

	//	スケール設定
	GetTransform()->SetScaleFactor(0.015f);

	//	初期アニメーション設定
	PlayAnimation(Dragonkin::AnimationType::Idle01, true);
	SetAnimationSpeed(1.0f);
	
}

//	当たり判定登録
void Dragonkin::RegisterCollisionData(const std::string& jsonFileName)
{
	//	Json書き出しパスとファイル名を設定
	collisionDataJsonFileName_ = jsonFileName;

	//	Jsonから読み込んで当たり判定データを設定
	LoadCollisionDataFromJson(collisionDataJsonFileName_);

}

//	更新処理
void Dragonkin::Update(const float& elapsedTime)
{
	//	更新フラグがfalseなら処理しない
	if (updateFlag_ == false)return;

	Character::Update(elapsedTime);

	//	----- アニメーション更新処理 -----
	UpdateAnimation(elapsedTime);

	//	----- ターゲット位置更新 -----
	UpdateTargetPosition();

	//	----- ビヘイビアツリー更新 -----
	UpdateBehaviorTree(elapsedTime);

	//	----- 当たり判定更新 -----
	UpdateCollisions(elapsedTime);

}

//	ビヘイビアツリー更新処理
void Dragonkin::UpdateBehaviorTree(const float& elapsedTime)
{
	//	ビヘイビアツリー更新フラグがfalseなら更新しない
	if (behaviorTreeUpdateFlag_ == false)return;

	//	現在実行するノードがあれば
	if (activeNode_ != nullptr)
	{
		//	ビヘイビアツリーからノードを実行
		activeNode_ = behaviorTree_->Run(activeNode_, behaviorData_, elapsedTime);
	}
	//	現在実行されているノードが無ければ
	if (activeNode_ == nullptr)
	{
		//	次に実行するノードを推論する
		activeNode_ = behaviorTree_->ActiveNodeInference(behaviorData_);
	}
}

//	ステージとの当たり判定
bool Dragonkin::RayVsVertical(const float& elapsedTime)
{

	return false;
}

bool Dragonkin::RayVsHorizontal(const float& elapsedTime)
{

	return false;
}

//	アニメーション
void Dragonkin::PlayAnimation(const AnimationType& animType, const bool& loop, const float& blendTime, const float& animSpeed, const float& startFrame, const float& endFrame)
{
	Character::PlayAnimation(static_cast<int>(animType), loop, blendTime, animSpeed, startFrame, endFrame);
}

//	当たり判定更新
void Dragonkin::UpdateCollisions(const float& elapsedTime)
{
	//	----- くらい判定更新 -----
	for (DamageDetectionData& data : damageDetectionData_)
	{
		//	ジョイントの名前で位置設定(名前がジョイントの名前ではないとき別途更新必要)
		data.SetJointPosition(GetJointPosition(data.GetUpdateName(), data.GetOffsetPosition()));

		data.Update(elapsedTime);
	}

	//	----- 攻撃判定更新 -----
	for (AttackDetectionData& data : attackDetectionData_)
	{
		//	ジョイントの名前で位置設定(名前がジョイントの名前ではないとき別途更新必要)
		data.SetJointPosition(GetJointPosition(data.GetUpdateName(), data.GetOffsetPosition()));
	}

	//	----- 押し出し判定更新 -----
	for (CollisionDetectionData& data : collisionDetectionData_)
	{
		//	ジョイントの名前で位置設定(名前がジョイントの名前ではないとき別途更新必要)
		DirectX::XMFLOAT3 pos = GetJointPosition(data.GetUpdateName(), data.GetOffsetPosition());

		//	Y軸固定
		if (data.GetFixedY())
			pos.y = 0.0f;

		data.SetPosition(pos);
		//data.SetJointPosition(pos);
	}

}

//	破棄処理
void Dragonkin::Destroy()
{
	Enemy::Destroy();	//	自身を破棄
}

//	描画処理
void Dragonkin::Render()
{
	//	ステート設定
	Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
	Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
	Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);

	//	モデル描画
	Character::Render();
}

//	デバッグ描画
void Dragonkin::DrawDebug()
{
	if (ImGui::TreeNode(u8"Dragonkin 竜人"))
	{
		//	----- 更新フラグ -----
		ImGui::Checkbox("UpdateFlag", &updateFlag_);	//	更新フラグ

		//	----- ビヘイビアツリー -----
		std::string str = "";
		if (activeNode_ != nullptr)
		{
			str = activeNode_->GetName();
		}
		if (ImGui::TreeNode("BehaviorTree"))
		{
			ImGui::Text(u8"Behavior　%s", str.c_str());								//	現在のビヘイビア
			ImGui::Checkbox("BehaviorTreeUpdateFlag", &behaviorTreeUpdateFlag_);	//	ビヘイビアツリー更新フラグ
			behaviorTree_->DrawDebug();

			ImGui::TreePop();
		}

		Character::DrawDebug();

		ImGui::DragFloat("SearchRange", &searchRange_, 0.01f);

		ImGui::TreePop();
	}
}

//	デバッグプリミティブ描画
void Dragonkin::DrawDebugPrimitive()
{
	DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();

	//	衝突判定用のデバッグ球を描画
	debugRenderer->DrawCylinder(this->GetTransform()->GetPosition(), radius_, height_, DirectX::XMFLOAT4(0, 0, 0, 1));

	//	索敵範囲描画(円柱)
	debugRenderer->DrawCylinder(this->GetTransform()->GetPosition(), searchRange_, 1.0f, { 0,1,0.1f,1.0f });

	//	当たり判定表示
	Character::DrawDebugPrimitive();

}
