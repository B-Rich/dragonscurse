#ifndef _World_H
#define _World_H

#include <list>
#include "phoboz/map.h"
#include "hazard.h"
#include "object.h"
#include "player.h"
#include "area.h"

class WorldDB;

class World {
public:
    World(Map *map, MediaDB *media, WorldDB *db, const char *music = 0);

    void set_lock(int x, int y) {
        m_lock_x = x;
        m_lock_y = y;
    }

    WorldDB* get_db() const { return m_db; }
    Map* get_map() const { return m_map; }
    const char* get_filename() const { return m_map->get_filename(); }

    Area* move(Player *player,
               int clip_x, int clip_y, int clip_w, int clip_h);
    void draw(Surface *dest, Player *player,
              int clip_x, int clip_y, int clip_w, int clip_h);

    Area* find_area(int id);

private:
    MediaDB *m_media;
    Map *m_map;
    int m_bg_color;
    int m_offset_x;
    int m_offset_y;
    int m_lock_x;
    int m_lock_y;
    Hazard *m_hazard;
    std::list<Object*> m_objects;
    WorldDB *m_db;
};

#endif

