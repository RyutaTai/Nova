#include "Audio3DSystem.h"

#include <corecrt_math_defines.h>

inline FLOAT32 VECTOR3Length(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
{
    return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z));
}

inline FLOAT32 Dot(const DirectX::XMFLOAT3& a)
{
    return sqrtf((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
}

#if 1
FLOAT32 Angle(DirectX::XMFLOAT3 point1, DirectX::XMFLOAT3 point2, DirectX::XMFLOAT3 vector)
{
    DirectX::XMFLOAT3 vectorListnerToEmitter =
    {
        point1.x - point2.x,
        point1.y - point2.y,
        point1.z - point2.z,
    };


    FLOAT32 frontDot = Dot(vector);
    DirectX::XMFLOAT3 frontNormalize = { vector.x / frontDot, vector.y / frontDot, vector.z / frontDot };

    FLOAT32 pointDot = Dot(vectorListnerToEmitter);
    DirectX::XMFLOAT3 pointNoramlize = { vectorListnerToEmitter.x / pointDot, vectorListnerToEmitter.y / pointDot, vectorListnerToEmitter.z / pointDot };

    return acosf(frontNormalize.x * pointNoramlize.x + frontNormalize.y * pointNoramlize.y + frontNormalize.z * pointNoramlize.z);
}
#else
FLOAT32 Angle(DirectX::XMFLOAT3 point_1, DirectX::XMFLOAT3 point_2, DirectX::XMFLOAT3 vector)
{
    DirectX::XMFLOAT3 vector_listner_to_emitter =
    {
        point_1.x - point_2.x,
        point_1.y - point_2.y,
        point_1.z - point_2.z,
    };

    // ベクトルの長さを計算
    FLOAT32 length_listener_to_emitter = sqrtf(vector_listner_to_emitter.x * vector_listner_to_emitter.x +
        vector_listner_to_emitter.y * vector_listner_to_emitter.y +
        vector_listner_to_emitter.z * vector_listner_to_emitter.z);

    // 正規化
    DirectX::XMFLOAT3 normalized_listener_to_emitter =
    {
        vector_listner_to_emitter.x / length_listener_to_emitter,
        vector_listner_to_emitter.y / length_listener_to_emitter,
        vector_listner_to_emitter.z / length_listener_to_emitter,
    };

    // ベクトルの長さを計算
    FLOAT32 length_vector = sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);

    // 正規化
    DirectX::XMFLOAT3 normalized_vector = { vector.x / length_vector, vector.y / length_vector, vector.z / length_vector };

    // 内積計算
    FLOAT32 dot_product = normalized_listener_to_emitter.x * normalized_vector.x +
        normalized_listener_to_emitter.y * normalized_vector.y +
        normalized_listener_to_emitter.z * normalized_vector.z;

    // acosf関数の範囲チェック
    dot_product = max(-1.0f, min(1.0f, dot_product));

    // 角度計算
    return acosf(dot_product);
}
#endif

void DSP(SoundDSPSetting& dspSetting, const SoundListener& listener, const SoundEmitter& emitter)
{
    //  距離
    dspSetting.distanceListnerToEmitter_ = VECTOR3Length(emitter.position_, listener.position_);

    // ドップラー効果
    dspSetting.dopplerScale_ = (SPEED_OF_SOUND - (listener.velocity_.x + listener.velocity_.y + listener.velocity_.z)) /
                                    (SPEED_OF_SOUND - (emitter.velocity_.x + emitter.velocity_.y + emitter.velocity_.z));

    //  角度
    dspSetting.radianListenerToEmitter_ = (Angle(emitter.position_, listener.position_,  listener.rightVec_) < M_PI * 0.5f) ?
                                                Angle(emitter.position_, listener.position_, listener.frontVec_) : -Angle(emitter.position_, listener.position_, listener.frontVec_);

    // 音の減衰率
    FLOAT32 scaler = max(0.0f, min(1.0f, 1.0f - dspSetting.distanceListnerToEmitter_ / emitter.maxDistance_));

    switch (dspSetting.srcChannelCount_ * dspSetting.dstChannelCount_)
    {
    case 1:
        dspSetting.outputMatrix_[0] = scaler;
        break;

    case 4:
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

#if 0    //  元のコード
        dspSetting.outputMatrix_[0] = dspSetting.outputMatrix_[1] = L;   
        dspSetting.outputMatrix_[2] = dspSetting.outputMatrix_[3] = R;
#else
        //  変更したら直った。
        dspSetting.outputMatrix_[0] = dspSetting.outputMatrix_[2] = L;    //  左を0、2に変更
        dspSetting.outputMatrix_[1] = dspSetting.outputMatrix_[3] = R;    //  右を1、3に変更
#endif

#if 0   //  dsp_setting.output_matrixの値が生きているか確認
        float pan = -90.0f;	//	真左
        float rad = ((-90.0f + 90.0f) / 2.0f) * (M_PI / 180.0f);	//  ラジアンに変換
        dspSetting.output_matrix[0] = cosf(rad);				    //  左ボリューム
        dspSetting.output_matrix[2] = cosf(rad);				    //  左ボリューム
        dspSetting.output_matrix[1] = sinf(rad);				    //  右ボリューム
        dspSetting.output_matrix[3] = sinf(rad);				    //  右ボリューム

#endif
        break;
    }


    //  リスナーと音源の角度からローパスに適用する値を計算
    dspSetting.filterParam_ = (std::abs(dspSetting.radianListenerToEmitter_) > listener.innerRadius_) ?
        listener.filterParam_ * min(1.0f, (std::abs(dspSetting.radianListenerToEmitter_) - listener.innerRadius_) / (listener.outerRadius_ - listener.innerRadius_)) :
        dspSetting.filterParam_ = 0.0f;
}
