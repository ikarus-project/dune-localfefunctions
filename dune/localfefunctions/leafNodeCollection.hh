// SPDX-FileCopyrightText: 2022 The dune-localfefunction developers mueller@ibb.uni-stuttgart.de
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once
#include "meta.hh"

#include <dune/common/indices.hh>

// #include <ikarus/utils/linearAlgebraHelper.hh>
#include <ranges>

#include <dune/localfefunctions/helper.hh>
namespace Dune {
  namespace Impl {

    template <typename LocalFunctionImpl>
    class LocalFunctionInterface;

    template <typename LF>
      requires(!std::is_arithmetic_v<LF>)
    consteval int countNonArithmeticLeafNodesImpl() {
      constexpr auto predicate = [](auto v) { return v != Dune::arithmetic; };
      return std::ranges::count_if(LF::id, predicate);
    }

    template <typename LF>
      requires LocalFunction<LF>
    auto collectNonArithmeticLeafNodesImpl(LF&& a) {
      using LFRaw = std::remove_cvref_t<LF>;
      //    static_assert(LocalFunction<LF>,"Only passing LocalFunctions allowed");
      if constexpr (IsBinaryExpr<LFRaw>)
        return std::tuple_cat(collectNonArithmeticLeafNodesImpl(a.l()), collectNonArithmeticLeafNodesImpl(a.r()));
      else if constexpr (IsUnaryExpr<LFRaw>)
        return std::make_tuple(collectNonArithmeticLeafNodesImpl(a.m()));
      else if constexpr (IsArithmeticExpr<LFRaw>)
        return std::make_tuple();
      else if constexpr (IsNonArithmeticLeafNode<LFRaw>)
        return std::make_tuple(std::ref(a));
      else
        static_assert("There are currently no other expressions. Thus you should not end up here.");
    }

  }  // namespace Impl

  template <typename LF>
  consteval int countNonArithmeticLeafNodes() {
    return Impl::countNonArithmeticLeafNodesImpl<LF>();
  }
  template <typename LF>
  consteval int countNonArithmeticLeafNodes(const LocalFunctionInterface<LF>&) {
    return countNonArithmeticLeafNodes<LF>();
  }

  template <typename LF>
  //  requires LocalFunction<LF>
  auto collectNonArithmeticLeafNodes(LF&& a) {
    return Std::makeNestedTupleFlatAndStoreReferences(Impl::collectNonArithmeticLeafNodesImpl(a.impl()));
  }

  /** This class contains the collection of leaf nodes of a local function expression */
  template <typename LF>
    requires LocalFunction<LF>
  struct LocalFunctionLeafNodeCollection {
    using LFRaw = std::remove_cvref_t<LF>;
    /* Since we need to enable perfect forwarding we have to implement this universal constructor. We also constrain it
     * with requires LocalFunction<LF_> to only allow it for local function types. Without this template and a signature
     * as LocalFunctionLeafNodeCollection( LF&& lf): ... perfect forwarding is not working for constructors. See
     * https://eel.is/c++draft/temp.deduct.call#3
     * */
    template <typename LF_>
      requires LocalFunction<LF_>
    LocalFunctionLeafNodeCollection(LF_&& lf) : leafNodes{collectNonArithmeticLeafNodes(std::forward<LF_>(lf))} {}

    /** Return the a const reference of the coefficients if the leaf node with id tag I is unique. Otherwise this
     * function is deactivated */
    template <std::size_t I = 0>
      requires(std::ranges::count(LFRaw::id, I) == 1)
    auto& coefficientsRef(Dune::index_constant<I> = Dune::index_constant<I>()) {
      static_assert(
          std::ranges::count(LFRaw::id, I) == 1,
          "Non-const coefficientsRef() can only be called, if there is only one node with the given leaf node ID.");
      return std::get<I>(leafNodes).coefficientsRef();
    }

    /** Returns a const reference of the coefficients. */
    template <std::size_t I = 0>
    const auto& coefficientsRef(Dune::index_constant<I> = Dune::index_constant<0UL>()) const {
      return std::get<I>(leafNodes).coefficientsRef();
    }

    template <typename Derived, std::size_t I = 0>
    void addToCoeffs(const Eigen::MatrixBase<Derived>& correction,
                     Dune::index_constant<I> = Dune::index_constant<0UL>()) {
      Dune::Hybrid::forEach(leafNodes, [&]<typename LFI>(LFI& lfi) {
        if constexpr (LFI::id[0] == I) lfi.coefficientsRef() += correction;
      });
    }

    // This function updates the embedding values of the underlying coefficients
    template <typename Derived, std::size_t I = 0>
    void addToCoeffsInEmbedding(const Eigen::MatrixBase<Derived>& correction,
                                Dune::index_constant<I> = Dune::index_constant<0UL>()) {
      Dune::Hybrid::forEach(leafNodes, [&]<typename LFI>(LFI& lfi) {
        if constexpr (LFI::id[0] == I) addInEmbedding(lfi.coefficientsRef(), correction);
      });
    }

    template <std::size_t I = 0>
    auto& basis(Dune::index_constant<I> = Dune::index_constant<0UL>()) {
      return std::get<I>(leafNodes).basis();
    }

    template <std::size_t I = 0>
    auto& node(Dune::index_constant<I> = Dune::index_constant<0UL>()) {
      return std::get<I>(leafNodes);
    }

    constexpr auto size() { return Dune::index_constant<std::tuple_size_v<LeafNodeTuple>>(); }

    using LeafNodeTuple = decltype(collectNonArithmeticLeafNodes(std::declval<LF&&>()));

  private:
    LeafNodeTuple leafNodes;
  };

  template <typename LF>
    requires LocalFunction<LF>
  auto collectLeafNodeLocalFunctions(LF&& lf) {
    return LocalFunctionLeafNodeCollection<LF>(std::forward<LF>(lf));
  }
}  // namespace Dune
