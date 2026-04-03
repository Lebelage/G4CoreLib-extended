//
// Created by bunny on 3.04.26.
//

export module GeantCore.Core.Materials.MaterialsConstatnts;
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
    };
}