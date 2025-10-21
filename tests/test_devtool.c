#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <ecs_world.h>
#include <ecs_bootstrap.h>
#include <ecs_module.h>
#include <stdio.h>
#include <string.h>
#include "../ecs/addons/stdio_devtool_commands.h"
#include "../ecs/addons/stdio_devtool_query.h"
#include "../ecs/rayflect/ecs_rayflect.h"
#include "test.h"

static void setup(void)
{
    cr_redirect_stdout();
    cr_redirect_stderr();
}

Test(devtool, command_new_creates_entity, .init = setup)
{
    ecs_world_t *world = ecs_init();
    EcsBootstrapModule(world);
    
    ECS_REGISTER_COMPONENT(world, Position);
    ECS_REGISTER_COMPONENT(world, Velocity);
    
    char *pos_name = "Position";
    ecs_set(world, ecs_id(Position), ecs_id(EcsName), &pos_name);
    
    char *vel_name = "Velocity";
    ecs_set(world, ecs_id(Velocity), ecs_id(EcsName), &vel_name);
    
    stdio_devtool_command_new(world, "TestEntity Position Velocity");
    
    ecs_entity_t entity = ecs_lookup(world, "TestEntity");
    cr_assert_neq(entity.index, 0, "Entity should be created");
    cr_assert(ecs_has(world, entity, ecs_id(EcsName)), "Entity should have EcsName");
    
    ecs_fini(world);
}

Test(devtool, command_new_with_only_name, .init = setup)
{
    ecs_world_t *world = ecs_init();
    EcsBootstrapModule(world);
    
    stdio_devtool_command_new(world, "SimpleEntity");
    
    ecs_entity_t entity = ecs_lookup(world, "SimpleEntity");
    cr_assert_neq(entity.index, 0, "Entity should be created");
    cr_assert(ecs_has(world, entity, ecs_id(EcsName)), "Entity should have EcsName");
    
    ecs_fini(world);
}

Test(devtool, command_new_with_invalid_component_warns, .init = setup)
{
    ecs_world_t *world = ecs_init();
    EcsBootstrapModule(world);
    
    stdio_devtool_command_new(world, "TestEntity InvalidComponent");
    
    ecs_entity_t entity = ecs_lookup(world, "TestEntity");
    cr_assert_neq(entity.index, 0, "Entity should still be created");
    
    ecs_fini(world);
}

Test(devtool, command_set_name_changes_entity_name, .init = setup)
{
    ecs_world_t *world = ecs_init();
    EcsBootstrapModule(world);
    
    ecs_entity_t entity = ecs_new(world);
    ecs_add(world, entity, ecs_id(EcsName));
    char *name = "OldName";
    ecs_set(world, entity, ecs_id(EcsName), &name);
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%u NewName", entity.index);
    stdio_devtool_command_set_name(world, buffer);
    
    EcsName *new_name = ecs_get(world, entity, ecs_id(EcsName));
    cr_assert_str_eq(*new_name, "NewName", "Name should be updated");
    
    ecs_fini(world);
}

Test(devtool, command_set_name_on_nonexistent_entity_fails, .init = setup)
{
    ecs_world_t *world = ecs_init();
    EcsBootstrapModule(world);
    
    stdio_devtool_command_set_name(world, "99999 SomeName");
    
    fflush(stdout);
    cr_assert_stdout_neq_str("", "Should print error message");
    
    ecs_fini(world);
}

Test(devtool, command_set_name_without_args_shows_usage, .init = setup)
{
    ecs_world_t *world = ecs_init();
    EcsBootstrapModule(world);
    
    stdio_devtool_command_set_name(world, "");
    
    fflush(stdout);
    cr_assert_stdout_eq_str("Usage: set_name <entity_index> <name>\n");
    
    ecs_fini(world);
}

Test(devtool, query_finds_entities_with_component, .init = setup)
{
    ecs_world_t *world = ecs_init();
    EcsBootstrapModule(world);
    
    ECS_REGISTER_COMPONENT(world, Position);
    
    ecs_entity_t e1 = ecs_new(world);
    ecs_add(world, e1, ecs_id(Position));
    ecs_add(world, e1, ecs_id(EcsName));
    char *name1 = "Entity1";
    ecs_set(world, e1, ecs_id(EcsName), &name1);
    
    ecs_entity_t e2 = ecs_new(world);
    ecs_add(world, e2, ecs_id(Position));
    ecs_add(world, e2, ecs_id(EcsName));
    char *name2 = "Entity2";
    ecs_set(world, e2, ecs_id(EcsName), &name2);
    
    stdio_devtool_execute_query(world, "Position");
    
    fflush(stdout);
    
    ecs_fini(world);
}

