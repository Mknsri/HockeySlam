#ifndef GAME_MATH_H
#define GAME_MATH_H

#include <assert.h>

#define M_PI 3.14159265358979323846 // pi
#define M_2PI M_PI * 2              // pi
float EPSILON = 0.0001f;            // 1e-4

union quat
{
  struct
  {
    float W;
    float X;
    float Y;
    float Z;
  };
  float E[4];
};

union v3
{
  struct
  {
    float X;
    float Y;
    float Z;
  };
  struct
  {
    float R;
    float G;
    float B;
  };
  float E[3];
};

union v4
{
  struct
  {
    float X;
    float Y;
    float Z;
    float W;
  };
  struct
  {
    float R;
    float G;
    float B;
    float A;
  };
  float E[4];
  struct
  {
    v3 XYZ;
    float _discard;
  };
};

union v2
{
  struct
  {
    float X;
    float Y;
  };
  struct
  {
    float U;
    float V;
  };
  float E[2];
};

union mat4x4
{
  struct
  {
    v4 A;
    v4 B;
    v4 C;
    v4 D;
  };
  float S[16];
  float M[4][4];
};

static mat4x4 IDENTITY_MATRIX = {
  1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
};
static mat4x4 MAT4_ZERO = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const v2 V2_ZERO = { { 0.0f } };
static const v2 V2_UP = { { 0.0f, 1.0f } };

static const quat QUAT_ZERO = { 0.0f, 0.0f, 0.0f, 0.0f };
static const quat IDENTITY_ROTATION = { 1.0f, 0.0f, 0.0f, 0.0f };

static const v3 VECTOR_UP = { { 0, 1, 0 } };
static const v3 VECTOR_RIGHT = { { 1, 0, 0 } };
static const v3 VECTOR_LEFT = { { -1, 0, 0 } };
static const v3 VECTOR_FORWARD = { { 0, 0, 1 } };
static const v3 VECTOR_BACKWARD = { { 0, 0, -1 } };
static const v3 V3_ZERO = { { 0.0f } };

static inline float scale(const float value, const float min, const float max)
{
  return (value - min) / (max - min);
}

static inline float clamp(const float value, const float min, const float max)
{
  if (value < min) {
    return min;
  } else if (value > max) {
    return max;
  }

  return value;
}

static inline float sign(const float& value)
{
  return value >= 0.0f ? 1.0f : -1.0f;
}

static inline float deg_to_rad(const float& f)
{
  return f * ((float)M_PI / 180.0f);
}

static inline float rad_to_deg(const float& f)
{
  return f * (180.0f / (float)M_PI);
}

static inline float normalize_deg(const float& f)
{
  float result = f;
  while (result > 360.0f) {
    result -= 360.0f;
  }
  while (result < 0.0f) {
    result += 360.0f;
  }
  return result;
}

static inline float min_f(const float a, const float b)
{
  return a < b ? a : b;
}

static inline float max_f(const float a, const float b)
{
  return a > b ? a : b;
}

static inline float mod_f(const float a, const float b)
{
  float r = a;
  while (r > b) {
    r -= b;
  }
  return r;
}

static inline double min_f(const double a, const double b)
{
  return a < b ? a : b;
}

static inline double max_f(const double a, const double b)
{
  return a > b ? a : b;
}

static inline float quantize_f(const float value, const float quantizeSize)
{
  return std::round(value / quantizeSize) * quantizeSize;
}

static inline float abs_f(const float a)
{
  return a >= 0.0 ? a : -a;
}

static inline bool equal_f(const float a,
                           const float b,
                           const float epsilon = EPSILON)
{
  return ((a - b) < epsilon) && ((b - a) < epsilon);
}

static inline v2 _v2(const float& a)
{
  v2 result = {};
  result.X = a;
  result.Y = a;

  return result;
}

static inline v2 _v2(const float& x, const float& y)
{
  v2 result = {};
  result.X = x;
  result.Y = y;

  return result;
}

static inline v2 operator+(const v2& lhs, const v2& rhs)
{
  return _v2(lhs.X + rhs.X, lhs.Y + rhs.Y);
}

static inline v2 operator-(const v2& lhs, const v2& rhs)
{
  return _v2(lhs.X - rhs.X, lhs.Y - rhs.Y);
}

static inline v2 operator-(const v2& value)
{
  return _v2(-value.X, -value.Y);
}

static inline v2 operator+=(v2& lhs, const v2& rhs)
{
  return lhs = _v2(lhs.X + rhs.X, lhs.Y + rhs.Y);
}

static inline v2 operator-=(v2& lhs, const v2& rhs)
{
  return lhs = _v2(lhs.X - rhs.X, lhs.Y - rhs.Y);
}

