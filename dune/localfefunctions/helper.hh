// SPDX-FileCopyrightText: 2022 The dune-localfefunction developers mueller@ibb.uni-stuttgart.de
// SPDX-License-Identifier: LGPL-2.1-or-later

//
// Created by Alex on 27.05.2021.
//

#pragma once
#include "linalgconcepts.hh"

#include <concepts>
#include <tuple>
#include <type_traits>

#include <dune/common/hybridutilities.hh>
#include <dune/istl/bvector.hh>

#include <Eigen/Core>

namespace Dune {

  namespace Std {

    template <typename T, std::size_t N1, std::size_t N2>
    constexpr std::array<T, N1 + N2> concatArray(std::array<T, N1> lhs, std::array<T, N2> rhs) {
      std::array<T, N1 + N2> result{};
      std::size_t index = 0;

      for (auto &el : lhs) {
        result[index] = std::move(el);
        ++index;
      }
      for (auto &el : rhs) {
        result[index] = std::move(el);
        ++index;
      }

      return result;
    }

    // Forward declare functions
    template <typename... Types>
    auto makeNestedTupleFlat(std::tuple<Types...> tup);

    template <typename>
    struct is_tuple : std::false_type {};

    template <typename... T>
    struct is_tuple<std::tuple<T...>> : std::true_type {};

    template <class Tuple, class Type>
      requires is_tuple<Tuple>::value
    consteval int countType() {
      int count = 0;
      Dune::Hybrid::forEach(Dune::Hybrid::integralRange(Dune::index_constant<std::tuple_size_v<Tuple>>()), [&](auto i) {
        using currentType = std::remove_cvref_t<std::tuple_element_t<i, Tuple>>;
        if constexpr (std::is_same_v<currentType, Type>) ++count;
      });
      return count;
    }

    template <typename T, typename Tuple>
    struct hasType;

    template <typename T>
    struct hasType<T, std::tuple<>> : std::false_type {};

    template <typename T>
    struct hasType<T, T> : std::true_type {};

    template <typename T, typename U, typename... Ts>
    struct hasType<T, std::tuple<U, Ts...>> : hasType<T, std::tuple<Ts...>> {};

    template <typename T, typename... Ts>
    struct hasType<T, std::tuple<T, Ts...>> : std::true_type {};

    template <template <typename...> class, typename...>
    struct isSpecialization : public std::false_type {};

    template <template <typename...> class U, typename... T>
    struct isSpecialization<U, U<T...>> : public std::true_type {};

    template <template <typename, auto...> class Type, typename>
    struct IsSpecializationTypeAndNonTypes : std::false_type {};

    template <template <typename, auto...> class Type, typename T, auto... N>
    struct IsSpecializationTypeAndNonTypes<Type, Type<T, N...>> : std::true_type {};

    template <template <auto...> class Type, typename>
    struct IsSpecializationNonTypes : std::false_type {};

    template <template <auto...> class Type, auto... N>
    struct IsSpecializationNonTypes<Type, Type<N...>> : std::true_type {};

    namespace Impl {
      template <class Tuple, std::size_t... I>
      constexpr auto makeTupleSubsetImpl(Tuple &&t, std::index_sequence<I...>) {
        return std::make_tuple(std::get<I>(std::forward<Tuple>(t))...);
      }

      template <class Tuple, std::size_t... I>
      constexpr auto subTupleFromIndicesImpl(Tuple &&t, std::index_sequence<I...>) {
        return std::make_tuple(std::get<I>(std::forward<Tuple>(t))...);
      }

      template <typename T, typename... Ts>
      struct uniqueImpl : std::type_identity<T> {};

      template <typename... Ts, typename U, typename... Us>
      struct uniqueImpl<std::tuple<Ts...>, U, Us...>
          : std::conditional_t<(std::is_same_v<U, Ts> || ...), uniqueImpl<std::tuple<Ts...>, Us...>,
                               uniqueImpl<std::tuple<Ts..., U>, Us...>> {};

      template <typename... Ts>
      using unique_tupleImpl = typename uniqueImpl<std::tuple<>, Ts...>::type;

