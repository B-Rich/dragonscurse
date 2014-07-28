#include "world.h"
#include "item.h"
#include "coin.h"
#include "curse.h"
#include "object_factory.h"
#include "world_db.h"
#include "monster.h"

Monster::Monster(const char *fn, MediaDB *media, int x, int y, Direction dir)
    : Actor(Object::TypeMonster, x, y, dir),
      m_invinsible(false), m_hit_ground(false)
{
    load(fn, media);
    m_curr_hp = get_attribute("hp");
    set_action(Still);
}

void Monster::world_initialize(World *world)
{
    Item *item = 0;
    int object_id = get_attribute("object_id");

    if (object_id) {
        WorldDB *db = world->get_db();
        ObjectInfo info;
        if (db->get_object_info(&info,
                                object_id, world->get_filename())) {
            Object *object = ObjectFactory::create_object(&info, m_media,
                                                          get_x(), get_y());
            if (object) {
                m_objects.push_back(object);
            }
        }
    }
}

bool Monster::set_hit(Object *object)
{
    bool result = false;

    if (!m_invinsible && m_hit == HitNone) {
        result = Actor::set_hit(object);

        if (result) {

            set_lock_direction(true);

            // Move backwards and upwards
            if (get_reference() == Right) {
                set_speed(-get_attribute("move_speed"), 0);
            }
            else {
                set_speed(get_attribute("move_speed"), 0);
            }

            // Reduce hp
            // TODO: Get attackers attack power
            m_curr_hp--;
            if (m_curr_hp <= 0) {
                set_perish();
            }
        }
    }

    return result;
}

Object* Monster::release_object()
{
    Object *object = 0;

    if (m_objects.size()) {
        object = m_objects.back();
        m_objects.pop_back();
    }
    else {
        const char *col = get_string("collectable");
        if (col) {
            object = ObjectFactory::create_object(col, m_media, "Collectable");
        }
    }

    return object;
}

void Monster::move(Map *map)
{
    set_ay(get_attribute("weight"));

    switch(m_action) {
        case Still:
            break;

        case Move:
            Body::move(map);
            if (get_fall()) {
                set_action(Fall);
            }
            break;

        case Fall:
            Body::move(map);
            if (!get_fall()) {
                m_hit_ground = true;
                set_vx(0);
                set_action(Still);
            }
            break;

        case Jump:
            Body::move(map);
            if (get_fall()) {
                set_action(Still);
            }
            break;

        case Hit:
            if (m_hit == HitPerish) {
                set_vx(0);
                if (m_perish_timer.expired(get_attribute("perish_time"))) {
                    m_hit = HitPerished;
                }
            }
            else if (m_hit_timer.expired(get_attribute("hit_time"))) {
                m_hit_timer.reset();
                set_vx(0);
                set_lock_direction(false);
                m_hit = HitNone;
                set_action(Still);
            }
            Body::move(map);
            break;

        default:
            break;
    }
}

