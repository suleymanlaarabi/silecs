#include <criterion/criterion.h>
#include "../ecs/addons/stdio_devtool_args.h"
#include "ecs_world.h"

Test(devtool_args, parse_simple_args) {
    command_args_t args;
    command_args_init(&args, "arg1 arg2 arg3");
    
    cr_assert_eq(command_args_count(&args), 3);
    cr_assert_str_eq(command_args_get(&args, 0), "arg1");
    cr_assert_str_eq(command_args_get(&args, 1), "arg2");
    cr_assert_str_eq(command_args_get(&args, 2), "arg3");
    
    command_args_free(&args);
}

Test(devtool_args, parse_args_with_spaces) {
    command_args_t args;
    command_args_init(&args, "  arg1   arg2  ");
    
    cr_assert_eq(command_args_count(&args), 2);
    cr_assert_str_eq(command_args_get(&args, 0), "arg1");
    cr_assert_str_eq(command_args_get(&args, 1), "arg2");
    
    command_args_free(&args);
}

Test(devtool_args, parse_quoted_args) {
    command_args_t args;
    command_args_init(&args, "arg1 \"quoted arg\" arg3");
    
    cr_assert_eq(command_args_count(&args), 3);
    cr_assert_str_eq(command_args_get(&args, 0), "arg1");
    cr_assert_str_eq(command_args_get(&args, 1), "quoted arg");
    cr_assert_str_eq(command_args_get(&args, 2), "arg3");
    
    command_args_free(&args);
}

Test(devtool_args, parse_empty_args) {
    command_args_t args;
    command_args_init(&args, "");
    
    cr_assert_eq(command_args_count(&args), 0);
    cr_assert_null(command_args_get(&args, 0));
    
    command_args_free(&args);
}

Test(devtool_args, parse_entity_by_name) {
    ecs_world_t *world = ecs_init();
    
    ecs_entity_t entity = ecs_new(world);
    ecs_add(world, entity, ecs_id(EcsName));
    char *name = strdup("TestEntity");
    ecs_set(world, entity, ecs_id(EcsName), &name);
    
    ecs_entity_t resolved;
    bool success = command_args_parse_entity(world, "TestEntity", &resolved);
    
    cr_assert(success);
    cr_assert_eq(resolved.value, entity.value);
    
    ecs_fini(world);
}

Test(devtool_args, parse_entity_by_index) {
    ecs_world_t *world = ecs_init();
    
    ecs_entity_t entity = ecs_new(world);
    
    char index_str[32];
    snprintf(index_str, sizeof(index_str), "%u", entity.index);
    
    ecs_entity_t resolved;
    bool success = command_args_parse_entity(world, index_str, &resolved);
    
    cr_assert(success);
    cr_assert_eq(resolved.value, entity.value);
    
    ecs_fini(world);
}

Test(devtool_args, parse_entity_not_found) {
    ecs_world_t *world = ecs_init();
    
    ecs_entity_t resolved;
    bool success = command_args_parse_entity(world, "NonExistent", &resolved);
    
    cr_assert_not(success);
    
    ecs_fini(world);
}
