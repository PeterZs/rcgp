Block {
  Context {
    stage: Subroutine,
    name: recorded,
  }
  $0 = Float
  $1 = Int32
  $2 = RecordingPair { x: $0, y: $1 }
  $3 = Local $2
  $4 = $3.x
  $5 = $3.y
  $6 = Local $0
  Store $6 $4
  $7 = Local $1
  Store $7 $5
#ifdef __clang__
  $8 = Local $0
  Store $8 $6
  $9 = Local $1
  Store $9 $7
  $10 = Add $8 $9
#elif defined(__GNUC__)
  $8 = Local $1
  Store $8 $7
  $9 = Local $0
  Store $9 $6
  $10 = Add $9 $8
#endif
  $11 = Local $0
  Store $11 $10
}