      template <typename T, typename... Types>
      auto makeNestedTupleFlatImpl() {
        constexpr bool isTuple = isSpecialization<std::tuple, T>::value;
        if constexpr (sizeof...(Types) > 0) {
          if constexpr (isTuple)
            return std::tuple_cat(makeNestedTupleFlat(T()), makeNestedTupleFlatImpl<Types...>());
          else
            return std::tuple_cat(std::make_tuple(T()), makeNestedTupleFlatImpl<Types...>());
        } else {
          if constexpr (isTuple)
            return makeNestedTupleFlat(T());
          else
            return std::make_tuple(T());
        }
      }

      template <typename T, typename... Types>
      auto makeNestedTupleFlatAndStoreReferencesImpl(const std::tuple<T, Types...> &tup) {
        constexpr bool isTuple = isSpecialization<std::tuple, std::remove_cvref_t<T>>::value;
        if constexpr (sizeof...(Types) > 0) {
          if constexpr (isTuple)
            return std::tuple_cat(
                makeNestedTupleFlatAndStoreReferencesImpl(std::get<0>(tup)),
                std::apply(
                    [](const T &, const Types &...args) {
                      return makeNestedTupleFlatAndStoreReferencesImpl(std::make_tuple(std::cref(args)...));
                    },
                    tup));
          else
            return std::tuple_cat(
                std::make_tuple(std::cref(std::get<0>(tup))),
                std::apply(
                    [](const T &, const Types &...args) {
                      return makeNestedTupleFlatAndStoreReferencesImpl(std::make_tuple(std::cref(args)...));
                    },
                    tup));
        } else {
          if constexpr (isTuple)
            return makeNestedTupleFlatAndStoreReferencesImpl(std::get<0>(tup));
          else
            return std::make_tuple(std::cref(std::get<0>(tup)));
        }
      }

      template <typename T, typename... Types>
      auto makeNestedTupleFlatAndStoreReferencesNonConstImpl(const std::tuple<T, Types...> &tupconst) {
        auto &tup              = const_cast<std::tuple<T, Types...> &>(tupconst);
        constexpr bool isTuple = isSpecialization<std::tuple, std::remove_cvref_t<T>>::value;
        if constexpr (sizeof...(Types) > 0) {
          if constexpr (isTuple)
            return std::tuple_cat(
                makeNestedTupleFlatAndStoreReferencesNonConstImpl(std::get<0>(tup)),
                std::apply(
                    [](T &, Types &...args) {
                      return makeNestedTupleFlatAndStoreReferencesNonConstImpl(std::make_tuple(std::ref(args)...));
                    },
                    tup));
          else
            return std::tuple_cat(
                std::make_tuple(std::ref(std::get<0>(tup))),
                std::apply(
                    [](T &, Types &...args) {
                      return makeNestedTupleFlatAndStoreReferencesNonConstImpl(std::make_tuple(std::ref(args)...));
                    },
                    tup));
        } else {
          if constexpr (isTuple)
            return makeNestedTupleFlatAndStoreReferencesNonConstImpl(std::get<0>(tup));
          else
            return std::make_tuple(std::ref(std::get<0>(tup)));
        }
      }

    }  // namespace Impl

    template <typename Tuple, typename Predicate>
    constexpr size_t find_if(Tuple &&tuple, Predicate pred) {
      size_t index        = std::tuple_size<std::remove_reference_t<Tuple>>::value;
      size_t currentIndex = 0;
      bool found          = false;

      Dune::Hybrid::forEach(tuple, [&](auto &&value) {
        if (!found && pred(value)) {
          index = currentIndex;
          found = true;
        }
        ++currentIndex;
      });
      return index;
    }

    template <typename Tuple, typename Predicate>
    bool none_of(Tuple &&tuple, Predicate pred) {
      return find_if(tuple, pred) == std::tuple_size<std::decay_t<Tuple>>::value;
    }

    template <typename Tuple, typename Predicate>
    bool any_of(Tuple &&tuple, Predicate pred) {
      return !none_of(tuple, pred);
    }

    template <typename Tuple, typename Predicate>
    auto filter(Tuple &&tuple, Predicate pred) {
      return std::apply(
          [&pred](auto... ts) {
            return std::tuple_cat(std::conditional_t<pred(ts), std::tuple<decltype(ts)>, std::tuple<>>{}...);
          },
          tuple);
    }

    template <typename... Types>
    constexpr auto unique(std::tuple<Types...> &&tuple) {
      return Impl::unique_tupleImpl<Types...>();
    }

