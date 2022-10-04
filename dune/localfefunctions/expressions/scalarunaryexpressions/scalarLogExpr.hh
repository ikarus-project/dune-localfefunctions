/*
 * This file is part of the Ikarus distribution (https://github.com/IkarusRepo/Ikarus).
 * Copyright (c) 2022. The Ikarus developers.
 *
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#pragma once

#include <dune/localfefunctions/expressions/scalarunaryexpressions/scalarUnaryExpression.hh>

namespace Dune {

  struct LogFunc {
    template <typename ScalarType>
    static ScalarType value(const ScalarType& v) {
      return log(v);
    }

    template <typename ScalarType>
    static ScalarType derivative(const ScalarType& v) {
      return ScalarType(1) / v;
    }

    template <typename ScalarType>
    static ScalarType secondDerivative(const ScalarType& v) {
      return ScalarType(-1) / (v * v);
    }

    template <typename ScalarType>
    static ScalarType thirdDerivative(const ScalarType& v) {
      return ScalarType(2) / (v * v * v);
    }
  };
  template <typename E1>
  requires IsLocalFunction<E1>
  constexpr auto log(E1&& u) {
    static_assert(std::remove_cvref_t<E1>::valueSize == 1,
                  "Log expression only defined for scalar valued local functions.");
    return ScalarUnaryExpr<E1, LogFunc>(std::forward<E1>(u));
  }
}  // namespace Dune