static inline v2 operator*(const v2& lhs, const float& rhs)
{
  return _v2(lhs.X * rhs, lhs.Y * rhs);
}

static inline v2 operator*(const float& lhs, const v2& rhs)
{
  return _v2(lhs * rhs.X, lhs * rhs.Y);
}

static inline float length(const v2& lhs)
{
  return sqrt(lhs.X * lhs.X + lhs.Y * lhs.Y);
}

static inline v2 hadamard_multiply(const v2& lhs, const v2& rhs)
{
  return _v2(lhs.X * rhs.X, lhs.Y * rhs.Y);
}

static inline v2 hadamard_division(const v2& lhs, const v2& rhs)
{
  return _v2(lhs.X / rhs.X, lhs.Y / rhs.Y);
}

static inline float dot(const v2& lhs, const v2& rhs)
{
  return (lhs.X * rhs.X) + (lhs.Y * rhs.Y);
}

static inline v2 normalize(const v2& v)
{
  float len = length(v);
  return _v2((v.X != 0 ? v.X / len : 0), (v.Y != 0 ? v.Y / len : 0));
}

static inline v3 _v3(const float& a)
{
  v3 result = {};
  result.X = a;
  result.Y = a;
  result.Z = a;

  return result;
}

static inline v3 _v3(const float& x, const float& y, const float& z)
{
  v3 result = {};
  result.X = x;
  result.Y = y;
  result.Z = z;

  return result;
}

static inline v3 _v3(const v2& v, const float& z)
{
  v3 result = {};
  result.X = v.X;
  result.Y = v.Y;
  result.Z = z;

  return result;
}

static inline v3 _v3(const v4& v)
{
  v3 result = {};
  result.X = v.X;
  result.Y = v.Y;
  result.Z = v.Z;

  return result;
}

static inline v3 v3_deg_to_rad(const v3& v)
{
  v3 result = {};
  for (int i = 0; i < 3; i++) {
    result.E[i] = deg_to_rad(v.E[i]);
  }

  return result;
}

static inline v3 v3_rad_to_deg(const v3& v)
{
  v3 result = {};
  for (int i = 0; i < 3; i++) {
    result.E[i] = rad_to_deg(v.E[i]);
  }

  return result;
}

static inline float length(const v3& v)
{
  return sqrtf((v.X * v.X) + (v.Y * v.Y) + (v.Z * v.Z));
}

static inline v3 normalize(const v3& v)
{
  float len = length(v);
  return _v3((v.X != 0 ? v.X / len : 0),
             (v.Y != 0 ? v.Y / len : 0),
             (v.Z != 0 ? v.Z / len : 0));
}

static inline v3 operator+(const v3& lhs, const float& rhs)
{
  return _v3(lhs.X + rhs, lhs.Y + rhs, lhs.Z + rhs);
}

static inline v3 operator-(const v3& lhs, const float& rhs)
{
  return _v3(lhs.X - rhs, lhs.Y - rhs, lhs.Z - rhs);
}

static inline v3 operator+(const v3& lhs, const v3& rhs)
{
  return _v3(lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z);
}

static inline v3 operator-(const v3& lhs, const v3& rhs)
{
  return _v3(lhs.X - rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z);
}

static inline v3 operator+=(v3& lhs, const v3& rhs)
{
  return lhs = lhs + rhs;
}

static inline v3 operator-=(v3& lhs, const v3& rhs)
{
  return lhs = lhs - rhs;
}

static inline v3 operator-(const v3& value)
{
  return _v3(-value.X, -value.Y, -value.Z);
}

static inline bool operator==(const v3& lhs, const v3& rhs)
{
  return lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Z == rhs.Z;
}

static inline bool operator!=(const v3& lhs, const v3& rhs)
{
  return !(lhs == rhs);
}

static inline v3 operator*(const v3& lhs, const float& rhs)
{
  return _v3(lhs.X * rhs, lhs.Y * rhs, lhs.Z * rhs);
}

static inline v3 operator*(const float& lhs, const v3& rhs)
{
  return _v3(lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z);
}

static inline v3 operator/(const v3& lhs, const float& rhs)
{
  return _v3(lhs.X / rhs, lhs.Y / rhs, lhs.Z / rhs);
}

static inline v3 operator/(const float& lhs, const v3& rhs)
{
  return _v3(lhs / rhs.X, lhs / rhs.Y, lhs / rhs.Z);
}

static inline v3 operator/(const v3& lhs, const v3& rhs)
{
  return _v3(lhs.X / rhs.X, lhs.Y / rhs.Y, lhs.Z / rhs.Z);
}

static inline float dot(const v3& lhs, const v3& rhs)
{
  return (lhs.X * rhs.X) + (lhs.Y * rhs.Y) + (lhs.Z * rhs.Z);
}

