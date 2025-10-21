#include <criterion/criterion.h>
#include "../ecs/rayflect/ecs_rayflect.h"
#include "../ecs/rayflect/rayflect_types.h"
#include "../ecs/rayflect/ecs_rayflect.h"
#include "rayflect/rayflect_format.h"
#include "rayflect/rayflect_parser.h"

#include <string.h>
#include <stdlib.h>

ECS_STRUCT(TestPosition, {
    float x;
    float y;
});

ECS_STRUCT(TestEntity, {
    int id;
    char *name;
    float health;
    int inventory[10];
});

ECS_STRUCT(TestVector3D, {
    double x;
    double y;
    double z;
});

Test(ecs_rayflect, parse_simple_struct) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));

    const char *def = "struct { int x; float y; }";
    rayflect_parse(&ecs_struct, def);

    cr_assert_eq(ecs_struct.fields.count, 2);

    ecs_field_t *field1 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 0);
    cr_assert_str_eq(field1->name, "x");
    cr_assert_eq(field1->size, 4);
    cr_assert_eq(field1->align, 4);

    ecs_field_t *field2 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 1);
    cr_assert_str_eq(field2->name, "y");
    cr_assert_eq(field2->size, 4);
    cr_assert_eq(field2->align, 4);

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, parse_struct_with_pointers) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));

    const char *def = "struct { char *name; int *data; }";
    rayflect_parse(&ecs_struct, def);

    cr_assert_eq(ecs_struct.fields.count, 2);

    ecs_field_t *field1 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 0);
    cr_assert_str_eq(field1->name, "name");
    cr_assert_eq(field1->size, sizeof(void*));
    cr_assert_eq(field1->align, sizeof(void*));

    ecs_field_t *field2 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 1);
    cr_assert_str_eq(field2->name, "data");
    cr_assert_eq(field2->size, sizeof(void*));
    cr_assert_eq(field2->align, sizeof(void*));

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, parse_struct_with_arrays) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));

    const char *def = "struct { int values[10]; char buffer[256]; }";
    rayflect_parse(&ecs_struct, def);

    cr_assert_eq(ecs_struct.fields.count, 2);

    ecs_field_t *field1 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 0);
    cr_assert_str_eq(field1->name, "values");
    cr_assert_eq(field1->size, 40);
    cr_assert_eq(field1->align, 4);

    ecs_field_t *field2 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 1);
    cr_assert_str_eq(field2->name, "buffer");
    cr_assert_eq(field2->size, 256);
    cr_assert_eq(field2->align, 1);

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, parse_struct_complex) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));

    const char *def = "struct { struct Vector3 position; float rotation[4]; int *ptr; }";
    rayflect_parse(&ecs_struct, def);

    cr_assert_eq(ecs_struct.fields.count, 3);

    ecs_field_t *field1 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 0);
    cr_assert_str_eq(field1->name, "position");

    ecs_field_t *field2 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 1);
    cr_assert_str_eq(field2->name, "rotation");
    cr_assert_eq(field2->size, 16);
    cr_assert_eq(field2->align, 4);

    ecs_field_t *field3 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 2);
    cr_assert_str_eq(field3->name, "ptr");
    cr_assert_eq(field3->size, sizeof(void*));
    cr_assert_eq(field3->align, sizeof(void*));

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, macro_ecs_struct_basic) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));

    rayflect_parse(&ecs_struct, ecs_rayflect_id(TestPosition));

    cr_assert_eq(ecs_struct.fields.count, 2);

    ecs_field_t *field1 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 0);
    cr_assert_str_eq(field1->name, "x");
    cr_assert_eq(field1->size, 4);
    cr_assert_eq(field1->align, 4);

    ecs_field_t *field2 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 1);
    cr_assert_str_eq(field2->name, "y");
    cr_assert_eq(field2->size, 4);
    cr_assert_eq(field2->align, 4);

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, macro_ecs_struct_complex) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));

    rayflect_parse(&ecs_struct, ecs_rayflect_id(TestEntity));

    rayflect_print(&ecs_struct, NULL);

    cr_assert_eq(ecs_struct.fields.count, 4);

    ecs_field_t *field1 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 0);
    cr_assert_str_eq(field1->name, "id");
    cr_assert_eq(field1->size, 4);
    cr_assert_eq(field1->align, 4);

    ecs_field_t *field2 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 1);
    cr_assert_str_eq(field2->name, "name");
    cr_assert_eq(field2->size, sizeof(void*));
    cr_assert_eq(field2->align, sizeof(void*));

    ecs_field_t *field3 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 2);
    cr_assert_str_eq(field3->name, "health");
    cr_assert_eq(field3->size, 4);
    cr_assert_eq(field3->align, 4);

    ecs_field_t *field4 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 3);
    cr_assert_str_eq(field4->name, "inventory");
    cr_assert_eq(field4->size, 40);
    cr_assert_eq(field4->align, 4);

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, macro_ecs_struct_with_print) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));

    rayflect_parse(&ecs_struct, ecs_rayflect_id(TestVector3D));

    rayflect_print(&ecs_struct, NULL);

    cr_assert_eq(ecs_struct.fields.count, 3);

    ecs_field_t *field1 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 0);
    cr_assert_eq(field1->size, 8);
    cr_assert_eq(field1->align, 8);

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, format_struct_basic) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));

    const char *def = "struct { int x; float y; }";
    rayflect_parse(&ecs_struct, def);

    ecs_string_t formatted = ecs_rayflect_format_struct(&ecs_struct, NULL);

    cr_assert_not_null(formatted.data);
    cr_assert_str_eq((char *)formatted.data, "{ x: i32, y: f32 }");

    ecs_vec_free(&formatted);
    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, format_struct_with_name) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));

    rayflect_parse(&ecs_struct, ecs_rayflect_id(TestPosition));

    ecs_string_t formatted = ecs_rayflect_format_struct(&ecs_struct, "Position");

    cr_assert_not_null(formatted.data);
    cr_assert_str_eq((char *)formatted.data, "Position { x: f32, y: f32 }");

    ecs_vec_free(&formatted);
    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, format_struct_complex) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));

    rayflect_parse(&ecs_struct, ecs_rayflect_id(TestEntity));

    ecs_string_t formatted = ecs_rayflect_format_struct(&ecs_struct, "Entity");

    cr_assert_not_null(formatted.data);
    const char *expected = "Entity { id: i32, name: ptr, health: f32, inventory: array[10] }";
    cr_assert_str_eq((char *)formatted.data, expected);

    ecs_vec_free(&formatted);
    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, user_example_multiline) {
    ECS_STRUCT(TestMultiline, {
        float x;
        float y;
    });

    TestMultiline unused;
    (void) unused;

    ecs_struct_t my_struct = {0};
    ecs_vec_init(&my_struct.fields, sizeof(ecs_field_t));

    rayflect_parse(&my_struct, ecs_rayflect_id(TestMultiline));
    rayflect_print(&my_struct, NULL);

    cr_assert_eq(my_struct.fields.count, 2);

    rayflect_free(&my_struct);
}

