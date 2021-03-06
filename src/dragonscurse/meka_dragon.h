#ifndef _MekaDragon_H
#define _MekaDragon_H

#include <vector>
#include "phoboz/timer.h"
#include "monster.h"
#include "vector_bullet.h"

class MekaDragon : public Monster {
public:
    MekaDragon(const char *fn, MediaDB *media, int x, int y, Direction dir);

    virtual bool attack_object(Object *object);

    virtual bool get_visible(Map *map, int clip_x, int clip_y,
                             int clip_w, int clip_h) const;
    virtual void move(Map *map);
    virtual void draw(Surface *dest, Map *map,
                      int clip_x, int clip_y, int clip_w, int clip_h);

private:
    void fire();

    HorizontalDirection m_horizontal_dir;
    int m_bullet_index;
    std::vector<VectorBullet*> m_bullets;
    bool m_attack_now;
    Timer m_attack_timer;
    Timer m_idle_timer;
    Timer m_fire_timer;
};

#endif

