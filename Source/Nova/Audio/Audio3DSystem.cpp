#include "Audio3DSystem.h"

#include <corecrt_math_defines.h>
#include <algorithm>

#include "../../Nova/Others/MathHelper.h"

FLOAT32 Angle(const DirectX::XMFLOAT3& emitterPos, const DirectX::XMFLOAT3& listenerPos, const DirectX::XMFLOAT3& vector)
{
    //  リスナーからエミッターまでのベクトルを算出
    DirectX::XMFLOAT3 listenerToEmitter = emitterPos - listenerPos;
    listenerToEmitter = Normalize(listenerToEmitter);

    //  軸ベクトル
    DirectX::XMVECTOR Axis = DirectX::XMLoadFloat3(&vector);
    Axis = DirectX::XMVectorSetY(Axis, 0.0f);
    //  正規化
    Axis = DirectX::XMVector3Normalize(Axis);

    //  内積から角度を算出
	float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&listenerToEmitter), Axis));

    //  範囲チェック(-1.0f ～ +1.0f)
    dot = std::clamp(dot, -1.0f, 1.0f);

    return acosf(dot);
   
}

void DSP(SoundDSPSetting& dspSetting, const SoundListener& listener, const SoundEmitter& emitter)
{
    //  リスナーからエミッターまでの距離
    dspSetting.distanceListnerToEmitter_ = Length(emitter.position_, listener.position_);

    // ドップラー効果
    dspSetting.dopplerScale_ = (SPEED_OF_SOUND - (listener.velocity_.x + listener.velocity_.y + listener.velocity_.z)) /
                                    (SPEED_OF_SOUND - (emitter.velocity_.x + emitter.velocity_.y + emitter.velocity_.z));

    //  リスナーからエミッターまでの角度
    float debugAngle = Angle(emitter.position_, listener.position_, listener.rightVec_);
	if (debugAngle < M_PI * 0.5f)
    {
        debugAngle = Angle(emitter.position_, listener.position_, listener.frontVec_);
    }
    else
    {
        debugAngle = -Angle(emitter.position_, listener.position_, listener.frontVec_);
    }

    dspSetting.radianListenerToEmitter_ = (Angle(emitter.position_, listener.position_,  listener.rightVec_) < M_PI * 0.5f) ?
                                                Angle(emitter.position_, listener.position_, listener.frontVec_) : -Angle(emitter.position_, listener.position_, listener.frontVec_);

    //  音の減衰率
    FLOAT32 scaler = std::clamp(1.0f - dspSetting.distanceListnerToEmitter_ / emitter.maxDistance_, 0.0f, 1.0f);

    //  チャンネル数によって音声行列の値を設定
    switch (dspSetting.srcChannelCount_ * dspSetting.dstChannelCount_)
    {
    case 1: //  音源:モノラル、出力:モノラル
        dspSetting.outputMatrix_[0] = scaler;
        break;

    case 2: //  音源:モノラル、出力:ステレオ
        {
            FLOAT32 angle = (Angle(emitter.position_, listener.position_, listener.rightVec_) < M_PI * 0.5f) ?
                dspSetting.radianListenerToEmitter_ : -Angle(emitter.position_, listener.position_, listener.frontVec_);
#if 1
			//angle = (dspSetting.radianListenerToEmitter_ + M_PI * 0.5f) * 0.5f;
            angle = static_cast<float>(((dspSetting.radianListenerToEmitter_ + 90.0f) / 2.0f) * (M_PI / 180.0f));
#else       
            angle = (dspSetting.radianListenerToEmitter_ + M_PI_2) * 0.5f;
#endif      
            FLOAT32 L = cosf(angle);
            FLOAT32 R = sinf(angle);
            if (dspSetting.distanceListnerToEmitter_ > emitter.minDistance_)
            {
                L *= scaler;
                R *= scaler;
            }

            dspSetting.outputMatrix_[0] = L;
            dspSetting.outputMatrix_[1] = R;

        }
        break;

    case 4: //  音源：ステレオ、出力：ステレオ
        {
            FLOAT32 angle = (Angle(emitter.position_, listener.position_, listener.rightVec_) < M_PI * 0.5f) ?
                dspSetting.radianListenerToEmitter_ : -Angle(emitter.position_, listener.position_, listener.frontVec_);
#if 1       
            angle = (dspSetting.radianListenerToEmitter_ + 90) * 0.5f;
#else       
            angle = (dspSetting.radianListenerToEmitter_ + M_PI_2) * 0.5f;
#endif      
            FLOAT32 L = cosf(angle);
            FLOAT32 R = sinf(angle);
            if (dspSetting.distanceListnerToEmitter_ > emitter.minDistance_)
            {
                L *= scaler;
                R *= scaler;
            }

            dspSetting.outputMatrix_[0] = dspSetting.outputMatrix_[2] = L;    //  左を0、2に変更
            dspSetting.outputMatrix_[1] = dspSetting.outputMatrix_[3] = R;    //  右を1、3に変更

        }
        break;
    }

    //  リスナーと音源の角度からローパスに適用する値を計算
    dspSetting.filterParam_ = (std::abs(dspSetting.radianListenerToEmitter_) > listener.innerRadius_) ?
        listener.filterParam_ * min(1.0f, (std::abs(dspSetting.radianListenerToEmitter_) - listener.innerRadius_) / (listener.outerRadius_ - listener.innerRadius_)) :
        dspSetting.filterParam_ = 0.0f;
}
