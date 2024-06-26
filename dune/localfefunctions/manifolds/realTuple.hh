// SPDX-FileCopyrightText: 2022 The dune-localfefunction developers mueller@ibb.uni-stuttgart.de
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once
#include <concepts>

#include <dune/localfefunctions/linearAlgebraHelper.hh>

#include <Eigen/Core>

namespace Dune {
  /**
   * \brief FunctionReturnType of Euklidean space \f$\mathbb{R}^d\f$
   *
   * \tparam ct The type used for the scalar coordinate values, e.g. double, float
   * \tparam d Dimension of the embedding space of the manifold
   */
  template <typename ct, int d>
  class RealTuple {
  public:
    /** \brief Type used for coordinates */
    using ctype      = ct;
    using field_type = ct;
    using block_type = field_type;

    /** \brief Size of how much values are needed to store the manifold */
    static constexpr int valueSize = d;

    /** \brief Size of how much values are needed to store the correction vector */
    static constexpr int correctionSize = d;

    /** \brief VectorType of the values of the manifold */
    using CoordinateType = DefaultLinearAlgebra::FixedSizedVector<ctype, valueSize>;

    /** \brief VectorType of the values of the correction living in the tangentspace */
    using CorrectionType = DefaultLinearAlgebra::FixedSizedVector<ctype, correctionSize>;

    /** \brief VectorType of the values of the correction living in the tangentspace in the embedding space*/
    using EmbeddedCorrectionType = CoordinateType;

    RealTuple() = default;

    template <typename ctOther, int dOther>
    friend class RealTuple;

    /** \brief Copy assignment if the other type has different underlying type*/
    template <typename ctype_>
      requires std::convertible_to<ctype_, ctype>
    RealTuple<ctype, d> &operator=(const RealTuple<ctype_, d> &other) {
      var = other.var;
      return *this;
    }

    RealTuple<ctype, d> &operator=(const CoordinateType &other) {
      var = other;
      return *this;
    }

    template <typename OtherType>
    struct rebind {
      using other = RealTuple<OtherType, valueSize>;
    };

    /** \brief Compute an orthonormal basis of the tangent space of R^n.
     * This is simply the identity matrix  */
    auto orthonormalFrame() const { return createScaledIdentityMatrix<ctype, valueSize, correctionSize>(); }

    /** \brief Copy-Constructor from the values in terms of coordinateType */
    explicit RealTuple(const CoordinateType &vec) noexcept : var{vec} {}

    /** \brief Move-Constructor from the values in terms of coordinateType */
    explicit RealTuple(CoordinateType &&vec) noexcept : var{std::move(vec)} {}

    /** \brief Get value of the manifold coordinates */
    CoordinateType getValue() const { return var; }

    /** \brief Set the coordinates of the manifold by const reference */
    void setValue(const CoordinateType &vec) { var = vec; }

    /** \brief Set the coordinates of the manifold by r_value reference */
    void setValue(CoordinateType &&vec) noexcept { var = std::move(vec); }

    /** \brief Update the manifold by an correction vector of size correctionSize */
    void update(const CorrectionType &correction) noexcept { var += correction; }

    /** \brief Update the manifold by an correction vector of size correctionSize */
    CoordinateType projectOntoNormalSpace(const CoordinateType &val) const noexcept { return val; }

    /** \brief Access to data by const reference */
    const ctype &operator[](int i) const { return var[i]; }

    /** \brief Access to data by const reference */
    ctype &operator[](int i) { return var[i]; }

    auto &operator+=(const CorrectionType &correction) {
      this->update(correction);
      return *this;
    }

    auto weingartenEmbedded(const CoordinateType &) const { return createZeroMatrix<ctype, valueSize, valueSize>(); }

    auto weingarten(const CoordinateType &) const { return createZeroMatrix<ctype, valueSize, valueSize>(); }

    void addInEmbedding(const CoordinateType &correction) { var += correction; }

    /** \brief size */
    [[nodiscard]] constexpr size_t size() const { return valueSize; }
    auto begin() { return var.begin(); }
    auto end() { return var.end(); }

    auto begin() const { return var.begin(); }
    auto end() const { return var.end(); }

    template <typename Scalar>
      requires std::is_arithmetic_v<Scalar>
    RealTuple &operator*=(const Scalar &factor) {
      var *= factor;
      return *this;
    }

    template <typename Scalar>
      requires std::is_arithmetic_v<Scalar>
    RealTuple &operator/=(const Scalar &factor) {
      var /= factor;
      return *this;
    }

    RealTuple &operator+=(const RealTuple &other) {
      var += other.var;
      return *this;
    }

    RealTuple &operator-=(const RealTuple &other) {
      var -= other.var;
      return *this;
    }

  private:
    CoordinateType var{createZeroVector<ctype, valueSize>()};
  };

  template <typename ctype2, int d2>
  std::ostream &operator<<(std::ostream &s, const RealTuple<ctype2, d2> &var2) {
    s << var2.getValue();
    return s;
  }

  template <typename ctype2, int d2, typename CorrectionType>
  [[nodiscard]] RealTuple<ctype2, d2> operator+(const RealTuple<ctype2, d2> &rt, const CorrectionType &correction) {
    if constexpr (std::is_same_v<RealTuple<ctype2, d2>, CorrectionType>)
      return RealTuple<ctype2, d2>(rt.getValue() + correction.getValue());
    else
      return RealTuple<ctype2, d2>(rt.getValue() + correction);
  }

  template <typename ctype2, int d2>
  [[nodiscard]] RealTuple<ctype2, d2> operator-(const RealTuple<ctype2, d2> &rt) {
    return RealTuple<ctype2, d2>(-rt.getValue());
  }

  template <typename ctype2, int d2, typename Scalar>
    requires std::is_arithmetic_v<Scalar>
  [[nodiscard]] RealTuple<ctype2, d2> operator*(const RealTuple<ctype2, d2> &rt, const Scalar &factor) {
    return RealTuple<ctype2, d2>(rt.getValue() * factor);
  }

  template <typename ctype2, int d2, typename Scalar>
    requires std::is_arithmetic_v<Scalar>
  [[nodiscard]] RealTuple<ctype2, d2> operator*(const Scalar &factor, const RealTuple<ctype2, d2> &rt) {
    return rt * factor;
  }

  template <typename ctype2, int d2>
  bool operator==(const RealTuple<ctype2, d2> &v1, const RealTuple<ctype2, d2> &v2) {
    return v1.getValue() == v2.getValue();
  }

  template <typename ScalarType, int d>
  struct FieldTraits<RealTuple<ScalarType, d>> {
    using field_type = ScalarType;
    using real_type  = ScalarType;
  };

}  // namespace Dune

#include <dune/python/common/fvecmatregistry.hh>

namespace Dune {
  namespace Python {
    template <class K, int size>
    struct registerFieldVecMat<Dune::RealTuple<K, size>> {
      static void apply() {}
    };
    namespace detail {
      template <class K, int n>
      inline static void copy(const char *ptr, const ssize_t *shape, const ssize_t *strides, Dune::RealTuple<K, n> &v) {
        if (*shape != static_cast<ssize_t>(n))
          throw pybind11::value_error("Invalid buffer size: " + std::to_string(*shape)
                                      + " (should be: " + std::to_string(n) + ").");

        for (ssize_t i = 0; i < static_cast<ssize_t>(n); ++i)
          v[i] = *reinterpret_cast<const K *>(ptr + i * (*strides));
      }
    }  // namespace detail
  }    // namespace Python
}  // namespace Dune
