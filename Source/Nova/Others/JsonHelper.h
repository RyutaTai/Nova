#pragma once

#include "../nlohmann/json.hpp"
#include <DirectXMath.h>

namespace DirectX
{
    inline void to_json(nlohmann::json& j, const XMFLOAT2& v) 
    {
        j = nlohmann::json{ {"x", v.x}, {"y", v.y} };
    }

    inline void from_json(const nlohmann::json& j, XMFLOAT2& v)
    {
        j.at("x").get_to(v.x);
        j.at("y").get_to(v.y);
    }

    inline void to_json(nlohmann::json& j, const XMFLOAT3& v)
    {
        j = nlohmann::json{ {"x", v.x}, {"y", v.y}, {"z", v.z} };
    }

    inline void from_json(const nlohmann::json& j, XMFLOAT3& v) 
    {
        j.at("x").get_to(v.x);
        j.at("y").get_to(v.y);
        j.at("z").get_to(v.z);
    }

    inline void to_json(nlohmann::json& j, const XMFLOAT4& v) 
    {
        j = nlohmann::json{ {"x", v.x}, {"y", v.y}, {"z", v.z}, {"w", v.w} };
    }

    inline void from_json(const nlohmann::json& j, XMFLOAT4& v) 
    {
        j.at("x").get_to(v.x);
        j.at("y").get_to(v.y);
        j.at("z").get_to(v.z);
        j.at("w").get_to(v.w);
    }

    inline void to_json(nlohmann::json& j, const XMFLOAT4X4& m) 
    {
        j = {
            {"_11", m._11}, {"_12", m._12}, {"_13", m._13}, {"_14", m._14},
            {"_21", m._21}, {"_22", m._22}, {"_23", m._23}, {"_24", m._24},
            {"_31", m._31}, {"_32", m._32}, {"_33", m._33}, {"_34", m._34},
            {"_41", m._41}, {"_42", m._42}, {"_43", m._43}, {"_44", m._44}
        };
    }

    inline void from_json(const nlohmann::json& j, XMFLOAT4X4& m)
    {
        j.at("_11").get_to(m._11); j.at("_12").get_to(m._12);
        j.at("_13").get_to(m._13); j.at("_14").get_to(m._14);
        j.at("_21").get_to(m._21); j.at("_22").get_to(m._22);
        j.at("_23").get_to(m._23); j.at("_24").get_to(m._24);
        j.at("_31").get_to(m._31); j.at("_32").get_to(m._32);
        j.at("_33").get_to(m._33); j.at("_34").get_to(m._34);
        j.at("_41").get_to(m._41); j.at("_42").get_to(m._42);
        j.at("_43").get_to(m._43); j.at("_44").get_to(m._44);
    }
}