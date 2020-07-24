sint add(sint a, sint b, sint c) {return 3}
sint add(sint a, sint b) {return 2}
sint magic_function(sint a) {
    add(42, a)
}

void main(sint argc) {
    add(magic_function(add(1, 2)), 3)
    return magic_function(argc, 42)
}
