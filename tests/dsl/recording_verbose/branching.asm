Block {
  Context {
    stage: Subroutine,
    name: recorded,
  }
  $0 = Int32
  $1 = Local $0
  $2 = 12
  Store $1 $2
  $3 = Local $0
#ifdef __clang__
  Store $3 $1
  $4 = Local $0
  $5 = 11
  Store $4 $5
  $6 = Greater $3 $4
#elif defined(__GNUC__)
  $4 = 11
  Store $3 $4
  $5 = Local $0
  Store $5 $1
  $6 = Less $5 $3
#endif
  $7 = Bool
  $8 = Local $7
  Store $8 $6
#ifdef __clang__
  $9 = Local $7
  Store $9 $8
  $10 = Local $7
  Store $10 $9
#elif defined(__GNUC__)
  $9 = Local $0
  $10 = 5
  Store $9 $10
#endif
  $11 = Local $0
  Store $11 $1
#ifdef __clang__
  $12 = Local $0
  $13 = 11
  Store $12 $13
  $14 = Less $11 $12
#elif defined(__GNUC__)
  $12 = Greater $11 $9
  $13 = Local $7
  Store $13 $12
  $14 = LogicalAnd $8 $13
#endif
  $15 = Local $7
  Store $15 $14
#ifdef __clang__
  $16 = Local $0
  Store $16 $1
#elif defined(__GNUC__)
  $16 = Local $7
  Store $16 $15
#endif
  $17 = Local $0
#ifdef __clang__
  $18 = 5
#elif defined(__GNUC__)
  $18 = 11
#endif
  Store $17 $18
#ifdef __clang__
  $19 = Greater $16 $17
  $20 = Local $7
  Store $20 $19
  $21 = LogicalAnd $15 $20
#elif defined(__GNUC__)
  $19 = Local $0
  Store $19 $1
  $20 = Greater $19 $17
  $21 = Local $7
  Store $21 $20
#endif
  $22 = Local $7
  Store $22 $21
  $23 = Local $7
  Store $23 $22
  $24 = Local $7
#ifdef __clang__
  Store $24 $10
#elif defined(__GNUC__)
  Store $24 $23
#endif
  $25 = Local $7
#ifdef __clang__
  Store $25 $23
#elif defined(__GNUC__)
  Store $25 $16
#endif
  $26 = Local $7
  Store $26 $24
  $27 = Block {
    $0 = Int32
    $28 = Local $0
    $29 = 1
    Store $28 $29
    $30 = Add $1 $28
    $31 = Local $0
    Store $31 $30
    Store $1 $31
  }
  $32 = Block {
    $0 = Int32
    $33 = Local $0
    $34 = 2
    Store $33 $34
    $35 = Add $1 $33
    $36 = Local $0
    Store $36 $35
    Store $1 $36
  }
  $37 = Block {
    $0 = Int32
    $38 = Local $0
    $39 = 3
    Store $38 $39
    $40 = Add $1 $38
    $41 = Local $0
    Store $41 $40
    Store $1 $41
  }
  Branch $26: $27, $25: $32, else: $37
}
