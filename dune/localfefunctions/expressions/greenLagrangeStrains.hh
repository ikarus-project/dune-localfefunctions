//
// Created by lex on 4/25/22.
//

#pragma once
#include "rebind.hh"

#include <dune/localfefunctions/expressions/expressionHelper.hh>
#include <dune/localfefunctions/expressions/unaryExpr.hh>
#include <dune/localfefunctions/helper.hh>

namespace Dune {

  template <typename E1>
  class GreenLagrangeStrains : public UnaryExpr<GreenLagrangeStrains, E1> {
  public:
    using Base = UnaryExpr<GreenLagrangeStrains, E1>;
    using Base::Base;
    using Traits = LocalFunctionTraits<GreenLagrangeStrains>;

    /** \brief Type used for coordinates */
    using ctype                           = typename Traits::ctype;
    static constexpr int strainSize       = Traits::valueSize;
    static constexpr int displacementSize = Base::E1Raw::valueSize;
    static constexpr int gridDim          = Traits::gridDim;

    static_assert(Base::E1Raw::template order<0>() == 1,
                  "Linear strain expression only supported for linear displacement function w.r.t. coefficients.");

    template <size_t ID_ = 0>
    static constexpr int orderID = 2;

    template <typename LFArgs>
    auto evaluateValueOfExpression(const LFArgs& lfArgs) const {
      const auto integrationPointPosition
          = returnIntegrationPointPosition(lfArgs.integrationPointOrIndex, this->m().basis());
      const auto referenceJacobian = this->m().geometry()->jacobianTransposed(integrationPointPosition);
      static_assert(std::is_same_v<typename decltype(referenceJacobian)::value_type, double>);
      const auto gradArgs = replaceWrt(lfArgs, wrt(DerivativeDirections::spatialAll));
      const auto gradu    = transposeEvaluated(evaluateDerivativeImpl(this->m(), gradArgs));

      Dune::FieldVector<ctype, strainSize> E;
      for (int i = 0; i < gridDim; ++i)
        E[i] = referenceJacobian[i] * gradu[i] + 0.5 * gradu[i].two_norm2();

      if constexpr (gridDim == 2) {
        const ctype v1 = referenceJacobian[0] * gradu[1];
        const ctype v2 = gradu[0] * referenceJacobian[1];
        const ctype v3 = gradu[0] * gradu[1];
        E[2]           = v1 + v2 + v3;
      } else if constexpr (gridDim == 3) {
        Dune::FieldVector<ctype, gridDim> a1 = referenceJacobian[0];
        a1 += gradu[0];
        Dune::FieldVector<ctype, gridDim> a2 = referenceJacobian[1];
        a2 += gradu[1];
        Dune::FieldVector<ctype, gridDim> a3 = referenceJacobian[2];
        a3 += gradu[2];
        E[3] = a2 * a3;
        E[4] = a1 * a3;
        E[5] = a1 * a2;
      }

      return E;
    }

