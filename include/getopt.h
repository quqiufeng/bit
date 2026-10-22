#ifndef __C1_GETOPT_H
#define __C1_GETOPT_H
extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;
int getopt(int argc, char *const argv[], const char *options);
struct option {
    const char *name;
    int has_arg;
    int *flag;
    int val;
};
#define no_argument 0
#define required_argument 1
#define optional_argument 2
int getopt_long(int argc, char *const argv[], const char *options,
                const struct option *long_options, int *index);
#endif
