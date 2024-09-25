int func(int *p) {
    int y = *p;
    return y;
}

int main() {
    int *p = nullptr;
    return func(p);
}