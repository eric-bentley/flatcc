#include "support/readfile.h"
#include "flatcc/reflection/reflection_reader.h"

#define lld(x) (long long int)(x)

/* This is not an exhaustive test. */
int test_schema(const char *monster_bfbs)
{
    void *buffer;
    size_t size;
    int ret = -1;
    reflection_Schema_table_t S;
    reflection_Object_vec_t Objs;
    reflection_Object_table_t Obj;
    reflection_Field_vec_t Flds;
    reflection_Field_table_t F;
    reflection_Type_table_t T;
    int k, monster_index;

    buffer = read_file(monster_bfbs, 10000, &size);
    if (!buffer) {
        printf("failed to load binary schema\n");
        goto done;
    }
    S = reflection_Schema_as_root(buffer);
    Objs = reflection_Schema_objects(S);
    for (k = 0; k < reflection_Object_vec_len(Objs); ++k) {
        printf("dbg: obj #%d : %s\n", k,
                reflection_Object_name(reflection_Object_vec_at(Objs, k)));
    }
    k = (int)reflection_Object_vec_find(Objs, "MyGame.Example.Monster");
    if (k < 0) {
        printf("Could not find monster in schema\n");
        goto done;
    }
    monster_index = k;
    Obj = reflection_Object_vec_at(Objs, k);
    if (strcmp(reflection_Object_name(Obj), "MyGame.Example.Monster")) {
        printf("Found wrong object in schema\n");
        goto done;
    }
    Flds = reflection_Object_fields(Obj);
    k = reflection_Field_vec_find(Flds, "mana");
    if (k < 0) {
        printf("Did not find mana field in Monster schema\n");
        goto done;
    }
    F = reflection_Field_vec_at(Flds, k);
    if (reflection_Field_default_integer(F) != 150) {
        printf("mana field has wrong default value\n");
        printf("field name: %s\n", reflection_Field_name(F));
        printf("%lld\n", lld(reflection_Field_default_integer(F)));
        goto done;
    }
    T = reflection_Field_type(F);
    if (reflection_Type_base_type(T) != reflection_BaseType_Short) {
        printf("mana field has wrong type\n");
        goto done;
    }
    k = reflection_Field_vec_find(Flds, "enemy");
    if (k < 0) {
        printf("enemy field not found\n");
        goto done;
    }
    T = reflection_Field_type(reflection_Field_vec_at(Flds, k));
    if (reflection_Type_base_type(T) != reflection_BaseType_Obj) {
        printf("enemy is not an object\n");
        goto done;
    }
    if (reflection_Type_index(T) != monster_index) {
        printf("enemy is not a monster\n");
        goto done;
    }
    k = reflection_Field_vec_find(Flds, "testarrayoftables");
    if (k < 0) {
        printf("array of tables not found\n");
        goto done;
    }
    T = reflection_Field_type(reflection_Field_vec_at(Flds, k));
    if (reflection_Type_base_type(T) != reflection_BaseType_Vector) {
        printf("array of tables is not of vector type\n");
        goto done;
    }
    if (reflection_Type_element(T) != reflection_BaseType_Obj) {
        printf("array of tables is not a vector of table type\n");
        goto done;
    }
    if (reflection_Type_index(T) != monster_index) {
        printf("array of tables is not a monster vector\n");
        goto done;
    }

    ret = 0;
done:
    if (buffer) {
        free(buffer);
    }
    return ret;
}

int main(int argc, char *argv[])
{
    return test_schema("monster_test.bfbs");
}
