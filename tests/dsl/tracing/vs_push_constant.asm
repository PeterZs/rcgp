Block {
  Context {
    stage: Vertex,
    stage inputs: { Smooth Vec3 }
    stage outputs: { Smooth Vec3 }
    resources: { $0 },
  }
  $0 = PushConstant +4294967295: Std430 fwd::View
  $1 = $0.model
  $2 = $0.view
  $3 = $0.proj
  $4 = StageInput 0: Smooth Vec3
  $5 = New Vec3($4)
  $6 = Local Vec3
  Store $6 $5
  $7 = Local Float
  $8 = 1
  Store $7 $8
  $9 = New Vec4($6, $7)
  $10 = Local Vec4
  Store $10 $9
  $11 = Multiply $1 $10
  $12 = Local Vec4
  Store $12 $11
  $13 = Multiply $3 $2
  $14 = Local FMat4x4
  Store $14 $13
  $15 = Multiply $14 $12
  $16 = Local Vec4
  Store $16 $15
  $17 = SV: ClipPosition
  Store $17 $16
  $18 = New Vec4($12)
  $19 = Local Vec4
  Store $19 $18
  $20 = New Vec3($19)
  $21 = Local Vec3
  Store $21 $20
  $22 = StageOutput 0: Smooth Vec3
  Store $22 $21
}