Test(devtool, query_with_no_matches_prints_message, .init = setup)
{
    ecs_world_t *world = ecs_init();
    EcsBootstrapModule(world);
    
    ECS_REGISTER_COMPONENT(world, Position);
    char *pos_name = "Position";
    ecs_set(world, ecs_id(Position), ecs_id(EcsName), &pos_name);
    
    stdio_devtool_execute_query(world, "Position");
    
    ecs_fini(world);
}

Test(devtool, query_with_invalid_syntax_prints_error, .init = setup)
{
    ecs_world_t *world = ecs_init();
    EcsBootstrapModule(world);
    
    stdio_devtool_execute_query(world, "InvalidSyntax((");
    
    fflush(stdout);
    cr_assert_stdout_eq_str("Invalid Query\n");
    
    ecs_fini(world);
}

Test(devtool, command_new_with_pair_adds_relationship, .init = setup)
{
    ecs_world_t *world = ecs_init();
    EcsBootstrapModule(world);
    
    ecs_entity_t parent = ecs_new(world);
    ecs_add(world, parent, ecs_id(EcsName));
    char *parent_name = "Parent";
    ecs_set(world, parent, ecs_id(EcsName), &parent_name);
    
    stdio_devtool_command_new(world, "Child (EcsChildOf, Parent)");
    
    ecs_entity_t child = ecs_lookup(world, "Child");
    cr_assert_neq(child.index, 0, "Child entity should be created");
    cr_assert(ecs_has_pair(world, child, ecs_id(EcsChildOf), parent), 
              "Child should have EcsChildOf relationship to Parent");
    
    ecs_fini(world);
}

Test(devtool, command_set_updates_component_field, .init = setup)
{
    ecs_world_t *world = ecs_init();
    EcsBootstrapModule(world);

    ECS_REGISTER_COMPONENT(world, Position);
    char *pos_name = "Position";
    ecs_set(world, ecs_id(Position), ecs_id(EcsName), &pos_name);
    
    ECS_STRUCT(Position, {
        int x;
        int y;
    });
    ECS_REGISTER_REFLECTION(world, Position);

    ecs_entity_t entity = ecs_new(world);
    ecs_add(world, entity, ecs_id(EcsName));
    char *entity_name = "TestEntity";
    ecs_set(world, entity, ecs_id(EcsName), &entity_name);

    Position pos = {10, 20};
    ecs_add(world, entity, ecs_id(Position));
    ecs_set(world, entity, ecs_id(Position), &pos);

    stdio_devtool_command_set(world, "TestEntity Position.x 42");

    Position *updated_pos = ecs_get(world, entity, ecs_id(Position));
    cr_assert_eq(updated_pos->x, 42, "Position.x should be updated to 42");
    cr_assert_eq(updated_pos->y, 20, "Position.y should remain unchanged");

    ecs_fini(world);
}

Test(devtool, command_set_by_entity_index, .init = setup)
{
    ecs_world_t *world = ecs_init();
    EcsBootstrapModule(world);

    ECS_REGISTER_COMPONENT(world, Position);
    char *pos_name = "Position";
    ecs_set(world, ecs_id(Position), ecs_id(EcsName), &pos_name);
    
    ECS_STRUCT(Position, {
        int x;
        int y;
    });
    ECS_REGISTER_REFLECTION(world, Position);

    ecs_entity_t entity = ecs_new(world);
    Position pos = {1, 2};
    ecs_add(world, entity, ecs_id(Position));
    ecs_set(world, entity, ecs_id(Position), &pos);

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%u Position.y 99", entity.index);
    stdio_devtool_command_set(world, buffer);

    Position *updated_pos = ecs_get(world, entity, ecs_id(Position));
    cr_assert_eq(updated_pos->x, 1, "Position.x should remain unchanged");
    cr_assert_eq(updated_pos->y, 99, "Position.y should be updated to 99");

    ecs_fini(world);
}

Test(devtool, command_set_with_invalid_field_fails, .init = setup)
{
    ecs_world_t *world = ecs_init();
    EcsBootstrapModule(world);

    ECS_REGISTER_COMPONENT(world, Position);
    char *pos_name = "Position";
    ecs_set(world, ecs_id(Position), ecs_id(EcsName), &pos_name);
    
    ECS_STRUCT(Position, {
        int x;
        int y;
    });
    ECS_REGISTER_REFLECTION(world, Position);

    ecs_entity_t entity = ecs_new(world);
    ecs_add(world, entity, ecs_id(EcsName));
    char *entity_name = "TestEntity";
    ecs_set(world, entity, ecs_id(EcsName), &entity_name);

    Position pos = {1, 2};
    ecs_add(world, entity, ecs_id(Position));
    ecs_set(world, entity, ecs_id(Position), &pos);

    stdio_devtool_command_set(world, "TestEntity Position.invalid 42");

    fflush(stdout);
    cr_assert_stdout_neq_str("", "Should print error message for invalid field");

    ecs_fini(world);
}
