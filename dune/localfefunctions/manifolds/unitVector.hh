// SPDX-FileCopyrightText: 2022 The dune-localfefunction developers mueller@ibb.uni-stuttgart.de
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once
#include <cmath>
#include <concepts>

#include <dune/localfefunctions/eigenDuneTransformations.hh>
#include <dune/localfefunctions/linearAlgebraHelper.hh>

namespace Dune {
  /**
   * \brief FunctionReturnType of unit vectors \f$\mathcal{S}^{d-1}\f$ embedded into space \f$\mathbb{R}^d\f$
   *
   * \tparam ct The type used for the scalar coordinate values, e.g. double,float
   * \tparam d Dimension of the embedding space of the manifold
   */
  template <typename ct, int d>  // requires (d>1)
  class UnitVector {
  public:
    /** \brief Type used for coordinates */
    using ctype      = ct;
    using field_type = ct;
    using block_type = field_type;

    /** \brief Size of how much values are needed to store the manifold */
    static constexpr int valueSize = d;

    /** \brief Size of how much values are needed to store the correction vector */
    static constexpr int correctionSize = d - 1;

    /** \brief VectorType of the values of the manifold */
    using CoordinateType = DefaultLinearAlgebra::FixedSizedVector<ctype, valueSize>;

    /** \brief VectorType of the values of the correction living in the tangentspace */
    using CorrectionType = DefaultLinearAlgebra::FixedSizedVector<ctype, correctionSize>;

    /** \brief VectorType of the values of the correction living in the tangentspace in the embedding space*/
    using EmbeddedCorrectionType = CoordinateType;

    UnitVector() = default;

    /** \brief Copy-Constructor from the values in terms of coordinateType */
    explicit UnitVector(const CoordinateType &vec) noexcept : var{vec / two_norm(vec)} {}

    /** \brief Move-Constructor from the values in terms of coordinateType */
    explicit UnitVector(CoordinateType &&vec) noexcept : var{vec / two_norm(vec)} {}

    const CoordinateType &getValue() const { return var; }

    void setValue(const CoordinateType &vec) { var = vec / two_norm(vec); }

    /** \brief Set the coordinates of the manifold by r_value reference */
    void setValue(CoordinateType &&vec) { var = vec / two_norm(vec); }

    /** \brief Access to data by const reference */
    const ctype &operator[](int i) const { return var[i]; }

    /** \brief Access to data by const reference */
    ctype &operator[](int i) { return var[i]; }

    /** \brief Update the manifold by an correction vector of size correctionSize */
    CoordinateType projectOntoNormalSpace(const CoordinateType &val) const noexcept { return (var * val) * var; }

    /** \brief size */
    [[nodiscard]] size_t size() const { return var.size(); }

    auto begin() { return var.begin(); }
    auto end() { return var.end(); }

    auto begin() const { return var.begin(); }
    auto end() const { return var.end(); }

    template <typename OtherType>
    struct rebind {
      using other = UnitVector<OtherType, valueSize>;
    };

    /** \brief Update the manifold by an correction vector of size correctionSize
     * For the unit vector in R^3 the correction are of size 2
     * Therefore, we need an basis for the tangent space.
     * This means we have two three dimensional vectors spanning this space.
     * This is done using the function orthonormalFrame which returns a 3x2 Matrix */
    void update(const CorrectionType &correction) {
      var += orthonormalFrame() * correction;
      var = var / two_norm(var);  // projection-based retraction
    }

    static auto derivativeOfProjectionWRTposition(const DefaultLinearAlgebra::FixedSizedVector<ctype, valueSize> &p) {
      const ctype norm                                                  = two_norm(p);
      const DefaultLinearAlgebra::FixedSizedVector<ctype, valueSize> pN = p / norm;

      DefaultLinearAlgebra::FixedSizedMatrix<ctype, valueSize, valueSize> result
          = (createScaledIdentityMatrix<ctype, valueSize, valueSize>() - outer(pN, pN)) / norm;

      return result;
    }

    DefaultLinearAlgebra::FixedSizedMatrix<ctype, valueSize, valueSize> weingartenEmbedded(
        const CoordinateType &p) const {
      return -createScaledIdentityMatrix<ctype, valueSize, valueSize>(inner(var, p));
    }

    DefaultLinearAlgebra::FixedSizedMatrix<ctype, correctionSize, correctionSize> weingarten(
        const CoordinateType &p) const {
      return -createScaledIdentityMatrix<ctype, correctionSize, correctionSize>(inner(var, p));
    }

    static DefaultLinearAlgebra::FixedSizedMatrix<ctype, valueSize, valueSize> secondDerivativeOfProjectionWRTposition(
        const DefaultLinearAlgebra::FixedSizedVector<ctype, valueSize> &p,
        const DefaultLinearAlgebra::FixedSizedVector<ctype, valueSize> &along) {
      const ctype normSquared = two_norm2(p);
      using std::sqrt;
      const ctype norm                                                  = sqrt(normSquared);
      const DefaultLinearAlgebra::FixedSizedVector<ctype, valueSize> pN = p / norm;

      DefaultLinearAlgebra::FixedSizedMatrix<ctype, valueSize, valueSize> Q_along
          = 1 / normSquared
            * (inner(pN, along) * (3 * outer(pN, pN) - createScaledIdentityMatrix<ctype, valueSize, valueSize>())
               - outer(along, pN) - outer(pN, along));

      return Q_along;
    }

