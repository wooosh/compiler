s64 main() {
  let x s64
  let y s64
  x = 1
  y = 0
  let out s64
  if y {
    out = 10000
  } else {
    out = 40
    if x {
      out = out + 2
    }
  }

  return out
}
