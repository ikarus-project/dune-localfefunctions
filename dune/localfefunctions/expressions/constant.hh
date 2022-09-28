//
// Created by lex on 4/25/22.
//

#pragma once
#include <dune/localfefunctions/expressions/unaryExpr.hh>
#include <dune/localfefunctions/meta.hh>

namespace Dune {

  template <typename Type>
  requires std::is_arithmetic_v<Type>
  class ConstantExpr : public LocalFEFunctionInterface<ConstantExpr<Type>> {
  public:
    explicit ConstantExpr(Type val_) : val{val_} {}

    const Type& value() const { return val; }
    Type& value() { return val; }

    auto clone() const { return ConstantExpr(val); }

    template <typename OtherType, size_t ID = 0>
    auto rebindClone(OtherType&& t, Dune::index_constant<ID>&& id = Dune::index_constant<0>()) const {
      if constexpr (Arithmetic::value == ID)
        return ConstantExpr(static_cast<OtherType>(val));
      else
        return clone();
    }

    template <typename OtherType>
    struct Rebind {
      using other = ConstantExpr<OtherType>;
    };

    static constexpr bool isLeaf = true;
    using Ids                    = Arithmetic;
    template <size_t ID_ = 0>
    static constexpr int orderID = ID_ == Arithmetic::value ? linear : constant;

  private:
    Type val;
  };

  template <typename Type>
  struct LocalFEFunctionTraits<ConstantExpr<Type>> {
    static constexpr int valueSize = 1;
    /** \brief Type for the points for evaluation, usually the integration points */
    using DomainType = Dune::FieldVector<double, 0>;
  };

}  // namespace Ikarus