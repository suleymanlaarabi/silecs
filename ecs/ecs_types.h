#ifndef ECS_TYPE_H
    #define ECS_TYPE_H
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>
    #include <stdio.h>
    #include "datastructure/ecs_vec.h"
    #include "ecs_config.h"
    #include "ecs_vec_sort.h"

    #define ecs_type_sort ecs_vec_sort_u64

    #define ECS_NULL (ecs_entity_t) {0}

    #define ECS_COMPONENT_DEFINE(component) \
        ecs_component_desc_t ECS_##component##ID = { \
            .size = sizeof(component), \
            .entity = {0}, \
            .name = #component \
        };

    #define ECS_TAG(name) typedef struct {} name;

    #define ECS_TAG1 ECS_TAG
    #define ECS_TAG2(one, two) ECS_TAG(one); ECS_TAG(two);
    #define ECS_TAG3(one, two, three) ECS_TAG(one); ECS_TAG(two); ECS_TAG(three);
    #define ECS_TAG4(one, two, three, four) ECS_TAG(one); ECS_TAG(two); ECS_TAG(three); ECS_TAG(four);
    #define ECS_TAG5(one, two, three, four, five) ECS_TAG(one); ECS_TAG(two); ECS_TAG(three); ECS_TAG(four); ECS_TAG(five);
    #define ECS_TAG6(one, two, three, four, five, six) ECS_TAG(one); ECS_TAG(two); ECS_TAG(three); ECS_TAG(four); ECS_TAG(five); ECS_TAG(six);
    #define ECS_TAG7(one, two, three, four, five, six, seven) ECS_TAG(one); ECS_TAG(two); ECS_TAG(three); ECS_TAG(four); ECS_TAG(five); ECS_TAG(six); ECS_TAG(seven);
    #define ECS_TAG8(one, two, three, four, five, six, seven, eight) ECS_TAG(one); ECS_TAG(two); ECS_TAG(three); ECS_TAG(four); ECS_TAG(five); ECS_TAG(six); ECS_TAG(seven); ECS_TAG(eight);
    #define ECS_TAG9(one, two, three, four, five, six, seven, eight, nine) ECS_TAG(one); ECS_TAG(two); ECS_TAG(three); ECS_TAG(four); ECS_TAG(five); ECS_TAG(six); ECS_TAG(seven); ECS_TAG(eight); ECS_TAG(nine);
    #define ECS_TAG10(one, two, three, four, five, six, seven, eight, nine, ten) ECS_TAG(one); ECS_TAG(two); ECS_TAG(three); ECS_TAG(four); ECS_TAG(five); ECS_TAG(six); ECS_TAG(seven); ECS_TAG(eight); ECS_TAG(nine); ECS_TAG(ten);

    #define GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,NAME,...) NAME
    #define ECS_TAGS(...) GET_MACRO(__VA_ARGS__, ECS_TAG10, ECS_TAG9, ECS_TAG8, ECS_TAG7, ECS_TAG6, ECS_TAG5, ECS_TAG4, ECS_TAG3, ECS_TAG2, ECS_TAG1)(__VA_ARGS__)

    #define _ecs_id_name(component) \
        ECS_##component##ID

    #define ECS_REGISTER_COMPONENT(world, component) \
        _ecs_id_name(component).entity = ecs_new(world);  \
        ecs_set_component_meta(world, _ecs_id_name(component).entity, sizeof(component)); \
        ecs_add(world, ecs_id(component), ecs_id(EcsName)); \
        char *ECS_##component##Name = #component; \
        ecs_set(world, ecs_id(component), ecs_id(EcsName), &ECS_##component##Name); \
        ecs_add(world, ecs_id(component), ecs_id(EcsComponent));

    #define ECS_COMPONENT_DECLARE(component) \
        extern ecs_component_desc_t ECS_##component##ID;

    #define ecs_id(component) ECS_##component##ID.entity
    #define ecs_relation ecs_make_pair
    #define ChildOf(target) ecs_make_pair(ecs_id(EcsChildOf), target)

typedef struct ecs_world_t ecs_world_t;

typedef union {
    struct {
        uint32_t index;
        uint16_t gen;
        uint16_t flags;
    };
    struct {
        uint32_t relation;
        uint16_t target;
    } relation;
    uint64_t value;
} ecs_entity_t;

typedef struct {
    ecs_entity_t entity;
    size_t size;
    char *name;
} ecs_component_desc_t;

typedef struct {
    ecs_entity_t component;
    void *value;
} ecs_value_t;

typedef ecs_vec_t ecs_type_t;
typedef size_t ecs_size_t;
typedef uint64_t ecs_component_id_t;

ECS_INLINE
ecs_type_t ecs_type_from_other(const ecs_type_t *other) {
    ecs_type_t type;

    ecs_vec_copy(other, &type);
    return type;
}

ECS_INLINE
ecs_type_t ecs_type_from_other_add(const ecs_type_t *other, ecs_entity_t component) {
    ecs_type_t type;

    ecs_vec_copy(other, &type);
    ecs_vec_push(&type, &component);
    ecs_vec_sort_last_u64(&type);
    return type;
}

ECS_INLINE
ecs_type_t *ecs_type_from_other_add_temp(const ecs_type_t *other, ecs_entity_t component) {
    static ecs_type_t type = {0};

    if (type.data == NULL) {
        ecs_vec_init(&type, sizeof(ecs_entity_t));
    }
    ecs_vec_copy_already_init(other, &type);
    ecs_vec_push(&type, &component);
    ecs_vec_sort_last_u64(&type);
    return &type;
}

ECS_INLINE
ecs_type_t ecs_type_from_other_remove(const ecs_type_t *other, ecs_entity_t component) {
    ecs_type_t type;

    ecs_vec_copy(other, &type);
    for (size_t i = 0; i < type.count; i++) {
        if (((uint64_t *) type.data)[i] == component.value) {
            ecs_vec_remove_ordered(&type, i);
            break;
        }
    }
    return type;
}

ECS_INLINE
ecs_type_t *ecs_type_from_other_remove_temp(const ecs_type_t *other, ecs_entity_t component) {
    static ecs_type_t type = {0};

    if (type.data == NULL) {
        ecs_vec_init(&type, sizeof(ecs_entity_t));
    }
    ecs_vec_copy_already_init(other, &type);
    for (size_t i = 0; i < type.count; i++) {
        if (((uint64_t *) type.data)[i] == component.value) {
            ecs_vec_remove_ordered(&type, i);
            break;
        }
    }
    return &type;
}

ECS_INLINE
bool ecs_type_has(const ecs_type_t *type, ecs_entity_t component) {
    const uint64_t *ids = (const uint64_t*)type->data;
    for (size_t i = 0; i < type->count; i++) {
        if (ids[i] == component.value) {
            return true;
        }
    }
    return false;
}

ECS_INLINE
void ecs_type_add(ecs_type_t *type, ecs_entity_t component) {
    ecs_vec_push(type, &component);
    ecs_vec_sort_last_u64(type);
}

static inline bool ecs_vec_is_subset(const ecs_vec_t *subset, const ecs_vec_t *set) {
    if (subset->count > set->count) return false;

    const uint64_t *sub_ids = (const uint64_t*)subset->data;
    const uint64_t *set_ids = (const uint64_t*)set->data;

    for (size_t i = 0; i < subset->count; i++) {
        uint64_t id = sub_ids[i];
        bool found = false;
        for (size_t j = 0; j < set->count; j++) {
            if (set_ids[j] == id) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

#include <inttypes.h>

static inline void ecs_vec_print_type(const ecs_vec_t *type) {
    const uint64_t *ids = (const uint64_t*)type->data;

    printf("(");
    for (size_t i = 0; i < type->count; i++) {
        printf("%" PRIu64, ids[i]);
        if (i + 1 < type->count) {
            printf(", ");
        }
    }
    printf(")\n");
}

ECS_INLINE
void print_entity(ecs_entity_t e) {
    printf("Entity: %u, Generation: %u\n", e.index, e.gen);
}



#endif
