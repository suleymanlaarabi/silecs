#include "ecs_types.h"
#include <criterion/criterion.h>
#include <ecs_world.h>
#include <stdio.h>

typedef struct {
    int x, y;
} _Position;
typedef struct {
    int value;
} _Health;

ECS_COMPONENT_DEFINE(_Position);
ECS_COMPONENT_DEFINE(_Health);
ECS_TAG_DEFINE(_MainScene);

Test(world, init_creates_valid_world) {
    ecs_world_t *world = ecs_init();
    cr_assert_not_null(world, "ecs_init must return a valid pointer");
}

Test(world, new_generates_unique_entities) {
    ecs_world_t *world = ecs_init();
    ecs_entity_t e1 = ecs_new(world);
    ecs_entity_t e2 = ecs_new(world);
    cr_assert(e1.value != e2.value, "Two calls to ecs_new must return distinct entities");
}

Test(world, add_component_size_registers_component) {
    ecs_world_t *world = ecs_init();
    ecs_entity_t comp = ecs_new(world);
    ecs_set_component_meta(world, comp, sizeof(_Position));

    ecs_entity_t player = ecs_new(world);
    ecs_add(world, player, comp);
    cr_assert(ecs_has(world, player, comp),
              "The player should have the added component");
}

Test(world, insert_stores_and_retrieves_data) {
    ecs_world_t *world = ecs_init();
    ecs_entity_t player = ecs_new(world);
    ecs_entity_t position = ecs_new(world);

    ecs_set_component_meta(world, position, sizeof(_Position));
    ecs_add(world, player, position);

    _Position p = {10, 20};
    ecs_set(world, player, position, &p);

    _Position *stored = ecs_get(world, player, position);
    cr_assert_not_null(stored, "ecs_get must return a valid pointer");
    cr_expect_eq(stored->x, 10, "x should be 10");
    cr_expect_eq(stored->y, 20, "y should be 20");
}

Test(world, has_returns_false_for_missing_component) {
    ecs_world_t *world = ecs_init();
    ecs_entity_t player = ecs_new(world);
    ecs_entity_t position = ecs_new(world);

    ecs_set_component_meta(world, position, sizeof(_Position));

    cr_assert_not(ecs_has(world, player, position),
                  "ecs_has must return false if the player lacks the component");
}

Test(world, components_sorted_functional) {
    ecs_world_t *world = ecs_init();

    ecs_entity_t compPos = ecs_new(world);
    ecs_entity_t compHp  = ecs_new(world);

    ecs_set_component_meta(world, compPos, sizeof(_Position));
    ecs_set_component_meta(world, compHp,  sizeof(_Health));

    ecs_entity_t player = ecs_new(world);
    ecs_add(world, player, compHp);
    ecs_add(world, player, compPos);

    _Position p = {42, 24};
    _Health   h = {100};
    ecs_set(world, player, compPos, &p);
    ecs_set(world, player, compHp,  &h);

    _Position *gotP = ecs_get(world, player, compPos);
    _Health *gotH = ecs_get(world, player, compHp);

    cr_assert_not_null(gotP, "ecs_get must return _Position if sorting works");
    cr_assert_not_null(gotH, "ecs_get must return _Health if sorting works");
    cr_expect_eq(gotP->x, 42);
    cr_expect_eq(gotP->y, 24);
    cr_expect_eq(gotH->value, 100);
}

Test(world, insert_overwrites_existing_component_data) {
    ecs_world_t *world = ecs_init();
    ecs_entity_t player = ecs_new(world);
    ecs_entity_t position = ecs_new(world);

    ecs_set_component_meta(world, position, sizeof(_Position));
    ecs_add(world, player, position);

    _Position p1 = {1, 2};
    _Position p2 = {99, 100};
    ecs_set(world, player, position, &p1);
    ecs_set(world, player, position, &p2);

    _Position *stored = ecs_get(world, player, position);
    cr_assert_not_null(stored, "ecs_get must still return valid data after overwrite");
    cr_expect_eq(stored->x, 99, "x should be updated to 99");
    cr_expect_eq(stored->y, 100, "y should be updated to 100");
}

Test(world, remove_component) {
    ecs_world_t *world = ecs_init();
    ecs_entity_t player = ecs_new(world);
    ecs_entity_t position = ecs_new(world);

    ecs_set_component_meta(world, position, sizeof(_Position));
    ecs_add(world, player, position);

    _Position p1 = {1, 2};
    _Position p2 = {99, 100};
    ecs_set(world, player, position, &p1);
    ecs_set(world, player, position, &p2);

    cr_expect(ecs_has(world, player, position));

    ecs_remove(world, player, position);

    cr_expect(!ecs_has(world, player, position));
}

Test(world, remove_component_get_other) {
    ecs_world_t *world = ecs_init();
    ecs_entity_t player = ecs_new(world);

    ecs_entity_t position = ecs_new(world);
    ecs_entity_t health = ecs_new(world);

    ecs_set_component_meta(world, position, sizeof(_Position));
    ecs_set_component_meta(world, health, sizeof(_Health));

    ecs_add(world, player, position);
    ecs_add(world, player, health);

    _Position p1 = {1, 2};
    _Position p2 = {99, 100};
    _Health h1 = {100};
    ecs_set(world, player, position, &p1);
    ecs_set(world, player, position, &p2);
    ecs_set(world, player, health, &h1);

    cr_expect(ecs_has(world, player, position));

    ecs_remove(world, player, position);

    cr_expect(!ecs_has(world, player, position));

    _Health *read_health = ecs_get(world, player, health);
    cr_assert_not_null(read_health, "ecs_get must still return valid data after overwrite");
    cr_expect_eq(read_health->value, 100, "value should be updated to 100");
}

Test(world, relation) {
    ecs_world_t *world = ecs_init();

    ecs_entity_t scene = ecs_new(world);
    ecs_entity_t child_of = ecs_new(world);
    ecs_entity_t player = ecs_new(world);

    ecs_entity_t relationship = ecs_make_pair(child_of, player);

    ecs_add_pair(world, player, child_of, scene);
    cr_assert(ecs_is_pair(relationship));
    cr_assert(ecs_has(world, player, ecs_make_pair(child_of, scene)));
    cr_assert(ecs_has(world, player, ecs_make_pair(child_of, ecs_id(EcsWildcard))));
}

Test(relation, wildcard) {
    ecs_world_t *world = ecs_init();
    ECS_REGISTER_COMPONENT(world, _Position);
    ECS_REGISTER_COMPONENT(world, _Health);
    ECS_TAG_REGISTER(world, _MainScene);

    ecs_entity_t player = ecs_new(world);

    ecs_add_pair(world, player, ecs_id(EcsChildOf), ecs_id(_MainScene));

    cr_assert(ecs_has(world, player, ecs_make_pair(ecs_id(EcsChildOf), ecs_id(_MainScene))));
    cr_assert(ecs_has(world, player, ecs_make_pair(ecs_id(EcsChildOf), ecs_id(EcsWildcard))));

    ecs_remove_pair(world, player, ecs_id(EcsChildOf), ecs_id(_MainScene));

    cr_assert(!ecs_has(world, player, ecs_make_pair(ecs_id(EcsChildOf), ecs_id(_MainScene))));
    cr_assert(!ecs_has(world, player, ecs_make_pair(ecs_id(EcsChildOf), ecs_id(EcsWildcard))));
}

ecs_entity_t position;

void remove_added_position(ecs_world_t *world, ecs_entity_t entity) {
    ecs_remove(world, entity, position);
}
