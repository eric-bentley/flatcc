#include <stdio.h>

/* Only needed for verification. */
#include "monster_test_reader.h"
#include "monster_test_json_parser.h"
#include "support/hexdump.h"
#include "support/readfile.h"
#include "support/elapsed.h"

#define FLATCC_BENCHMARK 0

#define BENCH_TITLE "monsterdata_test.golden"

#ifdef NDEBUG
#define COMPILE_TYPE "(optimized)"
#else
#define COMPILE_TYPE "(debug)"
#endif

#define FILE_SIZE_MAX (1024 * 10)

#undef ns
#define ns(x) FLATBUFFERS_WRAP_NAMESPACE(MyGame_Example, x)

/* A helper to simplify creating buffers vectors from C-arrays. */
#define c_vec_len(V) (sizeof(V)/sizeof((V)[0]))

int verify_parse(void *buffer)
{
    ns(Test_struct_t) test;
    ns(Vec3_struct_t) pos;
    ns(Monster_table_t) monster = ns(Monster_as_root_with_identifier)(buffer, ns(Monster_identifier));

    pos = ns(Monster_pos(monster));
    assert(pos);
    assert(ns(Vec3_x(pos) == 1));
    assert(ns(Vec3_y(pos) == 2));
    assert(ns(Vec3_z(pos) == 3));
    assert(ns(Vec3_test1(pos) == 3.0));
    assert(ns(Vec3_test2(pos) == ns(Color_Green)));
    test = ns(Vec3_test3(pos));
    assert(test);
    assert(ns(Test_a(test)) == 5);
    assert(ns(Test_b(test)) == 6);

    // TODO: hp and further fields

    return 0;

}
// TODO:
// when running benchmark with the wrong size argument (output size
// instead of input size), the warmup loop iterates indefinitely in the
// first iteration. This suggests there is an end check missing somwhere
// and this needs to be debugged. The input size as of this writing is 701
// bytes, and the output size is 288 bytes.
int test_parse()
{
    double t1, t2;

    const char *buf;
    void *flatbuffer;
    size_t in_size, out_size;
    flatcc_json_parser_t ctx;
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    int ret = -1;
    int i, rep = 1000000;
    int warmup_rep = 1000000;
    int flags = 0;
    const char *filename = "monsterdata_test.golden";

    flatcc_builder_init(B);

    buf = read_file(filename, FILE_SIZE_MAX, &in_size);
    if (!buf) {
        fprintf(stderr, "%s: could not read input json file\n", filename);
        return -1;
    }

    if (monster_test_parse_json(B, &ctx, buf, in_size, flags)) {
        goto failed;
    }
    fprintf(stderr, "%s: successfully parsed %d lines\n", filename, ctx.line);
    flatbuffer = flatcc_builder_finalize_buffer(B, &out_size);
    hexdump("parsed monsterdata_test.golden", flatbuffer, out_size, stdout);
    fprintf(stderr, "input size: %zu, output size: %zu\n", in_size, out_size);
    verify_parse(flatbuffer);

    flatcc_builder_reset(B);
#if FLATCC_BENCHMARK
    fprintf(stderr, "Now warming up\n");
    for (i = 0; i < warmup_rep; ++i) {
        if (monster_test_parse_json_root(B, &ctx, buf, in_size, flags)) {
            goto failed;
        }
        flatcc_builder_reset(B);
    }

    fprintf(stderr, "Now benchmarking\n");
    t1 = elapsed_realtime();
    for (i = 0; i < rep; ++i) {
        if (monster_test_parse_json_root(B, &ctx, buf, in_size, flags)) {
            goto failed;
        }
        flatcc_builder_reset(B);
    }
    t2 = elapsed_realtime();

    printf("----\n");
    show_benchmark(BENCH_TITLE " C generated JSON parse " COMPILE_TYPE, t1, t2, in_size, rep, "1M");
#endif
    ret = 0;

done:
    if (flatbuffer) {
        free(flatbuffer);
    }
    if (buf) {
        free((void *)buf);
    }
    flatcc_builder_clear(B);
    return ret;

failed:
    fprintf(stderr, "%s:%d:%d: %s\n",
            filename, (int)ctx.line, (int)(ctx.error_loc - ctx.line_start + 1),
            flatcc_json_parser_error_string(ctx.error));
    goto done;
}

int main(int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    fprintf(stderr, "JSON parse test\n");
    return test_parse();
}