static inline v3 reflect(const v3& v, const v3& normal)
{
  return v - 2.0f * dot(v, normal) * normal;
}

static inline v3 cross(const v3& lhs, const v3& rhs)
{
  return _v3((lhs.Y * rhs.Z) - (lhs.Z * rhs.Y),
             (lhs.Z * rhs.X) - (lhs.X * rhs.Z),
             (lhs.X * rhs.Y) - (lhs.Y * rhs.X));
}

static inline v3 hadamard_multiply(const v3& lhs, const v3& rhs)
{
  return _v3(lhs.X * rhs.X, lhs.Y * rhs.Y, lhs.Z * rhs.Z);
}

static inline v3 hadamard_division(const v3& lhs, const v3& rhs)
{
  return _v3(lhs.X / rhs.X, lhs.Y / rhs.Y, lhs.Z / rhs.Z);
}

static inline v3 lerp(const v3& lhs, const v3& rhs, const float& t)
{
  const float complement = (1.0f - t);
  v3 A = lhs * complement;
  v3 B = rhs * t;

  return A + B;
}

static inline v4 _v4(const float& x,
                     const float& y,
                     const float& z,
                     const float& w)
{
  v4 result = {};
  result.X = x;
  result.Y = y;
  result.Z = z;
  result.W = w;

  return result;
}

static inline v4 _v4(const float& v)
{
  v4 result = {};
  result.X = v;
  result.Y = v;
  result.Z = v;
  result.W = v;

  return result;
}

static inline v4 _v4(const v3& v, const float& v2)
{
  v4 result = {};
  result.X = v.X;
  result.Y = v.Y;
  result.Z = v.Z;
  result.W = v2;

  return result;
}
static inline v4 operator+(const v4& lhs, const float& rhs)
{
  return _v4(lhs.X + rhs, lhs.Y + rhs, lhs.Z + rhs, lhs.W + rhs);
}

static inline v4 operator-(const v4& lhs, const float& rhs)
{
  return _v4(lhs.X - rhs, lhs.Y - rhs, lhs.Z - rhs, lhs.W - rhs);
}

static inline v4 operator+(const v4& lhs, const v4& rhs)
{
  return _v4(lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z, lhs.W + rhs.W);
}

static inline v4 operator-(const v4& lhs, const v4& rhs)
{
  return _v4(lhs.X - rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z, lhs.W - rhs.W);
}

static inline v4 operator+=(v4& lhs, const v4& rhs)
{
  return lhs = lhs + rhs;
}

static inline v4 operator-=(v4& lhs, const v4& rhs)
{
  return lhs = lhs - rhs;
}

static inline v4 operator-(const v4& value)
{
  return _v4(-value.X, -value.Y, -value.Z, -value.W);
}

static inline bool operator==(const v4& lhs, const v4& rhs)
{
  return lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Z == rhs.Z && lhs.W == rhs.W;
}

static inline bool operator!=(const v4& lhs, const v4& rhs)
{
  return !(lhs == rhs);
}

static inline v4 operator*(const v4& lhs, const float& rhs)
{
  return _v4(lhs.X * rhs, lhs.Y * rhs, lhs.Z * rhs, lhs.W * rhs);
}

static inline v4 operator*(const float& lhs, const v4& rhs)
{
  return _v4(lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z, lhs * rhs.W);
}

static inline float dot(const v4& lhs, const v4& rhs)
{
  return (lhs.X * rhs.X) + (lhs.Y * rhs.Y) + (lhs.Z * rhs.Z) + (lhs.W * rhs.W);
}

static inline v4 operator*(const v4& lhs, const mat4x4& rhs)
{
  v4 result = {};

  result.X = dot(lhs, rhs.A);
  result.Y = dot(lhs, rhs.B);
  result.Z = dot(lhs, rhs.C);
  result.W = dot(lhs, rhs.D);

  return result;
}

static inline v4 operator*(const mat4x4& lhs, const v4& rhs)
{
  v4 result = {};

  result.X = (lhs.A.X * rhs.X) + (lhs.B.X * rhs.Y) + (lhs.C.X * rhs.Z) +
             (lhs.D.X * rhs.W);
  result.Y = (lhs.A.Y * rhs.X) + (lhs.B.Y * rhs.Y) + (lhs.C.Y * rhs.Z) +
             (lhs.D.Y * rhs.W);
  result.Z = (lhs.A.Z * rhs.X) + (lhs.B.Z * rhs.Y) + (lhs.C.Z * rhs.Z) +
             (lhs.D.Z * rhs.W);
  result.W = (lhs.A.W * rhs.X) + (lhs.B.W * rhs.Y) + (lhs.C.W * rhs.Z) +
             (lhs.D.W * rhs.W);

  return result;
}

