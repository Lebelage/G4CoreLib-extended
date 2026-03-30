//
// Created by sonora on 30.03.2026.
//
module;
#include <nlohmann/json.hpp>
export module GeantCore.Models.AlGaNModel;
export namespace GeantCore::Models {
    struct AlGanModel {
        uint16_t AbsorbedCount;
        uint16_t ReflectedCount;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AlGanModel, AbsorbedCount, ReflectedCount)
}
