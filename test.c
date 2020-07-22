int add(int a, int b) {}
int magic_function(int a) {
    return add(42, a)
}

void main(int argc) {
    add(magic_function(add(1, 2)), 3)
    return magic_function(argc, 42)
}