static v4& operator*=(v4& lhs, const mat4x4& rhs)
{
  return lhs = lhs * rhs;
}

static inline v4 operator/(const v4& lhs, const float& rhs)
{
  v4 result = {};

  result.X = lhs.X / rhs;
  result.Y = lhs.Y / rhs;
  result.Z = lhs.Z / rhs;
  result.W = lhs.W / rhs;

  return result;
}

static v4& operator/=(v4& lhs, const float& rhs)
{
  return lhs = lhs / rhs;
}

static inline float length(const v4& v)
{
  return sqrtf((v.X * v.X) + (v.Y * v.Y) + (v.Z * v.Z) + (v.W * v.W));
}

static inline v4 hadamard_multiply(const v4& lhs, const v4& rhs)
{
  return _v4(lhs.X * rhs.X, lhs.Y * rhs.Y, lhs.Z * rhs.Z, lhs.W * rhs.W);
}

static inline v4 hadamard_division(const v4& lhs, const v4& rhs)
{
  return _v4(lhs.X / rhs.X, lhs.Y / rhs.Y, lhs.Z / rhs.Z, lhs.W / rhs.W);
}

static inline v4 lerp(const v4& lhs, const v4& rhs, const float& t)
{
  const float complement = (1.0f - t);
  v4 A = lhs * complement;
  v4 B = rhs * t;

  return A + B;
}

static inline quat _quat(const float& w,
                         const float& x,
                         const float& y,
                         const float& z)
{
  quat result = {};
  result.W = w;
  result.X = x;
  result.Y = y;
  result.Z = z;

  return result;
}

static inline quat quat_from_euler(const v3& v)
{
  quat result = {};

  v3 vRad = v3_deg_to_rad(v);

  double c1 = cos(vRad.X * 0.5);
  double c2 = cos(vRad.Y * 0.5);
  double c3 = cos(vRad.Z * 0.5);
  double s1 = sin(vRad.X * 0.5);
  double s2 = sin(vRad.Y * 0.5);
  double s3 = sin(vRad.Z * 0.5);

  result.W = (float)(c1 * c2 * c3 - s1 * s2 * s3);
  result.X = (float)(s1 * c2 * c3 + c1 * s2 * s3);
  result.Y = (float)(c1 * s2 * c3 - s1 * c2 * s3);
  result.Z = (float)(s1 * s2 * c3 + c1 * c2 * s3);

  return result;
}

static inline quat quat_from_euler(const float& x,
                                   const float& y,
                                   const float& z)
{
  return quat_from_euler(_v3(x, y, z));
}

static inline mat4x4 quat_to_mat4x4(const quat& q)
{
  // Normalized
  float len =
    1.0f / (float)sqrt((q.W * q.W) + (q.X * q.X) + (q.Y * q.Y) + (q.Z * q.Z));
  quat n;
  n.W = q.W * len;
  n.X = q.X * len;
  n.Y = q.Y * len;
  n.Z = q.Z * len;

  return { (1.0f - 2.0f * n.Y * n.Y - 2.0f * n.Z * n.Z),
           (2.0f * n.X * n.Y + 2.0f * n.Z * n.W),
           (2.0f * n.X * n.Z - 2.0f * n.Y * n.W),
           (0.0f),
           (2.0f * n.X * n.Y - 2.0f * n.Z * n.W),
           (1.0f - 2.0f * n.X * n.X - 2.0f * n.Z * n.Z),
           (2.0f * n.Y * n.Z + 2.0f * n.X * n.W),
           (0.0f),
           (2.0f * n.X * n.Z + 2.0f * n.Y * n.W),
           (2.0f * n.Y * n.Z - 2.0f * n.X * n.W),
           (1.0f - 2.0f * n.X * n.X - 2.0f * n.Y * n.Y),
           (0.0f),
           (0.0f),
           (0.0f),
           (0.0f),
           (1.0f) };
}

