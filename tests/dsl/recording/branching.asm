Block {
  Context {
    stage: Subroutine,
    name: recorded,
  }
  $0 = Local Int32
  $1 = 12
  Store $0 $1
  $2 = Local Int32
#ifdef __clang__
  Store $2 $0
  $3 = Local Int32
  $4 = 11
  Store $3 $4
  $5 = Greater $2 $3
#elif defined(__GNUC__)
  $3 = 11
  Store $2 $3
  $4 = Local Int32
  Store $4 $0
  $5 = Less $4 $2
#endif
  $6 = Local Bool
  Store $6 $5
#ifdef __clang__
  $7 = Local Bool
  Store $7 $6
  $8 = Local Bool
  Store $8 $7
#elif defined(__GNUC__)
  $7 = Local Int32
  $8 = 5
  Store $7 $8
#endif
  $9 = Local Int32
  Store $9 $0
#ifdef __clang__
  $10 = Local Int32
  $11 = 11
  Store $10 $11
  $12 = Less $9 $10
#elif defined(__GNUC__)
  $10 = Greater $9 $7
  $11 = Local Bool
  Store $11 $10
  $12 = LogicalAnd $6 $11
#endif
  $13 = Local Bool
  Store $13 $12
#ifdef __clang__
  $14 = Local Int32
  Store $14 $0
#elif defined(__GNUC__)
  $14 = Local Bool
  Store $14 $13
#endif
  $15 = Local Int32
#ifdef __clang__
  $16 = 5
#elif defined(__GNUC__)
  $16 = 11
#endif
  Store $15 $16
#ifdef __clang__
  $17 = Greater $14 $15
  $18 = Local Bool
  Store $18 $17
  $19 = LogicalAnd $13 $18
#elif defined(__GNUC__)
  $17 = Local Int32
  Store $17 $0
  $18 = Greater $17 $15
  $19 = Local Bool
  Store $19 $18
#endif
  $20 = Local Bool
  Store $20 $19
  $21 = Local Bool
  Store $21 $20
  $22 = Local Bool
#ifdef __clang__
  Store $22 $8
#elif defined(__GNUC__)
  Store $22 $21
#endif
  $23 = Local Bool
#ifdef __clang__
  Store $23 $21
#elif defined(__GNUC__)
  Store $23 $14
#endif
  $24 = Local Bool
  Store $24 $22
  $25 = Block {
    $26 = Local Int32
    $27 = 1
    Store $26 $27
    $28 = Add $0 $26
    $29 = Local Int32
    Store $29 $28
    Store $0 $29
  }
  $30 = Block {
    $31 = Local Int32
    $32 = 2
    Store $31 $32
    $33 = Add $0 $31
    $34 = Local Int32
    Store $34 $33
    Store $0 $34
  }
  $35 = Block {
    $36 = Local Int32
    $37 = 3
    Store $36 $37
    $38 = Add $0 $36
    $39 = Local Int32
    Store $39 $38
    Store $0 $39
  }
  Branch $24: $25, $23: $30, else: $35
}