    static DefaultLinearAlgebra::FixedSizedMatrix<ctype, valueSize, valueSize> thirdDerivativeOfProjectionWRTposition(
        const DefaultLinearAlgebra::FixedSizedVector<ctype, valueSize> &p,
        const DefaultLinearAlgebra::FixedSizedVector<ctype, valueSize> &along1,
        const DefaultLinearAlgebra::FixedSizedVector<ctype, valueSize> &along2) {
      using FieldMat          = DefaultLinearAlgebra::FixedSizedMatrix<ctype, valueSize, valueSize>;
      const ctype normSquared = two_norm2(p);
      using std::sqrt;
      const ctype norm                                                  = sqrt(normSquared);
      const DefaultLinearAlgebra::FixedSizedVector<ctype, valueSize> pN = p / norm;
      const ctype tscala1                                               = inner(pN, along1);
      const ctype tscalwd1                                              = inner(pN, along2);
      const ctype a1scalwd1                                             = inner(along1, along2);
      const ctype normwcubinv                                           = 1 / (normSquared * norm);
      const FieldMat a1dyadt                                            = outer(along1, pN);
      const FieldMat wd1dyadt                                           = outer(along2, pN);
      const FieldMat tDyadict                                           = outer(pN, pN);
      const FieldMat Id3minus5tdyadt = createScaledIdentityMatrix<ctype, valueSize, valueSize>() - 5.0 * tDyadict;
      FieldMat Chi_along             = normwcubinv
                           * (3.0 * tscalwd1 * (a1dyadt + 0.5 * tscala1 * Id3minus5tdyadt)
                              + 3.0 * (0.5 * a1scalwd1 * tDyadict + tscala1 * wd1dyadt) - outer(along1, along2)
                              - createScaledIdentityMatrix<ctype, valueSize, valueSize>(a1scalwd1 * 0.5));
      Chi_along = Chi_along + transposeEvaluated(Chi_along);
      return Chi_along;
    }

    /** \brief Compute an orthonormal basis of the tangent space of S^n.
     * Taken from Oliver Sander's dune-gfe */
    DefaultLinearAlgebra::FixedSizedMatrix<ctype, valueSize, correctionSize> orthonormalFrame() const {
#if DUNE_LOCALFEFUNCTIONS_USE_EIGEN == 1
      using ResultType = Eigen::Matrix<ctype, valueSize, correctionSize>;
      ResultType result;

      // Coordinates of the stereographic projection
      Eigen::Vector<ctype, correctionSize> X;
      Eigen::Vector<ctype, valueSize> varEigen = toEigenVectorMap(var);

      if (var[correctionSize] <= 0)
        // Stereographic projection from the North Pole onto R^{N-1}
        X = varEigen.template head<correctionSize>() / (1 - var[correctionSize]);
      else
        // Stereographic projection from the South Pole onto R^{N-1}
        X = varEigen.template head<correctionSize>() / (1 + var[correctionSize]);

      result.template topLeftCorner<correctionSize, correctionSize>()
          = (2 * (1 + X.squaredNorm())) * Eigen::Matrix<ctype, correctionSize, correctionSize>::Identity()
            - 4 * X * X.transpose();
      result.template bottomLeftCorner<1, correctionSize>() = 4 * X.transpose();

      // Upper hemisphere: adapt formulas, so it is the stereographic projection from the South Pole
      if (var[correctionSize] > 0) result.template bottomLeftCorner<1, correctionSize>() *= -1;

      // normalize the cols to make the orthogonal basis orthonormal
      result.colwise().normalize();

      return result;
#else
      Dune::FieldMatrix<ctype, valueSize, correctionSize> result;

      // Coordinates of the stereographic projection
      Dune::FieldVector<ctype, correctionSize> X;

      if (var[correctionSize] <= 0) {
        // Stereographic projection from the north pole onto R^{N-1}
        for (size_t i = 0; i < correctionSize; i++)
          X[i] = var[i] / (1 - var[correctionSize]);
      } else {
        // Stereographic projection from the south pole onto R^{N-1}
        for (size_t i = 0; i < correctionSize; i++)
          X[i] = var[i] / (1 + var[correctionSize]);
      }

      ctype RSquared = X.two_norm2();

      for (size_t i = 0; i < correctionSize; i++)
        for (size_t j = 0; j < correctionSize; j++)
          result[i][j] = 2 * (i == j) * (1 + RSquared) - 4 * X[i] * X[j];

      for (size_t j = 0; j < correctionSize; j++)
        result[correctionSize][j] = 4 * X[j];

      // Upper hemisphere: adapt formulas so it is the stereographic projection from the south pole
      if (var[correctionSize] > 0)
        for (size_t j = 0; j < correctionSize; j++)
          result[correctionSize][j] *= -1;

      // normalize the columns to make the orthogonal basis orthonormal
      for (size_t i = 0; i < correctionSize; i++) {
        ctype colLength = 0;
        for (size_t j = 0; j < valueSize; j++)
          colLength += result[j][i] * result[j][i];
        using std::sqrt;
        colLength = sqrt(colLength);
        for (size_t j = 0; j < valueSize; j++)
          result[j][i] /= colLength;
      }

      return result;
#endif
    }