Test(ecs_rayflect, type_sizes_and_alignment) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));

    const char *def = "struct { char c; short s; int i; long l; float f; double d; }";
    rayflect_parse(&ecs_struct, def);

    cr_assert_eq(ecs_struct.fields.count, 6);

    ecs_field_t *char_field = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 0);
    cr_assert_eq(char_field->size, 1);
    cr_assert_eq(char_field->align, 1);
    cr_assert_eq(char_field->type, ECS_TYPE_I8);

    ecs_field_t *short_field = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 1);
    cr_assert_eq(short_field->size, 2);
    cr_assert_eq(short_field->align, 2);
    cr_assert_eq(short_field->type, ECS_TYPE_I16);

    ecs_field_t *int_field = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 2);
    cr_assert_eq(int_field->size, 4);
    cr_assert_eq(int_field->align, 4);
    cr_assert_eq(int_field->type, ECS_TYPE_I32);

    ecs_field_t *float_field = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 4);
    cr_assert_eq(float_field->size, 4);
    cr_assert_eq(float_field->align, 4);
    cr_assert_eq(float_field->type, ECS_TYPE_F32);

    ecs_field_t *double_field = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 5);
    cr_assert_eq(double_field->size, 8);
    cr_assert_eq(double_field->align, 8);
    cr_assert_eq(double_field->type, ECS_TYPE_F64);

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, simple_types_display) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));

    const char *def = "struct { int id; float *position; double values[5]; char name; }";
    rayflect_parse(&ecs_struct, def);
    rayflect_print(&ecs_struct, NULL);

    cr_assert_eq(ecs_struct.fields.count, 4);

    ecs_field_t *field1 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 0);
    cr_assert_eq(field1->type, ECS_TYPE_I32);

    ecs_field_t *field2 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 1);
    cr_assert_eq(field2->type, ECS_TYPE_PTR);

    ecs_field_t *field3 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 2);
    cr_assert_eq(field3->type, ECS_TYPE_ARRAY);
    cr_assert_eq(field3->array_size, 5);

    ecs_field_t *field4 = ECS_VEC_GET(ecs_field_t, &ecs_struct.fields, 3);
    cr_assert_eq(field4->type, ECS_TYPE_I8);

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, print_with_data) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));

    const char *def = "struct { int x; float y; }";
    rayflect_parse(&ecs_struct, def);

    struct { int x; float y; } test_data = { 42, 3.14f };

    printf("\nTest: printing struct with values:\n");
    rayflect_print(&ecs_struct, &test_data);

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, set_field_int) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));
    const char *def = "struct { int x; float y; }";
    rayflect_parse(&ecs_struct, def);

    struct { int x; float y; } test_data = { 42, 3.14f };

    int new_x = 100;
    int result = rayflect_set_field(&ecs_struct, &test_data, "x", &new_x);
    
    cr_assert_eq(result, 0, "rayflect_set_field should return 0 on success");
    cr_assert_eq(test_data.x, 100, "x field should be updated to 100");
    cr_assert_float_eq(test_data.y, 3.14f, 0.001f, "y field should remain unchanged");

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, set_field_float) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));
    const char *def = "struct { int x; float y; }";
    rayflect_parse(&ecs_struct, def);

    struct { int x; float y; } test_data = { 42, 3.14f };

    float new_y = 9.99f;
    int result = rayflect_set_field(&ecs_struct, &test_data, "y", &new_y);
    
    cr_assert_eq(result, 0, "rayflect_set_field should return 0 on success");
    cr_assert_eq(test_data.x, 42, "x field should remain unchanged");
    cr_assert_float_eq(test_data.y, 9.99f, 0.001f, "y field should be updated to 9.99");

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, set_field_complex_struct) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));
    rayflect_parse(&ecs_struct, ecs_rayflect_id(TestEntity));

    TestEntity test_entity = { 1, NULL, 100.0f, {0} };

    int new_id = 999;
    int result = rayflect_set_field(&ecs_struct, &test_entity, "id", &new_id);
    cr_assert_eq(result, 0, "rayflect_set_field should return 0 on success");
    cr_assert_eq(test_entity.id, 999, "id field should be updated to 999");

    float new_health = 75.5f;
    result = rayflect_set_field(&ecs_struct, &test_entity, "health", &new_health);
    cr_assert_eq(result, 0, "rayflect_set_field should return 0 on success");
    cr_assert_float_eq(test_entity.health, 75.5f, 0.001f, "health field should be updated to 75.5");

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, set_field_pointer) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));
    rayflect_parse(&ecs_struct, ecs_rayflect_id(TestEntity));

    TestEntity test_entity = { 1, NULL, 100.0f, {0} };

    char *new_name = "TestName";
    int result = rayflect_set_field(&ecs_struct, &test_entity, "name", &new_name);
    cr_assert_eq(result, 0, "rayflect_set_field should return 0 on success");
    cr_assert_str_eq(test_entity.name, "TestName", "name field should be updated");

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, set_field_not_found) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));
    const char *def = "struct { int x; float y; }";
    rayflect_parse(&ecs_struct, def);

    struct { int x; float y; } test_data = { 42, 3.14f };

    int new_value = 100;
    int result = rayflect_set_field(&ecs_struct, &test_data, "z", &new_value);
    
    cr_assert_eq(result, -1, "rayflect_set_field should return -1 when field is not found");

    rayflect_free(&ecs_struct);
}

Test(ecs_rayflect, set_field_null_params) {
    ecs_struct_t ecs_struct = {0};
    ecs_vec_init(&ecs_struct.fields, sizeof(ecs_field_t));
    const char *def = "struct { int x; float y; }";
    rayflect_parse(&ecs_struct, def);

    struct { int x; float y; } test_data = { 42, 3.14f };
    int new_value = 100;

    int result = rayflect_set_field(NULL, &test_data, "x", &new_value);
    cr_assert_eq(result, -1, "rayflect_set_field should return -1 when struct is NULL");

    result = rayflect_set_field(&ecs_struct, NULL, "x", &new_value);
    cr_assert_eq(result, -1, "rayflect_set_field should return -1 when instance is NULL");

    result = rayflect_set_field(&ecs_struct, &test_data, NULL, &new_value);
    cr_assert_eq(result, -1, "rayflect_set_field should return -1 when field_name is NULL");

    result = rayflect_set_field(&ecs_struct, &test_data, "x", NULL);
    cr_assert_eq(result, -1, "rayflect_set_field should return -1 when value is NULL");

    rayflect_free(&ecs_struct);
}
