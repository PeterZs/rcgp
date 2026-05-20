Block {
  Context {
    stage: Subroutine,
    name: recorded,
  }
  $0 = Local Float
  $1 = 1
  Store $0 $1
  $2 = New Vec2($0, $0)
  $3 = Local Vec2
  Store $3 $2
  $4 = Local Float
  $5 = 1
  Store $4 $5
  $6 = Subtract $3 $4
  $7 = Local Vec2
  Store $7 $6
#ifdef __clang__
  $8 = Swizzle($7: x)
#elif defined(__GNUC__)
  $8 = Swizzle($7: y)
#endif
  $9 = Local Float
  Store $9 $8
#ifdef __clang__
  $10 = Swizzle($7: y)
  $11 = Local Float
  Store $11 $10
  $12 = Local Float
  $13 = -1
  Store $12 $13
  $14 = Multiply $12 $11
#elif defined(__GNUC__)
  $10 = Local Float
  $11 = -1
  Store $10 $11
  $12 = Multiply $10 $9
  $13 = Local Float
  Store $13 $12
  $14 = Swizzle($7: x)
#endif
  $15 = Local Float
  Store $15 $14
#ifdef __clang__
  $16 = New Vec2($9, $15)
#elif defined(__GNUC__)
  $16 = New Vec2($15, $13)
#endif
  $17 = Local Vec2
  Store $17 $16
  Store $7 $17
}
