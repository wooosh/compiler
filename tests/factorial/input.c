s64 main() {
  return factorial(10)
}

s64 factorial(s64 n) {
  if n - 1 {
    return n * factorial(n-1)
  } else {
    return 1
  }
}
