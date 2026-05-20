Block {
  Context {
    stage: Vertex,
    stage outputs: { Smooth Vec3, Flat UVec2 }
  }
  $0 = Local Float
  $1 = 1
  Store $0 $1
  $2 = New Vec3($0, $0, $0)
  $3 = Local Vec3
  Store $3 $2
  $4 = StageOutput 0: Smooth Vec3
  $5 = New Vec3($3)
  $6 = Local Vec3
  Store $6 $5
  Store $4 $6
  $7 = Local UInt32
#ifdef __clang__
  $8 = 1
#elif defined(__GNUC__)
  $8 = 4
#endif
  Store $7 $8
  $9 = Local UInt32
#ifdef __clang__
  $10 = 4
#elif defined(__GNUC__)
  $10 = 1
#endif
  Store $9 $10
#ifdef __clang__
  $11 = New UVec2($7, $9)
#elif defined(__GNUC__)
  $11 = New UVec2($9, $7)
#endif
  $12 = Local UVec2
  Store $12 $11
  $13 = StageOutput 1: Flat UVec2
  $14 = New UVec2($12)
  $15 = Local UVec2
  Store $15 $14
  Store $13 $15
}
