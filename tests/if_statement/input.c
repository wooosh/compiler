void main() {
  let x sint
  let y sint
  x = 1
  y = 0
  let out sint
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
