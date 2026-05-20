Block {
  Context {
    stage: Vertex,
    stage outputs: { Smooth Vec3, Smooth Vec3, Smooth UVec2, Smooth Vec3, Smooth Vec3 }
  }
  $0 = Local Float
  $1 = 1
  Store $0 $1
  $2 = Local Float
  Store $2 $0
  $3 = Local Vec3
  sr1($2, $3)
#ifdef __clang__
  $4 = New Vec3($3)
  $5 = Local Vec3
  Store $5 $4
#elif defined(__GNUC__)
  $4 = Local UInt32
  $5 = 2
  Store $4 $5
#endif
  $6 = Local Float
  $7 = 1
  Store $6 $7
#ifdef __clang__
  $8 = Local UInt32
  $9 = 2
  Store $8 $9
  $10 = Local Float
  Store $10 $6
  $11 = Local UInt32
  Store $11 $8
  $12 = Local Vec3
#elif defined(__GNUC__)
  $8 = Local Float
  Store $8 $6
  $9 = Local UInt32
  Store $9 $4
  $10 = Local Vec3
  $11 = Local UVec2
  sr2($8, $9, $10, $11)
  $12 = New UVec2($10)
#endif
  $13 = Local UVec2
#ifdef __clang__
  sr2($10, $11, $12, $13)
  $14 = New Vec3($12)
#elif defined(__GNUC__)
  Store $13 $12
  $14 = New Vec3($11)
#endif
  $15 = Local Vec3
  Store $15 $14
#ifdef __clang__
  $16 = New UVec2($13)
  $17 = Local UVec2
  Store $17 $16
  $18 = New UVec2($17)
  $19 = Local UVec2
  Store $19 $18
  $20 = New Vec3($15)
  $21 = Local Vec3
  Store $21 $20
  $22 = Local Float
  $23 = 2
  Store $22 $23
  $24 = Local Float
  Store $24 $22
  $25 = Local rcgp::Ray
  sr3($24, $25)
  $26 = $25.origin
  $27 = $25.direction
  $28 = New Vec3($26)
#elif defined(__GNUC__)
  $16 = Local Float
  $17 = 2
  Store $16 $17
  $18 = Local Float
  Store $18 $16
  $19 = Local Ray
  sr3($18, $19)
  $20 = $19.origin
  $21 = $19.direction
  $22 = New Vec3($21)
  $23 = Local Vec3
  Store $23 $22
  $24 = New Vec3($20)
  $25 = Local Vec3
  Store $25 $24
  $26 = New UVec2($13)
  $27 = Local UVec2
  Store $27 $26
  $28 = New Vec3($15)
#endif
  $29 = Local Vec3
  Store $29 $28
#ifdef __clang__
  $30 = New Vec3($27)
#elif defined(__GNUC__)
  $30 = New Vec3($3)
#endif
  $31 = Local Vec3
  Store $31 $30
#ifdef __clang__
  $32 = New Vec3($31)
  $33 = Local Vec3
  Store $33 $32
  $34 = New Vec3($29)
  $35 = Local Vec3
  Store $35 $34
  $36 = New UVec2($19)
  $37 = Local UVec2
  Store $37 $36
  $38 = New Vec3($21)
  $39 = Local Vec3
  Store $39 $38
  $40 = New Vec3($5)
  $41 = Local Vec3
  Store $41 $40
  $42 = StageOutput 0: Smooth Vec3
  Store $42 $41
  $43 = StageOutput 1: Smooth Vec3
  Store $43 $39
  $44 = StageOutput 2: Smooth UVec2
  Store $44 $37
  $45 = StageOutput 3: Smooth Vec3
  Store $45 $35
  $46 = StageOutput 4: Smooth Vec3
  Store $46 $33
#elif defined(__GNUC__)
  $32 = StageOutput 0: Smooth Vec3
  Store $32 $31
  $33 = StageOutput 1: Smooth Vec3
  Store $33 $29
  $34 = StageOutput 2: Smooth UVec2
  Store $34 $27
  $35 = StageOutput 3: Smooth Vec3
  Store $35 $25
  $36 = StageOutput 4: Smooth Vec3
  Store $36 $23
#endif
}