    template <typename Tuple, typename Predicate>
    constexpr size_t count_if(Tuple &&tuple, Predicate pred) {
      size_t counter = 0;
      Dune::Hybrid::forEach(tuple, [&](auto &&value) {
        if (pred(value)) ++counter;
      });
      return counter;
    }

    template <template <auto...> class Type, typename Tuple>
    constexpr int findTypeSpecialization() {
      return find_if(std::remove_cvref_t<Tuple>(),
                     []<typename T>(T &&) { return IsSpecializationNonTypes<Type, std::remove_cvref_t<T>>::value; });
    }

    template <template <auto...> class Type, typename Tuple>
    auto getSpecialization(Tuple &&tuple) {
      constexpr int index = findTypeSpecialization<Type, Tuple>();
      return std::get<index>(tuple);
    }

    template <template <auto...> class Type, typename Tuple>
    constexpr bool hasTypeSpecialization() {
      return (find_if(std::remove_cvref_t<Tuple>(),
                      []<typename T>(T &&) { return IsSpecializationNonTypes<Type, std::remove_cvref_t<T>>::value; })
              < std::tuple_size_v<std::remove_cvref_t<Tuple>>);
    }

    template <typename Tuple, template <auto...> class Type>
    constexpr bool countTypeSpecialization() {
      return count_if(Tuple(),
                      []<typename T>(T &&) { return IsSpecializationNonTypes<Type, std::remove_cvref_t<T>>::value; });
    }

    template <typename Tuple, template <auto...> class Type>
    static constexpr bool countTypeSpecialization_v = countTypeSpecialization<Tuple, Type>();

    //  template<int N, class Tuple>
    //  constexpr auto makeTupleSubset(Tuple &&t) {
    //    static_assert(N < std::tuple_size_v<std::remove_reference_t<Tuple>>,
    //                  "The requested size needs to be smaller than the size of the tuple.");
    //
    //    return Impl::makeTupleSubsetImpl(std::forward<Tuple>(t), std::make_index_sequence<N>{});
    //  }

    template <class Tuple, std::size_t... I>
    constexpr auto subTupleFromIndices(Tuple &&t) {
      return Impl::subTupleFromIndicesImpl(std::forward<Tuple>(t), std::index_sequence<I...>{});
    }

    template <template <auto...> typename, template <auto...> typename>
    struct isSameTemplate : std::false_type {};

    template <template <auto...> typename TT>
    struct isSameTemplate<TT, TT> : std::true_type {};

    template <template <auto...> typename TT, template <auto...> typename UU>
    inline constexpr bool isSameTemplate_v = isSameTemplate<TT, UU>::value;

    template <typename... Types>
    auto makeNestedTupleFlat(std::tuple<Types...> tup) {
      return decltype(Impl::makeNestedTupleFlatImpl<Types...>())();
    }

    template <typename Tuple>
    auto makeNestedTupleFlatAndStoreReferences(Tuple &&tup) {
      if constexpr (std::tuple_size_v<std::remove_cvref_t<Tuple>> == 0)
        return tup;
      else if constexpr (!std::is_const_v<std::remove_reference_t<Tuple>>)
        return Impl::makeNestedTupleFlatAndStoreReferencesNonConstImpl(std::forward<Tuple>(tup));
      else
        return Impl::makeNestedTupleFlatAndStoreReferencesImpl(std::forward<Tuple>(tup));
    }

    /*
     * Rebind the underlying type of containers
     */
    template <class Container, class NewType>
    struct Rebind;

    /*
     * Specialization for types like std::vector<...> and nested std::vector<std::vector>
     */
    template <class OldType, class... Args, template <class...> class Container, class NewType>
    struct Rebind<Container<OldType, Args...>, NewType> {
      using other = Container<NewType, typename Rebind<Args, NewType>::other...>;
    };

    /*
     * Specialization for types like std::array<...,N>
     */
    template <class OldType, std::size_t N, template <class, std::size_t> class Container, class NewType>
    struct Rebind<Container<OldType, N>, NewType> {
      using other = Container<NewType, N>;
    };

    template <class T1, class T2>
    consteval bool areTypesEqual(T1 &&, T2 &&) {
      return std::is_same_v<T1, T2>;
    }

    template <class T1, class T2>
    consteval bool areTypesEqual() {
      return std::is_same_v<T1, T2>;
    }
  }  // namespace Std

}  // namespace Dune