// https://en.wikipedia.org/wiki/Slerp#Quaternion_Slerp
static quat slerp(const quat& v0, const quat& v1, const float& t)
{
  float v0Len =
    sqrtf((v0.W * v0.W) + (v0.X * v0.X) + (v0.Y * v0.Y) + (v0.Z * v0.Z));
  float v1Len =
    sqrtf((v1.W * v1.W) + (v1.X * v1.X) + (v1.Y * v1.Y) + (v1.Z * v1.Z));
  quat A = _quat((v0.W != 0 ? v0.W / v0Len : 0),
                 (v0.X != 0 ? v0.X / v0Len : 0),
                 (v0.Y != 0 ? v0.Y / v0Len : 0),
                 (v0.Z != 0 ? v0.Z / v0Len : 0));

  quat B = _quat((v1.W != 0 ? v1.W / v1Len : 0),
                 (v1.X != 0 ? v1.X / v1Len : 0),
                 (v1.Y != 0 ? v1.Y / v1Len : 0),
                 (v1.Z != 0 ? v1.Z / v1Len : 0));

  // Compute the cosine of the angle between the two vectors.
  float dot = (A.W * B.W) + (A.X * B.X) + (A.Y * B.Y) + (A.Z * B.Z);

  if (dot < 0.0f) {
    A = _quat(-A.W, -A.X, -A.Y, -A.Z);
    dot = -dot;
  }

  const float DOT_THRESHOLD = 1 - 1e-6f;
  if (dot > DOT_THRESHOLD) {
    A = _quat(A.W * t, A.X * t, A.Y * t, A.Z * t);
    B = _quat(
      (1.0f - t) * B.W, (1.0f - t) * B.X, (1.0f - t) * B.Y, (1.0f - t) * B.Z);

    quat C = _quat(A.W + B.W, A.X + B.X, A.Y + B.Y, A.Z + B.Z);

    float cLen = sqrtf((C.W * C.W) + (C.X * C.X) + (C.Y * C.Y) + (C.Z * C.Z));
    C = _quat((C.W != 0 ? C.W / cLen : 0),
              (C.X != 0 ? C.X / cLen : 0),
              (C.Y != 0 ? C.Y / cLen : 0),
              (C.Z != 0 ? C.Z / cLen : 0));

    return C;
  }

  // Since dot is in range [0, DOT_THRESHOLD], acos is safe
  float theta_0 = (float)acos(dot);    // theta_0 = angle between input vectors
  float theta = theta_0 * t;           // theta = angle between v0 and result
  float sin_theta = (float)sin(theta); // compute this value only once
  float sin_theta_0 = (float)sin(theta_0); // compute this value only once

  float s0 =
    (float)cos(theta) -
    dot * sin_theta / sin_theta_0; // == sin(theta_0 - theta) / sin(theta_0)
  float s1 = sin_theta / sin_theta_0;

  return _quat((s0 * A.W) + (s1 * B.W),
               (s0 * A.X) + (s1 * B.X),
               (s0 * A.Y) + (s1 * B.Y),
               (s0 * A.Z) + (s1 * B.Z));
}

static inline quat operator+(const quat& lhs, const quat& rhs)
{
  return _quat(lhs.W + rhs.W, lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z);
}

static inline quat operator+=(quat& lhs, const quat& rhs)
{
  return lhs = lhs + rhs;
}

static inline quat operator*(const quat& lhs, const float& rhs)
{
  return _quat(lhs.W * rhs, lhs.X * rhs, lhs.Y * rhs, lhs.Z * rhs);
}

static inline quat operator*(const float& lhs, const quat& rhs)
{
  return _quat(lhs * rhs.W, lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z);
}

static inline float length(const quat& v)
{
  return sqrtf((v.X * v.X) + (v.Y * v.Y) + (v.Z * v.Z) + (v.W * v.W));
}

static inline quat lerp(const quat& lhs, const quat& rhs, const float& t)
{
  const float complement = (1.0f - t);
  quat A = lhs * complement;
  quat B = rhs * t;

  return A + B;
}

static inline mat4x4 operator+(const mat4x4& lhs, const mat4x4& rhs)
{
  mat4x4 result = {};

  result.A = lhs.A + rhs.A;
  result.B = lhs.B + rhs.B;
  result.C = lhs.C + rhs.C;
  result.D = lhs.D + rhs.D;

  return result;
}

static mat4x4& operator+=(mat4x4& lhs, const mat4x4& rhs)
{
  return lhs = lhs + rhs;
}

static inline mat4x4 operator*(const mat4x4& lhs, const float& rhs)
{
  mat4x4 result = {};

  result.A = lhs.A * rhs;
  result.B = lhs.B * rhs;
  result.C = lhs.C * rhs;
  result.D = lhs.D * rhs;
  result.S[15] = 1.0f;

  return result;
}

static mat4x4& operator*=(mat4x4& lhs, const float& rhs)
{
  return lhs = lhs * rhs;
}

static inline mat4x4 operator*(const mat4x4& lhs, const mat4x4& rhs)
{
  mat4x4 result = {};

  // for (int j = 0; j < 4; j++) {
  //  for (int i = 0; i < 4; i++) {
  //    for (int k = 0; k < 4; k++) {
  //      result.M[i][j] += (lhs.M[k][j] * rhs.M[i][k]);
  //    }
  //  }
  //}

  for (unsigned int i = 0; i < 16; i++) {
    unsigned int row = (int)i / 4, col = i % 4;
    result.M[col][row] =
      lhs.M[0][row] * rhs.M[col][0] + lhs.M[1][row] * rhs.M[col][1] +
      lhs.M[2][row] * rhs.M[col][2] + lhs.M[3][row] * rhs.M[col][3];
  }

  return result;
}

