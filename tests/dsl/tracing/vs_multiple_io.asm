Block {
  Context {
    stage: Vertex,
    stage inputs: { Smooth Vec3, Smooth Vec3, Smooth Vec2 }
    stage outputs: { Smooth Vec3, Smooth Vec3, Smooth Vec2 }
  }
  $0 = StageInput 0: Smooth Vec3
  $1 = StageInput 1: Smooth Vec3
  $2 = StageInput 2: Smooth Vec2
#ifdef __clang__
  $3 = New Vec3($0)
  $4 = Local Vec3
#elif defined(__GNUC__)
  $3 = New Vec2($2)
  $4 = Local Vec2
#endif
  Store $4 $3
  $5 = New Vec3($1)
  $6 = Local Vec3
  Store $6 $5
#ifdef __clang__
  $7 = New Vec2($2)
  $8 = Local Vec2
#elif defined(__GNUC__)
  $7 = New Vec3($0)
  $8 = Local Vec3
#endif
  Store $8 $7
  $9 = StageOutput 0: Smooth Vec3
#ifdef __clang__
  $10 = New Vec3($4)
#elif defined(__GNUC__)
  $10 = New Vec3($8)
#endif
  $11 = Local Vec3
  Store $11 $10
  Store $9 $11
  $12 = StageOutput 1: Smooth Vec3
  $13 = New Vec3($6)
  $14 = Local Vec3
  Store $14 $13
  Store $12 $14
  $15 = StageOutput 2: Smooth Vec2
#ifdef __clang__
  $16 = New Vec2($8)
#elif defined(__GNUC__)
  $16 = New Vec2($4)
#endif
  $17 = Local Vec2
  Store $17 $16
  Store $15 $17
}
