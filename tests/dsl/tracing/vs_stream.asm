Block {
  Context {
    stage: Vertex,
    stage inputs: { Smooth Vec3 }
    stage outputs: { Smooth Vec3 }
  }
  $0 = StageInput 0: Smooth Vec3
  $1 = New Vec3($0)
  $2 = Local Vec3
  Store $2 $1
  $3 = Local Float
  $4 = 1
  Store $3 $4
  $5 = New Vec4($2, $3)
  $6 = Local Vec4
  Store $6 $5
  $7 = SV: ClipPosition
  Store $7 $6
  $8 = New Vec3($2)
  $9 = Local Vec3
  Store $9 $8
  $10 = StageOutput 0: Smooth Vec3
  Store $10 $9
}