static mat4x4& operator*=(mat4x4& lhs, const mat4x4& rhs)
{
  return lhs = lhs * rhs;
}

static inline mat4x4 mat4x4_translate(const v3& v)
{
  mat4x4 result = IDENTITY_MATRIX;

  result.M[3][0] = v.X;
  result.M[3][1] = v.Y;
  result.M[3][2] = v.Z;

  return result;
}

static inline mat4x4 mat4x4_scale(const v3& v)
{
  mat4x4 result = IDENTITY_MATRIX;

  result.M[0][0] = v.X;
  result.M[1][1] = v.Y;
  result.M[2][2] = v.Z;

  return result;
}

static inline mat4x4 mat4x4_rotate(const v3& v)
{
  quat q = quat_from_euler(v);
  return quat_to_mat4x4(q);
}

static inline mat4x4 hadamard_multiply(const mat4x4& lhs, const mat4x4& rhs)
{
  mat4x4 result = {};
  result.A = hadamard_multiply(lhs.A, rhs.A);
  result.B = hadamard_multiply(lhs.B, rhs.B);
  result.C = hadamard_multiply(lhs.C, rhs.C);
  result.D = hadamard_multiply(lhs.D, rhs.D);
  return result;
}

static inline mat4x4 hadamard_division(const mat4x4& lhs, const mat4x4& rhs)
{
  mat4x4 result = {};
  result.A = hadamard_division(lhs.A, rhs.A);
  result.B = hadamard_division(lhs.B, rhs.B);
  result.C = hadamard_division(lhs.C, rhs.C);
  result.D = hadamard_division(lhs.D, rhs.D);
  return result;
}

mat4x4 orthographic_matrix(const float& left,
                           const float& right,
                           const float& top,
                           const float& bottom,
                           const float& clipNear,
                           const float& clipFar)
{
  const float rlm = (right - left);
  const float rlp = (right + left);
  const float tbm = (top - bottom);
  const float tbp = (top + bottom);
  const float fnm = (clipFar - clipNear);
  const float fnp = (clipFar + clipNear);

  mat4x4 result = {};
  result.M[0][0] = 2.0f / rlm;
  result.M[1][1] = 2.0f / tbm;
  result.M[2][2] = -2.0f / fnm;
  result.M[3][3] = 1.0f;
  result.M[3][0] = -rlp / rlm;
  result.M[3][1] = -tbp / tbm;
  result.M[3][2] = -fnp / fnm;

  return result;
}

static mat4x4 perspective_matrix(const float& fov,
                                 const float& aspectRatio,
                                 const float& clipNear,
                                 const float& clipFar)
{
  assert(fov > 0);
  assert(aspectRatio > 0);

  float tanHalfFov = (float)tan(deg_to_rad(fov) / 2.0f);
  float xScale = 1.0f / (aspectRatio * tanHalfFov);
  float yScale = 1.0f / tanHalfFov;
  float zRange = clipFar - clipNear;

  mat4x4 result = {};
  result.M[0][0] = xScale;
  result.M[1][1] = yScale;
  result.M[2][2] = -(clipFar + clipNear) / zRange;
  result.M[2][3] = -1.0f;
  result.M[3][2] = -(2.0f * clipFar * clipNear) / zRange;

  return result;
}

static mat4x4 look_at(const v3& cameraPos, const v3& target)
{
  v3 dir = normalize(cameraPos - target);
  v3 cameraRight;
  if (VECTOR_UP == dir) {
    cameraRight = normalize(cross(VECTOR_RIGHT, dir));
  } else {
    cameraRight = normalize(cross(VECTOR_UP, dir));
  }
  v3 cameraUp = cross(dir, cameraRight);

  float rightDot = dot(cameraRight, cameraPos);
  float upDot = dot(cameraUp, cameraPos);
  float dirDot = dot(dir, cameraPos);

  mat4x4 orientation = {};
  orientation.A = { { cameraRight.X, cameraUp.X, dir.X, 0.0f } };
  orientation.B = { { cameraRight.Y, cameraUp.Y, dir.Y, 0.0f } };
  orientation.C = { { cameraRight.Z, cameraUp.Z, dir.Z, 0.0f } };
  orientation.D = { { -rightDot, -upDot, -dirDot, 1.0f } };

  return orientation;
}

