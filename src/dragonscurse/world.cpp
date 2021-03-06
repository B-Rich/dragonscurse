#include <iostream>
#include <vector>
#include <string>
#include "Tmx/Tmx.h"
#include "object_factory.h"
#include "player.h"
#include "monster.h"
#include "collectable.h"
#include "item.h"
#include "curse.h"
#include "chest.h"
#include "event.h"
#include "morph.h"
#include "statusbar.h"
#include "world_db.h"
#include "world.h"

World::World(Map *map, MediaDB *media, WorldDB *db, const char *music)
    : m_map(map), m_media(media), m_db(db)
{
    // Load attributes
    m_bg_color = m_map->get_numeric_property("bg_color");
    m_offset_x = m_map->get_numeric_property("offset_x");
    m_offset_y = m_map->get_numeric_property("offset_y");
    m_lock_x = m_map->get_numeric_property("lock_x");
    m_lock_y = m_map->get_numeric_property("lock_y");

    // Load hazard
    std::string hazard_name = m_map->get_literal_property("hazard");
    if (hazard_name == std::string("No such property!")) {
        m_hazard = 0;
    }
    else {
        m_hazard = new Hazard(hazard_name.c_str(), media);
    }

    // Play music
    if (music) {
        media->play_music(music);
    }
    else {
        media->play_music(m_map->get_literal_property("music").c_str());
    }

    int num_groups = map->get_num_object_groups();
    for (int object_group = 0; object_group < num_groups; object_group++) {
        const Tmx::ObjectGroup *group = map->get_object_group(object_group);
        std::string name = group->GetName();
        int num_objects = group->GetNumObjects();

        for (int i = 0; i < num_objects; i++) {
            Object *object = ObjectFactory::create_object(0, m_media, 0, 0, 0,
                                                          group->GetObject(i));
            if (object) {
                object->world_initialize(this);
                m_objects.push_back(object);
            }
            else {
                std::cerr << "Warning - Unable to load object: " << i
                          << std::endl;
            }
        }
    }
}

