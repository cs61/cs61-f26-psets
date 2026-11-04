#include "io61.hh"
#include <sys/stat.h>

// Usage: ./reversewrite61 [-s SIZE] [-o OUTFILE] [FILE]
//    Copies the input FILE to OUTFILE one character at a time,
//    reversing the order of characters while writing the output.
//    Reads using stdio and writes using io61.

int main(int argc, char* argv[]) {
    // Parse arguments
    io61_args args = io61_args("s:o:i:Fy").parse(argc, argv);

    // Open files, measure file sizes
    FILE* inf = stdio_open_check(args.input_file, O_RDONLY);
    args.after_open(inf, O_RDONLY);
    io61_file* outf = io61_open_check(args.output_file,
                                      O_WRONLY | O_CREAT | O_TRUNC);
    if (io61_seek(outf, 0) < 0) {
        fprintf(stderr, "reversewrite61: output file is not seekable\n");
        exit(1);
    }

    if (ssize_t(args.file_size) < 0) {
        struct stat st;
        if (fstat(fileno(inf), &st) == 0 && S_ISREG(st.st_mode)) {
            args.file_size = st.st_size;
        }
    }
    if (ssize_t(args.file_size) < 0) {
        fprintf(stderr, "reversewrite61: can't get size of input file\n");
        exit(1);
    }

    while (args.file_size != 0) {
        --args.file_size;
        int ch = fgetc(inf);
        assert(ch >= 0);

        int r = io61_seek(outf, args.file_size);
        assert(r == 0);

        r = io61_writec(outf, ch);
        assert(r == 0);

        args.after_write(outf);
    }

    fclose(inf);
    io61_close(outf);
}
