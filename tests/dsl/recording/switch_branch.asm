Block {
  Context {
    stage: Subroutine,
    name: recorded,
  }
  $0 = Local Int32
  $1 = 1
  Store $0 $1
  $2 = Local Int32
#ifdef __clang__
  Store $2 $0
  $3 = Local Int32
  $4 = 0
  Store $3 $4
#elif defined(__GNUC__)
  $3 = 1
  Store $2 $3
  $4 = Local Int32
  Store $4 $2
#endif
  $5 = Local Int32
#ifdef __clang__
  Store $5 $3
  $6 = Local Int32
  Store $6 $2
#elif defined(__GNUC__)
  $6 = 0
  Store $5 $6
#endif
  $7 = Local Int32
  Store $7 $5
#ifdef __clang__
  $8 = Equal $6 $7
  $9 = Local Bool
  Store $9 $8
  $10 = Local Bool
  Store $10 $9
  $11 = Local Int32
  Store $11 $2
  $12 = Local Int32
  $13 = 1
  Store $12 $13
#elif defined(__GNUC__)
  $8 = Local Int32
  Store $8 $0
  $9 = Local Int32
  Store $9 $7
  $10 = Local Int32
  Store $10 $8
  $11 = Equal $10 $9
  $12 = Local Bool
  Store $12 $11
  $13 = Local Bool
  Store $13 $12
#endif
  $14 = Local Int32
#ifdef __clang__
  Store $14 $12
#elif defined(__GNUC__)
  Store $14 $8
#endif
  $15 = Local Int32
#ifdef __clang__
  Store $15 $11
#elif defined(__GNUC__)
  Store $15 $4
#endif
  $16 = Local Int32
  Store $16 $14
#ifdef __clang__
  $17 = Equal $15 $16
#elif defined(__GNUC__)
  $17 = Equal $16 $15
#endif
  $18 = Local Bool
  Store $18 $17
  $19 = Local Bool
  Store $19 $18
  $20 = Local Int32
#ifdef __clang__
  Store $20 $11
#elif defined(__GNUC__)
  Store $20 $14
#endif
  $21 = Local Int32
  Store $21 $20
  $22 = Block {
    $23 = Local Int32
    $24 = 10
    Store $23 $24
    $25 = Add $0 $23
    $26 = Local Int32
    Store $26 $25
    Store $0 $26
  }
  $27 = Block {
    $28 = Local Int32
    $29 = 20
    Store $28 $29
    $30 = Add $0 $28
    $31 = Local Int32
    Store $31 $30
    Store $0 $31
  }
  $32 = Block {
    $33 = Local Int32
    $34 = 30
    Store $33 $34
    $35 = Add $0 $33
    $36 = Local Int32
    Store $36 $35
    Store $0 $36
  }
#ifdef __clang__
  Branch $10: $22, $19: $27, else: $32
#elif defined(__GNUC__)
  Branch $13: $22, $19: $27, else: $32
#endif
}
