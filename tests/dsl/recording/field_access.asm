Block {
  Context {
    stage: Subroutine,
    name: recorded,
  }
  $0 = Local RecordingPair
  $1 = $0.x
  $2 = $0.y
  $3 = Local Float
  Store $3 $1
  $4 = Local Int32
  Store $4 $2
#ifdef __clang__
  $5 = Local Float
  Store $5 $3
  $6 = Local Int32
  Store $6 $4
  $7 = Add $5 $6
#elif defined(__GNUC__)
  $5 = Local Int32
  Store $5 $4
  $6 = Local Float
  Store $6 $3
  $7 = Add $6 $5
#endif
  $8 = Local Float
  Store $8 $7
}
