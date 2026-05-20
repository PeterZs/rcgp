Block {
  Context {
    stage: Subroutine,
    name: sr,
    arguments: { Float },
#ifdef __clang__
    returns: { rcgp::Ray },
#elif defined(__GNUC__)
    returns: { Ray },
#endif
  }
  $0 = Argument 0: Float
  $1 = Local Float
  Store $1 $0
  $2 = Local Float
  $3 = 0
  Store $2 $3
  $4 = New Vec3($2, $2, $2)
  $5 = Local Vec3
  Store $5 $4
  $6 = Local Float
  $7 = 1
  Store $6 $7
  $8 = Local Float
  $9 = 1
  Store $8 $9
#ifdef __clang__
  $10 = New Vec3($6, $1, $8)
#elif defined(__GNUC__)
  $10 = New Vec3($8, $1, $6)
#endif
  $11 = Local Vec3
  Store $11 $10
  $12 = Normalize($11)
  $13 = Local Vec3
  Store $13 $12
  $15 = Return 0: $14
#ifdef __clang__
  $16 = New Vec3($5)
#elif defined(__GNUC__)
  $16 = New Vec3($13)
#endif
  $17 = Local Vec3
  Store $17 $16
  $18 = New Vec3($17)
  $19 = Local Vec3
  Store $19 $18
#ifdef __clang__
  $20 = New Vec3($13)
#elif defined(__GNUC__)
  $20 = New Vec3($5)
#endif
  $21 = Local Vec3
  Store $21 $20
  $22 = New Vec3($21)
  $23 = Local Vec3
  Store $23 $22
#ifdef __clang__
  $24 = New Vec3($23)
#elif defined(__GNUC__)
  $24 = New Vec3($19)
#endif
  $25 = Local Vec3
  Store $25 $24
#ifdef __clang__
  $26 = New Vec3($19)
#elif defined(__GNUC__)
  $26 = New Vec3($23)
#endif
  $27 = Local Vec3
  Store $27 $26
#ifdef __clang__
  $28 = New rcgp::Ray($27, $25)
#elif defined(__GNUC__)
  $28 = New Ray($27, $25)
#endif
  Store $15 $28
}