    template <int DerivativeOrder, typename LFArgs>
    auto evaluateDerivativeOfExpression(const LFArgs& lfArgs) const {
      if constexpr (DerivativeOrder == 1 and LFArgs::hasSingleCoeff) {
        const auto integrationPointPosition
            = returnIntegrationPointPosition(lfArgs.integrationPointOrIndex, this->m().basis());
        const auto referenceJacobian
            = this->m().geometry()->jacobianTransposed(integrationPointPosition);  // the rows are X_{,1} and X_{,2}
        const auto gradArgs = replaceWrt(lfArgs, wrt(DerivativeDirections::spatialAll));
        const auto gradu
            = transposeEvaluated(evaluateDerivativeImpl(this->m(), gradArgs));  // the rows are u_{,1} and u_{,2}
        const auto gradArgsdI = addWrt(lfArgs, wrt(DerivativeDirections::spatialAll));
        const auto gradUdI    = evaluateDerivativeImpl(this->m(), gradArgsdI);  // derivative of grad u wrt I-th coeff

        Dune::FieldMatrix<double, strainSize, gridDim> bopI{};
        Dune::FieldVector<ctype, gridDim> g1 = referenceJacobian[0];
        g1 += gradu[0];
        if constexpr (displacementSize == 1) {
          coeff(bopI, 0, 0) = gradUdI[0].scalar() * g1[0];
        } else if constexpr (displacementSize == 2) {
          Dune::FieldVector<ctype, gridDim> g2 = referenceJacobian[1];
          g2 += gradu[1];
          const auto& dNIdT1 = gradUdI[0].scalar();
          const auto& dNIdT2 = gradUdI[1].scalar();
          row(bopI, 0)       = dNIdT1 * g1;                // dE11_dCIx,dE11_dCIy
          row(bopI, 1)       = dNIdT2 * g2;                // dE22_dCIx,dE22_dCIy
          row(bopI, 2)       = dNIdT2 * g1 + dNIdT1 * g2;  // 2*dE12_dCIx,2*dE12_dCIy
        } else if constexpr (displacementSize == 3) {
          Dune::FieldVector<ctype, gridDim> g2 = referenceJacobian[1];
          g2 += gradu[1];
          Dune::FieldVector<ctype, gridDim> g3 = referenceJacobian[2];
          g3 += gradu[2];
          const auto& dNIdT1 = gradUdI[0].scalar();
          const auto& dNIdT2 = gradUdI[1].scalar();
          const auto& dNIdT3 = gradUdI[2].scalar();
          row(bopI, 0)       = dNIdT1 * g1;                // dE11_dCIx,dE11_dCIy,dE11_dCIz
          row(bopI, 1)       = dNIdT2 * g2;                // dE22_dCIx,dE22_dCIy,dE22_dCIz
          row(bopI, 2)       = dNIdT3 * g3;                // dE33_dCIx,dE33_dCIy,dE33_dCIz
          row(bopI, 3)       = dNIdT3 * g2 + dNIdT2 * g3;  // dE23_dCIx,dE23_dCIy,dE23_dCIz
          row(bopI, 4)       = dNIdT3 * g1 + dNIdT1 * g3;  // dE13_dCIx,dE13_dCIy,dE13_dCIz
          row(bopI, 5)       = dNIdT2 * g1 + dNIdT1 * g2;  // dE12_dCIx,dE12_dCIy,dE12_dCIz
        }

        return bopI;

      } else if constexpr (DerivativeOrder == 1 and LFArgs::hasOneSpatialAll) {
        DUNE_THROW(Dune::NotImplemented, "Higher spatial derivatives of linear strain expression not implemented.");
        return createZeroMatrix<ctype, strainSize, gridDim>();
      } else if constexpr (DerivativeOrder == 1 and LFArgs::hasOneSpatialSingle) {
        DUNE_THROW(Dune::NotImplemented, "Higher spatial derivatives of linear strain expression not implemented.");
        return createZeroMatrix<ctype, strainSize, 1>();
      } else if constexpr (DerivativeOrder == 2) {
        if constexpr (LFArgs::hasNoSpatial and LFArgs::hasTwoCoeff) {
          const auto& S      = std::get<0>(lfArgs.alongArgs.args);
          using StressVector = std::remove_cvref_t<decltype(S)>;

          static_assert(StressVector::dimension == strainSize);

          const auto gradArgsdIJ         = addWrt(lfArgs, wrt(DerivativeDirections::spatialAll));
          const auto& [gradUdI, gradUdJ] = evaluateSecondOrderDerivativesImpl(this->m(), gradArgsdIJ);
          if constexpr (displacementSize == 1) {
            const auto& dNIdT1 = gradUdI[0].scalar();
            const auto& dNJdT1 = gradUdJ[0].scalar();
            const ctype val    = S[0] * dNIdT1 * dNJdT1;
            return createScaledIdentityMatrix<displacementSize, displacementSize>(val);
          } else if constexpr (displacementSize == 2) {
            const auto& dNIdT1 = gradUdI[0].scalar();
            const auto& dNIdT2 = gradUdI[1].scalar();
            const auto& dNJdT1 = gradUdJ[0].scalar();
            const auto& dNJdT2 = gradUdJ[1].scalar();
            const ctype val
                = S[0] * dNIdT1 * dNJdT1 + S[1] * dNIdT2 * dNJdT2 + S[2] * (dNIdT1 * dNJdT2 + dNJdT1 * dNIdT2);
            return createScaledIdentityMatrix<displacementSize, displacementSize>(val);
          } else if constexpr (displacementSize == 3) {
            const auto& dNIdT1 = gradUdI[0].scalar();
            const auto& dNIdT2 = gradUdI[1].scalar();
            const auto& dNIdT3 = gradUdI[2].scalar();
            const auto& dNJdT1 = gradUdJ[0].scalar();
            const auto& dNJdT2 = gradUdJ[1].scalar();
            const auto& dNJdT3 = gradUdJ[2].scalar();
            const ctype val    = S[0] * dNIdT1 * dNJdT1 + S[1] * dNIdT2 * dNJdT2 + S[2] * dNIdT3 * dNJdT3
                              + S[3] * (dNIdT2 * dNJdT3 + dNJdT2 * dNIdT3) + S[4] * (dNIdT1 * dNJdT3 + dNJdT1 * dNIdT3)
                              + S[5] * (dNIdT1 * dNJdT2 + dNJdT1 * dNIdT2);
            return createScaledIdentityMatrix<displacementSize, displacementSize>(val);
          }
        } else if constexpr (LFArgs::hasOneSpatial and LFArgs::hasSingleCoeff) {
          if constexpr (LFArgs::hasOneSpatialSingle and LFArgs::hasSingleCoeff) {
            DUNE_THROW(Dune::NotImplemented, "Higher spatial derivatives of linear strain expression not implemented.");
            return createZeroMatrix<ctype, strainSize, displacementSize>();
          } else if constexpr (LFArgs::hasOneSpatialAll and LFArgs::hasSingleCoeff) {
            DUNE_THROW(Dune::NotImplemented, "Higher spatial derivatives of linear strain expression not implemented.");
            return std::array<Dune::FieldMatrix<ctype, strainSize, displacementSize>, gridDim>{};
          }
        }
      } else if constexpr (DerivativeOrder == 3) {
        DUNE_THROW(Dune::NotImplemented, "Higher spatial derivatives of linear strain expression not implemented.");
        if constexpr (LFArgs::hasOneSpatialSingle) {
          return createZeroMatrix<ctype, displacementSize, displacementSize>();
        } else if constexpr (LFArgs::hasOneSpatialAll) {
          return createZeroMatrix<ctype, displacementSize, displacementSize>();
        } else
          static_assert(
              LFArgs::hasOneSpatialSingle or LFArgs::hasOneSpatialAll,
              "Only a spatial single direction or all spatial directions are supported. You should not end up here.");
      } else
        static_assert(DerivativeOrder > 3 or DerivativeOrder < 1,
                      "Only first, second and third order derivatives are supported.");
    }
  };

  template <typename E1>
  struct LocalFunctionTraits<GreenLagrangeStrains<E1>> {
    using E1Raw = std::remove_cvref_t<E1>;
    /** \brief Size of the function value */
    static constexpr int valueSize = E1Raw::valueSize == 1 ? 1 : (E1Raw::valueSize == 2 ? 3 : 6);
    /** \brief Type for the points for evaluation, usually the integration points */
    using DomainType = std::common_type_t<typename E1Raw::DomainType>;
    /** \brief Type used for coordinates */
    using ctype = std::common_type_t<typename E1Raw::ctype>;
    /** \brief Dimension of the grid */
    static constexpr int gridDim = E1Raw::gridDim;
    /** \brief Dimension of the world where this function is mapped to from the reference element */
    static constexpr int worldDimension = E1Raw::worldDimension;
  };

  template <typename E1>
  requires IsLocalFunction<E1>
  constexpr auto greenLagrangeStrains(E1&& u) { return GreenLagrangeStrains<E1>(std::forward<E1>(u)); }

}  // namespace Dune