static mat4x4 mat4x4_invert(const mat4x4& m)
{
  mat4x4 result = {};

  result.S[0] = m.S[5] * m.S[10] * m.S[15] - m.S[5] * m.S[11] * m.S[14] -
                m.S[9] * m.S[6] * m.S[15] + m.S[9] * m.S[7] * m.S[14] +
                m.S[13] * m.S[6] * m.S[11] - m.S[13] * m.S[7] * m.S[10];

  result.S[4] = -m.S[4] * m.S[10] * m.S[15] + m.S[4] * m.S[11] * m.S[14] +
                m.S[8] * m.S[6] * m.S[15] - m.S[8] * m.S[7] * m.S[14] -
                m.S[12] * m.S[6] * m.S[11] + m.S[12] * m.S[7] * m.S[10];

  result.S[8] = m.S[4] * m.S[9] * m.S[15] - m.S[4] * m.S[11] * m.S[13] -
                m.S[8] * m.S[5] * m.S[15] + m.S[8] * m.S[7] * m.S[13] +
                m.S[12] * m.S[5] * m.S[11] - m.S[12] * m.S[7] * m.S[9];

  result.S[12] = -m.S[4] * m.S[9] * m.S[14] + m.S[4] * m.S[10] * m.S[13] +
                 m.S[8] * m.S[5] * m.S[14] - m.S[8] * m.S[6] * m.S[13] -
                 m.S[12] * m.S[5] * m.S[10] + m.S[12] * m.S[6] * m.S[9];

  result.S[1] = -m.S[1] * m.S[10] * m.S[15] + m.S[1] * m.S[11] * m.S[14] +
                m.S[9] * m.S[2] * m.S[15] - m.S[9] * m.S[3] * m.S[14] -
                m.S[13] * m.S[2] * m.S[11] + m.S[13] * m.S[3] * m.S[10];

  result.S[5] = m.S[0] * m.S[10] * m.S[15] - m.S[0] * m.S[11] * m.S[14] -
                m.S[8] * m.S[2] * m.S[15] + m.S[8] * m.S[3] * m.S[14] +
                m.S[12] * m.S[2] * m.S[11] - m.S[12] * m.S[3] * m.S[10];

  result.S[9] = -m.S[0] * m.S[9] * m.S[15] + m.S[0] * m.S[11] * m.S[13] +
                m.S[8] * m.S[1] * m.S[15] - m.S[8] * m.S[3] * m.S[13] -
                m.S[12] * m.S[1] * m.S[11] + m.S[12] * m.S[3] * m.S[9];

  result.S[13] = m.S[0] * m.S[9] * m.S[14] - m.S[0] * m.S[10] * m.S[13] -
                 m.S[8] * m.S[1] * m.S[14] + m.S[8] * m.S[2] * m.S[13] +
                 m.S[12] * m.S[1] * m.S[10] - m.S[12] * m.S[2] * m.S[9];

  result.S[2] = m.S[1] * m.S[6] * m.S[15] - m.S[1] * m.S[7] * m.S[14] -
                m.S[5] * m.S[2] * m.S[15] + m.S[5] * m.S[3] * m.S[14] +
                m.S[13] * m.S[2] * m.S[7] - m.S[13] * m.S[3] * m.S[6];

  result.S[6] = -m.S[0] * m.S[6] * m.S[15] + m.S[0] * m.S[7] * m.S[14] +
                m.S[4] * m.S[2] * m.S[15] - m.S[4] * m.S[3] * m.S[14] -
                m.S[12] * m.S[2] * m.S[7] + m.S[12] * m.S[3] * m.S[6];

  result.S[10] = m.S[0] * m.S[5] * m.S[15] - m.S[0] * m.S[7] * m.S[13] -
                 m.S[4] * m.S[1] * m.S[15] + m.S[4] * m.S[3] * m.S[13] +
                 m.S[12] * m.S[1] * m.S[7] - m.S[12] * m.S[3] * m.S[5];

  result.S[14] = -m.S[0] * m.S[5] * m.S[14] + m.S[0] * m.S[6] * m.S[13] +
                 m.S[4] * m.S[1] * m.S[14] - m.S[4] * m.S[2] * m.S[13] -
                 m.S[12] * m.S[1] * m.S[6] + m.S[12] * m.S[2] * m.S[5];

  result.S[3] = -m.S[1] * m.S[6] * m.S[11] + m.S[1] * m.S[7] * m.S[10] +
                m.S[5] * m.S[2] * m.S[11] - m.S[5] * m.S[3] * m.S[10] -
                m.S[9] * m.S[2] * m.S[7] + m.S[9] * m.S[3] * m.S[6];

  result.S[7] = m.S[0] * m.S[6] * m.S[11] - m.S[0] * m.S[7] * m.S[10] -
                m.S[4] * m.S[2] * m.S[11] + m.S[4] * m.S[3] * m.S[10] +
                m.S[8] * m.S[2] * m.S[7] - m.S[8] * m.S[3] * m.S[6];

  result.S[11] = -m.S[0] * m.S[5] * m.S[11] + m.S[0] * m.S[7] * m.S[9] +
                 m.S[4] * m.S[1] * m.S[11] - m.S[4] * m.S[3] * m.S[9] -
                 m.S[8] * m.S[1] * m.S[7] + m.S[8] * m.S[3] * m.S[5];

  result.S[15] = m.S[0] * m.S[5] * m.S[10] - m.S[0] * m.S[6] * m.S[9] -
                 m.S[4] * m.S[1] * m.S[10] + m.S[4] * m.S[2] * m.S[9] +
                 m.S[8] * m.S[1] * m.S[6] - m.S[8] * m.S[2] * m.S[5];

  float det;
  det = m.S[0] * result.S[0] + m.S[1] * result.S[4] + m.S[2] * result.S[8] +
        m.S[3] * result.S[12];

  if (det == 0.0f) {
    det = 1.0f;
  }

  det = 1.0f / det;

  for (int i = 0; i < 16; i++) {
    result.S[i] = result.S[i] * det;
  }

  return result;
}

