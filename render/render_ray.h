/*
 * LSGL - Large Scale Graphics Library
 *
 * Copyright (c) 2013 - 2015 Advanced Institute for Computational Science,
 *RIKEN.
 * All rights reserved.
 *
 */

#ifndef __LSGL_RENDER_RAY_H__
#define __LSGL_RENDER_RAY_H__

#include <limits>
#include <cmath>
#include <cstdio>
#ifdef _MSC_VER
#include "stdint.h" // compat/stdint.h
#else
#include <stdint.h>
#endif

#include "render_common.h"
#include "render_simd_util.h"

namespace lsgl {
namespace render {

namespace {

#if defined(__SSE2__)
inline float sse_fastinv(float x) {
  __m128 a = _mm_set_ss(x);
  __m128 rcp = _mm_rcp_ss(a);
  __m128 vret =
      _mm_sub_ss(_mm_add_ss(rcp, rcp), _mm_mul_ss(_mm_mul_ss(rcp, rcp), a));
  float ret;
  _mm_store_ss(&ret, vret);
  return ret;
}
#else
inline float sse_fastinv(float x) { return 1.0f / x; }
#endif
}

class Ray;

class Ray {
public:

  inline static float safe_invert(float x) {
#if 0 // This will produce some black pixels in the rendered image.
    static const float EPSILON = std::numeric_limits<float>::epsilon() * 1000;
    if (std::abs(x) < EPSILON) {
      return std::numeric_limits<float>::max();
    } else {
      //return float(1)/x;
      return sse_fastinv(x);
    }
#else
    if (fabs(x) < 1.0e-6f) {
      if (x < 0.0f) {
        return -FLT_MAX;
      } else {
        return FLT_MAX;
      }
    } else {
      return sse_fastinv(x);
    }
#endif
  }
  inline static real3 safe_invert3(const real3 &v) {
    return real3(safe_invert(v[0]), safe_invert(v[1]), safe_invert(v[2]));
  }

public:
  Ray(double org[3], double dir[3]) {
    org_[0] = org[0];
    org_[1] = org[1];
    org_[2] = org[2];

    dir_[0] = dir[0];
    dir_[1] = dir[1];
    dir_[2] = dir[2];

    idir_ = safe_invert3(dir_);

    double_sided = 1; // default: Do double-sided intersection

    prev_prim_id = (unsigned int)(-1);
    prev_hit_t = 0.0;
    prev_hit_normal[0] = 0.0;
    prev_hit_normal[1] = 0.0;
    prev_hit_normal[2] = 0.0;
    prev_node = NULL;
  }

  Ray(const Ray &rhs)
      : org_(rhs.org_), dir_(rhs.dir_), idir_(rhs.idir_),
        double_sided(rhs.double_sided), prev_prim_id(rhs.prev_prim_id),
        prev_node(rhs.prev_node), prev_hit_t(rhs.prev_hit_t), prev_hit_normal(rhs.prev_hit_normal) {
  }

  Ray(const real3 &org, const real3 &dir)
      : org_(org), dir_(dir), idir_(safe_invert3(dir)),
        double_sided(1),
        prev_prim_id((unsigned int)(-1)), prev_node(NULL), prev_hit_t(0.0) {}
  Ray(const real3 &org, const real3 &dir, const real3 &idir)
      : org_(org), dir_(dir), idir_(idir),
        double_sided(1), prev_prim_id((unsigned int)(-1)), prev_node(NULL), prev_hit_t(0.0) {}

  Ray &operator=(const Ray &rhs) {
    org_ = rhs.org_;
    dir_ = rhs.dir_;
    idir_ = rhs.idir_;
    double_sided = rhs.double_sided;
    return *this;
  }

  // real3& origin()           {return org_;}
  const real3 &origin() const { return org_; }

  // real3& direction()           {return dir_;}
  const real3 &direction() const { return dir_; }

  // real3& inversed_direction()           {return idir_;}
  const real3 &inversed_direction() const { return idir_; }

  // Pixel position
  float px;
  float py;

  // trace depth
  int depth;

  // user attribute
  float user_attrib;

  // double-sided isect parameter.
  int double_sided;

  // Variables to avoid self-intersection.
  unsigned int prev_prim_id;
  const unsigned char *prev_node;
  real3 prev_hit_normal;
  double prev_hit_t;

protected:
  real3 org_;
  real3 dir_;
  real3 idir_;
};

} // namespace render
} // namespace lsgl

#endif // __LSGL_RENDER_RAY_H__
