Block {
  Context {
    stage: Subroutine,
    name: sr,
    arguments: { Float, UInt32 },
    returns: { Vec3, UVec2 },
  }
  $0 = Argument 0: Float
  $1 = Argument 1: UInt32
#ifdef __clang__
  $2 = Local Float
  Store $2 $0
  $3 = Local UInt32
  Store $3 $1
#elif defined(__GNUC__)
  $2 = Local UInt32
  Store $2 $1
  $3 = Local Float
  Store $3 $0
#endif
  $4 = Local Float
#ifdef __clang__
  Store $4 $2
#elif defined(__GNUC__)
  Store $4 $3
#endif
  $5 = New Vec3($4, $4, $4)
  $6 = Local Vec3
  Store $6 $5
  $7 = Local UInt32
  $8 = 13
  Store $7 $8
#ifdef __clang__
  $9 = New UVec2($3, $7)
#elif defined(__GNUC__)
  $9 = New UVec2($2, $7)
#endif
  $10 = Local UVec2
  Store $10 $9
  $11 = New UVec2($10)
  $12 = Local UVec2
  Store $12 $11
  $13 = New Vec3($6)
  $14 = Local Vec3
  Store $14 $13
  $16 = Return 0: $15
  Store $16 $14
  $18 = Return 1: $17
  Store $18 $12
}