    template <typename ctOther, int dOther>
    friend class UnitVector;

    /** \brief Copy assignment if the other type has different underlying type*/
    template <typename ctype_>
      requires std::convertible_to<ctype_, ctype>
    UnitVector<ctype, d> &operator=(const UnitVector<ctype_, d> &other) {
      var = other.var;
      return *this;
    }

    UnitVector<ctype, d> &operator=(const CoordinateType &other) {
      var = other;
      var /= two_norm(other);
      return *this;
    }

    auto &operator+=(const CorrectionType &correction) {
      this->update(correction);
      return *this;
    }

    void addInEmbedding(const CoordinateType &correction) { var += correction; }

    template <typename Scalar>
      requires std::is_arithmetic_v<Scalar>
    UnitVector &operator*=(const Scalar &factor) {
      DUNE_THROW(MathError,
                 "The += operation does not make sense. It is only here to store unit vectors in Dune::BlockVector");
      return *this;
    }

    template <typename Scalar>
      requires std::is_arithmetic_v<Scalar>
    UnitVector &operator/=(const Scalar &factor) {
      DUNE_THROW(MathError,
                 "The /= operation does not make sense. It is only here to store unit vectors in Dune::BlockVector");
      return *this;
    }

    UnitVector &operator+=(const UnitVector &other) {
      DUNE_THROW(MathError,
                 "The += operation does not make sense. It is only here to store unit vectors in Dune::BlockVector");

      return *this;
    }

    UnitVector &operator-=(const UnitVector &other) {
      DUNE_THROW(MathError,
                 "The -= operation does not make sense. It is only here to store unit vectors in Dune::BlockVector");
      return *this;
    }

  private:
    CoordinateType var{createOnesVector<ctype, valueSize>() / two_norm(createOnesVector<ctype, valueSize>())};
  };

  template <typename ctype2, int d2>
  bool operator==(const UnitVector<ctype2, d2> &v1, const UnitVector<ctype2, d2> &v2) {
    return v1.getValue() == v2.getValue();
  }

  template <typename ctype2, int d2>
  std::ostream &operator<<(std::ostream &s, const UnitVector<ctype2, d2> &var2) {
    s << var2.getValue();
    return s;
  }

  template <typename ctype2, int d2>
  [[nodiscard]] UnitVector<ctype2, d2> update(const UnitVector<ctype2, d2> &rt,
                                              const typename UnitVector<ctype2, d2>::CorrectionType &correction) {
    return UnitVector<ctype2, d2>(rt.getValue() + rt.orthonormalFrame() * correction);
  }

  template <typename ctype2, int d2>
  [[nodiscard]] UnitVector<ctype2, d2> operator+(const UnitVector<ctype2, d2> &rt,
                                                 const typename UnitVector<ctype2, d2>::CorrectionType &correction) {
    return UnitVector<ctype2, d2>(rt.getValue() + rt.orthonormalFrame() * correction);
  }

  template <typename ctype2, int d2>
  class RealTuple;

  template <typename ctype2, int d2, typename Scalar>
    requires std::is_arithmetic_v<Scalar>
  [[nodiscard]] RealTuple<ctype2, d2> operator*(const UnitVector<ctype2, d2> &rt, const Scalar &factor) {
    return RealTuple<ctype2, d2>(rt.getValue() * factor);
  }

  template <typename ctype2, int d2, typename Scalar>
    requires std::is_arithmetic_v<Scalar>
  [[nodiscard]] RealTuple<ctype2, d2> operator*(const Scalar &factor, const UnitVector<ctype2, d2> &rt) {
    return rt * factor;
  }

  template <typename ScalarType, int d>
  struct FieldTraits<UnitVector<ScalarType, d>> {
    using field_type = ScalarType;
    using real_type  = ScalarType;
  };

}  // namespace Dune

#include <dune/python/common/fvecmatregistry.hh>
namespace Dune {
  namespace Python {
    template <class K, int size>
    struct registerFieldVecMat<Dune::UnitVector<K, size>> {
      static void apply() {}
    };

    namespace detail {
      template <class K, int n>
      inline static void copy(const char *ptr, const ssize_t *shape, const ssize_t *strides,
                              Dune::UnitVector<K, n> &v) {
        if (*shape != static_cast<ssize_t>(n))
          throw pybind11::value_error("Invalid buffer size: " + std::to_string(*shape)
                                      + " (should be: " + std::to_string(n) + ").");

        for (ssize_t i = 0; i < static_cast<ssize_t>(n); ++i)
          v[i] = *reinterpret_cast<const K *>(ptr + i * (*strides));
      }
    }  // namespace detail
  }    // namespace Python
}  // namespace Dune
