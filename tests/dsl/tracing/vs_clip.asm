Block {
  Context {
    stage: Vertex,
  }
  $0 = Local Float
  $1 = 1
  Store $0 $1
  $2 = New Vec4($0, $0, $0, $0)
  $3 = Local Vec4
  Store $3 $2
  $4 = SV: ClipPosition
  Store $4 $3
}
