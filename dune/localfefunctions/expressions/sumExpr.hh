// SPDX-FileCopyrightText: 2022 The dune-localfefunction developers mueller@ibb.uni-stuttgart.de
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once
#include <dune/localfefunctions/expressions/binaryExpr.hh>
#include <dune/localfefunctions/expressions/rebind.hh>
#include <dune/localfefunctions/linearAlgebraHelper.hh>

namespace Dune {

  template <typename E1, typename E2>
  class SumExpr : public BinaryExpr<SumExpr, E1, E2> {
  public:
    using Base = BinaryExpr<SumExpr, E1, E2>;
    using Base::Base;
    using Traits        = LocalFunctionTraits<SumExpr>;
    using LinearAlgebra = typename Base::E1Raw::LinearAlgebra;

    template <size_t ID_ = 0>
    static constexpr int orderID = std::max(Base::E1Raw::template order<ID_>(), Base::E1Raw::template order<ID_>());

    using ctype                    = typename Traits::ctype;
    static constexpr int valueSize = Traits::valueSize;
    static constexpr int gridDim   = Traits::gridDim;

    template <typename LocalFunctionEvaluationArgs_>
    auto evaluateValueOfExpression(const LocalFunctionEvaluationArgs_& localFunctionArgs) const {
      return Dune::eval(evaluateFunctionImpl(this->l(), localFunctionArgs)
                        + evaluateFunctionImpl(this->r(), localFunctionArgs));
    }

    template <int DerivativeOrder, typename LocalFunctionEvaluationArgs_>
    auto evaluateDerivativeOfExpression(const LocalFunctionEvaluationArgs_& localFunctionArgs) const {
      return Dune::eval(evaluateDerivativeImpl(this->l(), localFunctionArgs)
                        + evaluateDerivativeImpl(this->r(), localFunctionArgs));
    }
  };

  template <typename E1, typename E2>
  struct LocalFunctionTraits<SumExpr<E1, E2>> : public LocalFunctionTraits<std::remove_cvref_t<E1>> {
    using Base = LocalFunctionTraits<std::remove_cvref_t<E1>>;
  };

  template <typename E1, typename E2>
    requires IsLocalFunction<E1, E2>
  constexpr auto operator+(E1&& u, E2&& v) {
    static_assert(Concepts::AddAble<typename std::remove_cvref_t<E1>::FunctionReturnType,
                                    typename std::remove_cvref_t<E2>::FunctionReturnType>,
                  "The function values of your local functions are not addable!");
    return SumExpr<E1, E2>(std::forward<E1>(u), std::forward<E2>(v));
  }
}  // namespace Dune
