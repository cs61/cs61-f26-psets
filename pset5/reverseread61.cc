#include "io61.hh"

// Usage: ./reverseread61 [-s SIZE] [-o OUTFILE] [FILE]
//    Copies the input FILE to OUTFILE one character at a time,
//    reversing the order of characters in the input.
//    Reads using io61 and writes using stdio.

int main(int argc, char* argv[]) {
    // Parse arguments
    io61_args args = io61_args("s:o:i:qFyA:").parse(argc, argv);

    // Open files, measure file sizes
    io61_file* inf = io61_open_check(args.input_file, O_RDONLY);
    if (io61_seek(inf, 0) < 0) {
        fprintf(stderr, "reverseread61: input file is not seekable\n");
        exit(1);
    }
    FILE* outf = stdio_open_check(args.output_file,
                                  O_WRONLY | O_CREAT | O_TRUNC);
    args.after_open(outf, O_WRONLY);

    if (ssize_t(args.file_size) < 0) {
        args.file_size = io61_filesize(inf);
    }
    if (ssize_t(args.file_size) < 0) {
        fprintf(stderr, "reverseread61: can't get size of input file\n");
        exit(1);
    }

    while (args.file_size != 0) {
        --args.file_size;
        int r = io61_seek(inf, args.file_size);
        assert(r == 0 || args.quiet);

        int ch = io61_readc(inf);
        assert(ch >= 0);

        r = fputc(ch, outf);
        // Unlike `io61_writec`, `fputc` returns the character written.
        assert(r == ch);

        args.after_write(outf);
    }

    io61_close(inf);
    fclose(outf);
}