Area* World::move(Player *player,
                  int clip_x, int clip_y, int clip_w, int clip_h)
{
    Area *result = 0;

    if (!player->is_morphing()) {
        result = player->get_warp();
    }

    player->move(m_map);

    int window_width = clip_w - clip_x;
    if (m_lock_x) {
        m_map->set_x(m_lock_x, window_width);
    }
    else {
        int map_x = player->get_x() - window_width / 2 + m_offset_x;
        if (map_x < 0) {
            map_x = 0;
        }
        m_map->set_x(map_x, window_width);
    }

    int window_height = clip_h - clip_y;
    if (m_lock_y) {
        m_map->set_y(m_lock_y, window_height);
    }
    else {
        int map_y = player->get_y() - window_height / 2 + m_offset_y;
        if (map_y < 0) {
            map_y = 0;
        }
        m_map->set_y(map_y, window_height);
    }

    // Handle environment
    if (m_hazard) {
        player->check_hazard(m_map, m_hazard, m_db->get_status());
    }

    // Handle player world altering abilities
    if (m_db->get_status()->get_break_rock()) {
        int rock_x, rock_y;
        if (player->check_break_rock(&rock_x, &rock_y, m_map)) {
            int tile_x = rock_x / m_map->get_tile_width();
            int tile_y = rock_y / m_map->get_tile_height();
            ObjectInfo info;
            if (m_db->get_object_info(&info, tile_x, tile_y, get_filename())) {
                Object *object = ObjectFactory::create_object(&info, m_media);
                object->set_x(tile_x * m_map->get_tile_width());
                object->set_y(tile_y * m_map->get_tile_height());
                m_objects.push_back(object);
            }
        }
    }

    if (m_db->get_status()->get_create_rock()) {
        player->check_create_rock(m_map);
    }

    std::vector<Object*> perished;

    for (std::list<Object*>::iterator it = m_objects.begin();
         it != m_objects.end();
         ++it) {
        Object *object = *it;

        if (object->get_visible(m_map, clip_x, clip_y, clip_w, clip_h)) {
            Object::Type object_type = object->get_type();

            // Handle area objects
            if (object_type == Object::TypeArea) {
                Area *area = (Area *) object;

                if (area->is_over(player)) {

                    if (area->is_locked()) {

                        // Check if the player has the key
                        Status *status = m_db->get_status();
                        Item *item = status->check_item(area->get_data());
                        if (item) {
                            if (area->unlock(this, item)) {
                                status->remove_item(item);
                            }
                        }
                    }
                    else {
                        area->move(m_map);

                        if (area->is_open()) {
                            result = area;
                        }
                    }
                }
            }

            // Handle monster object
            else if (object_type == Object::TypeMonster) {
                Monster *monster = (Monster *) object;

                monster->set_always_visible(true);
                monster->move(m_map);
                monster->set_reference(player->get_front(),
                                       player->get_medium_y());
                if (monster->check_attack_collision(player) ||
                    monster->attack_object(player)) {
                    if (!monster->get_invisible()) {
                        player->set_hit(monster, m_db->get_status(), m_map);
                    }
                }
                if (player->attack_object(monster)) {
                    monster->set_hit(player, m_db->get_status(), m_map);
                }
                if (monster->get_action() == Actor::HitPerished) {
                    perished.push_back(monster);
                }
            }

            // Handle item objects
            else if (object_type == Object::TypeItem) {
                Item *item = (Item *) object;

                item->move(m_map);

                // Check if player picked up item
                if (item->get_reachable() &&
                    player->check_collision(item)) {
                    Status *status = m_db->get_status();
                    status->aquire_item(item);
                    item->aquire(this);
                    item->set_reused(true);
                    perished.push_back(item);
                }
            }

            // Handle collectable objects
            else if (object_type == Object::TypeCollectable) {
                Collectable *collectable = (Collectable *) object;

                collectable->move(m_map);

                // Check if player picked up the collectable
                if (collectable->get_reachable() &&
                    player->check_collision(collectable)) {
                    Status *status = m_db->get_status();
                    status->aquire_collectable(collectable);
                    collectable->aquire(this);
                    perished.push_back(collectable);
                }
            }

            // Handle event objects
            if (object_type == Object::TypeEvent) {
                Event *event = (Event *) object;

                event->move(m_map);

                // Check if player activated the event
                if (event->get_reachable() &&
                    player->check_collision(event)) {
                    m_objects.push_back(event->get_event_object());
                    perished.push_back(event);
                }
            }

            // Handle curse objects
            else if (object_type == Object::TypeCurse) {
                Curse *curse = (Curse *) object;

                curse->move(m_map);
                if (player->check_collision(curse)) {
                    std::string morph = std::string(player->get_name()) +
                                        std::string("_to_") +
                                        std::string(curse->get_player());
                    player->set_morph(new Morph(morph.c_str(),
                                                m_media,
                                                player->get_x(),
                                                player->get_y(),
                                                player->get_dir()));
                    player->set_warp(new Area(curse));
                    perished.push_back(curse);
                }
            }

            // Handle chest objects
            else if (object_type == Object::TypeChest) {
                Chest *chest = (Chest *) object;
                if (chest->is_open(player)) {
                    Object *released_object = chest->release_object();
                    if (released_object) {
                        released_object->set_x(chest->get_x());
                        released_object->set_y(chest->get_y());
                        released_object->set_reference(player->get_front(),
                                                       player->get_y());
                        m_objects.push_back(released_object);
                    }
                }
            }
        }
    }

    // Remove all perished objects
    for (int i = 0; i < perished.size(); i++) {

        // If monster drop object
        if (perished[i]->get_type() == Object::TypeMonster) {
            Monster *monster = (Monster *) perished[i];
            Object *released_object = monster->release_object();
            if (released_object) {
                released_object->set_x(monster->get_x());
                released_object->set_y(monster->get_y());
                released_object->set_reference(player->get_front(),
                                               player->get_y());
                m_objects.push_back(released_object);
            }
        }
        m_objects.remove(perished[i]);

        if (!perished[i]->get_reused()) {
            delete perished[i];
        }
    }

    return result;
}

void World::draw(Surface *dest, Player *player,
                 int clip_x, int clip_y, int clip_w, int clip_h)
{
    // Draw background
    Rect rect(clip_x, clip_y, clip_w, clip_h);
    Color col(m_bg_color);
    dest->fill_rect(&rect, &col);

    int num_layers = m_map->get_num_layers();

    if (num_layers > 0) {
        m_map->draw_layer(dest, clip_x, clip_y, clip_w, clip_h, 0);
    }

    // Draw objects
    for (std::list<Object*>::iterator it = m_objects.begin();
         it != m_objects.end();
         ++it) {
        Object *object = *it;
        object->draw(dest, m_map, clip_x, clip_y, clip_w, clip_h);
    }

    player->draw(dest, m_map, clip_x, clip_y, clip_w, clip_h);

    if (num_layers > 1) {
        m_map->draw_layer(dest, clip_x, clip_y, clip_w, clip_h, 1);
    }

}

Area* World::find_area(int id)
{
    Area *result = 0;

    for (std::list<Object*>::iterator it = m_objects.begin();
         it != m_objects.end();
         ++it) {
        Object *object = *it;

        if (object->get_type() == Object::TypeArea) {
            Area *area = (Area *) object;
            if (area->get_type() == Area::TypeMap) {
                if (area->get_attribute("id") == id) {
                    result = area;
                }
            }
        }
    }

    return result;
}

