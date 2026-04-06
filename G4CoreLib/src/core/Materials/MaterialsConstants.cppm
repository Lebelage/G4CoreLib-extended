//
// Created by bunny on 3.04.26.
//
module;
#include <G4Material.hh>
#include <G4SystemOfUnits.hh>

export module GeantCore.Core.Materials.MaterialsConstants;
export namespace GeantCore::Core::Materials {
    class MaterialsConstants {
    public:
        // Молярные массы (г/моль)
        static constexpr  double M_Al = 26.98;
        static constexpr double M_Ga = 69.72;
        static constexpr double M_In = 114.82;
        static constexpr double M_N  = 14.01;

        // Плотности базовых бинарных соединений (г/см3)
        static constexpr double DENS_AlN = 3.26;
        static constexpr double DENS_GaN = 6.15;
        static constexpr double DENS_InN = 6.81;

        // Ширина запрещенной зоны при T = 300K (эВ)
        static constexpr double EG_GAN = 3.44; // Базовое значение для GaN
        static constexpr double EG_ALN = 6.13; // Для расчета широкозонного барьера
        static constexpr double EG_INN = 0.70; // Для расчета узкозонной активной области

        // Параметры искривления (Bowing parameters) для закона Вегарда (эВ)
        static constexpr double B_ALGAN = 0.70; // Параметр для AlGaN
        static constexpr double B_INGAN = 1.43; // Параметр для InGaN

        // Энергия образования ЭДП для чистого GaN (эВ)
        static constexpr double W_EHP_GAN = 8.9; // Экспериментальное значение

        static constexpr float MAX_STEP_LIMIT = 1;
    };
}
