Block {
  Context {
    stage: Subroutine,
    name: recorded,
  }
  $0 = Argument 0: Int32
  $1 = Argument 1: Int32
  $2 = Local Float
  $3 = 0
  Store $2 $3
  $4 = Local Int32
  $5 = 0
  Store $4 $5
  $6 = Block {
    $7 = Local Int32
#ifdef __clang__
    Store $7 $4
#elif defined(__GNUC__)
    Store $7 $0
#endif
    $8 = Local Int32
#ifdef __clang__
    Store $8 $0
    $9 = Less $7 $8
#elif defined(__GNUC__)
    Store $8 $4
    $9 = Less $8 $7
#endif
    $10 = Local Bool
    Store $10 $9
    $11 = LogicalNot $10
    $12 = Local Bool
    Store $12 $11
    $13 = Local Bool
    Store $13 $12
    $14 = Local Bool
    Store $14 $13
    $15 = Block {
      Break
    }
    Branch $14: $15
    $16 = Local Float
    Store $16 $4
    $17 = Add $2 $16
    $18 = Local Float
    Store $18 $17
    Store $2 $18
    $19 = Add $4 $1
    $20 = Local Int32
    Store $20 $19
    Store $4 $20
  }
  Loop $6
  $22 = Return 0: $21
  Store $22 $2
}
