Block {
  Context {
    stage: Subroutine,
    name: recorded,
  }
  $0 = Float
  $1 = Local $0
  $2 = 1
  Store $1 $2
  $3 = Vec2
  $4 = New $3($1, $1)
  $5 = Local $3
  Store $5 $4
  $6 = Local $0
  $7 = 1
  Store $6 $7
  $8 = Subtract $5 $6
  $9 = Local $3
  Store $9 $8
#ifdef __clang__
  $10 = Swizzle($9: x)
#elif defined(__GNUC__)
  $10 = Swizzle($9: y)
#endif
  $11 = Local $0
  Store $11 $10
#ifdef __clang__
  $12 = Swizzle($9: y)
  $13 = Local $0
  Store $13 $12
  $14 = Local $0
  $15 = -1
  Store $14 $15
  $16 = Multiply $14 $13
#elif defined(__GNUC__)
  $12 = Local $0
  $13 = -1
  Store $12 $13
  $14 = Multiply $12 $11
  $15 = Local $0
  Store $15 $14
  $16 = Swizzle($9: x)
#endif
  $17 = Local $0
  Store $17 $16
#ifdef __clang__
  $18 = New $3($11, $17)
#elif defined(__GNUC__)
  $18 = New $3($17, $15)
#endif
  $19 = Local $3
  Store $19 $18
  Store $9 $19
}
