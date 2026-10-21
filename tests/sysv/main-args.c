int main(int argc, char **argv) {
    if (argc < 1)
        return 1;
    if (!argv)
        return 2;
    if (!argv[0])
        return 3;
    return 0;
}
