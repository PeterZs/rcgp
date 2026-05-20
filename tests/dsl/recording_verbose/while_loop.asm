Block {
  Context {
    stage: Subroutine,
    name: recorded,
  }
  $0 = Int32
  $1 = Float
  $2 = Argument 0: $0
  $3 = Argument 1: $0
  $4 = Local $1
  $5 = 0
  Store $4 $5
  $6 = Local $0
  $7 = 0
  Store $6 $7
  $8 = Block {
    $1 = Float
    $0 = Int32
    $9 = Local $0
#ifdef __clang__
    Store $9 $6
#elif defined(__GNUC__)
    Store $9 $2
#endif
    $10 = Local $0
#ifdef __clang__
    Store $10 $2
    $11 = Less $9 $10
#elif defined(__GNUC__)
    Store $10 $6
    $11 = Less $10 $9
#endif
    $12 = Bool
    $13 = Local $12
    Store $13 $11
    $14 = LogicalNot $13
    $15 = Local $12
    Store $15 $14
    $16 = Local $12
    Store $16 $15
    $17 = Local $12
    Store $17 $16
    $18 = Block {
      Break
    }
    Branch $17: $18
    $19 = Local $1
    Store $19 $6
    $20 = Add $4 $19
    $21 = Local $1
    Store $21 $20
    Store $4 $21
    $22 = Add $6 $3
    $23 = Local $0
    Store $23 $22
    Store $6 $23
  }
  Loop $8
  $24 = Return 0: $1
  Store $24 $4
}
