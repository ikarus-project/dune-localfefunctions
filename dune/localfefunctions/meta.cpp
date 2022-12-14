// SPDX-FileCopyrightText: 2022 The dune-localfefunction developers mueller@ibb.uni-stuttgart.de
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "meta.hh"
namespace Dune {

  On<DerivativeDirections::GridElement> on(DerivativeDirections::GridElement) { return {}; }
  On<DerivativeDirections::ReferenceElement> on(DerivativeDirections::ReferenceElement) { return {}; }

  namespace DerivativeDirections {
    SpatialPartial spatial(size_t i) { return {i}; }

    SingleCoeff<0> coeff(size_t i) {
      using namespace Dune::Indices;
      SingleCoeff<0> coeffs;
      std::get<1>(coeffs.index._data) = i;
      return coeffs;
    }

    TwoCoeff<0, 0> coeff(size_t i, size_t j) {
      using namespace Dune::Indices;
      TwoCoeff<0, 0> coeffs;
      std::get<1>(coeffs.index.first._data)  = i;
      std::get<1>(coeffs.index.second._data) = j;
      return coeffs;
    }
  }  // namespace DerivativeDirections
}  // namespace Dune