static void get_aabb_for_frustum(const mat4x4 projectionMatrix,
                                 const mat4x4 targetProjection,
                                 v3* min,
                                 v3* max)
{
  mat4x4 projInv = mat4x4_invert(projectionMatrix);
  v4 RTN = projInv * _v4(1.0f, 1.0f, -1.0f, 1.0f);
  RTN = RTN / RTN.W;
  v4 LTN = projInv * _v4(-1.0f, 1.0f, -1.0f, 1.0f);
  LTN = LTN / LTN.W;
  v4 RBN = projInv * _v4(1.0f, -1.0f, -1.0f, 1.0f);
  RBN = RBN / RBN.W;
  v4 LBN = projInv * _v4(-1.0f, -1.0f, -1.0f, 1.0f);
  LBN = LBN / LBN.W;

  v4 RTF = projInv * _v4(1.0f, 1.0f, 1.0f, 1.0f);
  RTF = RTF / RTF.W;
  v4 LTF = projInv * _v4(-1.0f, 1.0f, 1.0f, 1.0f);
  LTF = LTF / LTF.W;
  v4 RBF = projInv * _v4(1.0f, -1.0f, 1.0f, 1.0f);
  RBF = RBF / RBF.W;
  v4 LBF = projInv * _v4(-1.0f, -1.0f, 1.0f, 1.0f);
  LBF = LBF / LBF.W;

  RTN = targetProjection * RTN;
  LTN = targetProjection * LTN;
  RBN = targetProjection * RBN;
  LBN = targetProjection * LBN;
  RTF = targetProjection * RTF;
  LTF = targetProjection * LTF;
  RBF = targetProjection * RBF;
  LBF = targetProjection * LBF;

  min->X =
    min_f(RTN.X,
          min_f(LTN.X,
                min_f(RBN.X,
                      min_f(LBN.X,
                            min_f(RTF.X, min_f(LTF.X, min_f(RBF.X, LBF.X)))))));
  min->Y =
    min_f(RTN.Y,
          min_f(LTN.Y,
                min_f(RBN.Y,
                      min_f(LBN.Y,
                            min_f(RTF.Y, min_f(LTF.Y, min_f(RBF.Y, LBF.Y)))))));
  min->Z =
    min_f(RTN.Z,
          min_f(LTN.Z,
                min_f(RBN.Z,
                      min_f(LBN.Z,
                            min_f(RTF.Z, min_f(LTF.Z, min_f(RBF.Z, LBF.Z)))))));

  max->X =
    max_f(RTN.X,
          max_f(LTN.X,
                max_f(RBN.X,
                      max_f(LBN.X,
                            max_f(RTF.X, max_f(LTF.X, max_f(RBF.X, LBF.X)))))));
  max->Y =
    max_f(RTN.Y,
          max_f(LTN.Y,
                max_f(RBN.Y,
                      max_f(LBN.Y,
                            max_f(RTF.Y, max_f(LTF.Y, max_f(RBF.Y, LBF.Y)))))));
  max->Z =
    max_f(RTN.Z,
          max_f(LTN.Z,
                max_f(RBN.Z,
                      max_f(LBN.Z,
                            max_f(RTF.Z, max_f(LTF.Z, max_f(RBF.Z, LBF.Z)))))));
}

static v3 get_midpoint_for_frustum(const mat4x4 projectionMatrix)
{
  mat4x4 projInv = mat4x4_invert(projectionMatrix);
  v4 farCenter = projInv * _v4(0.0f, 0.0f, 1.0f, 1.0f);
  farCenter = farCenter / farCenter.W;

  v4 nearCenter = projInv * _v4(0.0f, 0.0f, -1.0f, 1.0f);
  nearCenter = nearCenter / nearCenter.W;
  return _v3((nearCenter + farCenter) * 0.5f);
}

#endif // GAME_MATH_H