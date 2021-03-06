#include <math.h>
#include "roller.h"

Roller::Roller(const char *fn, MediaDB *media, int x, int y,
               const Tmx::Polyline *polyline)
    : Monster(fn, media, x, y, Right),
      m_polyline(polyline),
      m_curr_point(0),
      m_reverse(false),
      m_was_hit(false),
      m_x0(x), m_y0(y)
{
    set_solid(false);
    set_recursive_hit(false);
}

bool Roller::set_hit(Object *object, Status *status, Map *map)
{
    if (!m_was_hit) {
        m_was_hit = true;
        get_speed(&m_old_dx, &m_old_dy);
        set_speed(0, 0);
    }

    return Monster::set_hit(object, status, map);
}

void Roller::next_point()
{
    if (!m_reverse) {
        if (++m_curr_point == m_polyline->GetNumPoints() - 1) {
            m_reverse = true;
        }
    }
    else {
        if (--m_curr_point == 0) {
            m_reverse = false;
        }
    }
}

void Roller::update_speed(Map *map)
{
    int x0 = m_polyline->GetPoint(m_curr_point).x + m_x0;
    int y0 = m_polyline->GetPoint(m_curr_point).y + m_y0;

    next_point();

    int x1 = m_polyline->GetPoint(m_curr_point).x + m_x0;
    int y1 = m_polyline->GetPoint(m_curr_point).y + m_y0;

    float dx = x1 - x0;
    float dy = y1 - y0;

    float len = sqrt(dx * dx + dy * dy);
    dx /= len;
    dy /= len;

    int speed = get_move_speed(map);
    set_speed(dx * speed, dy * speed);

    m_tx = x1;
    m_ty = y1;
}

void Roller::move(Map *map)
{
    int speed = 0;

    Body::move(map);

    switch(m_action) {
       case Still:
           if (m_was_hit) {
               set_speed(m_old_dx, m_old_dy);
               m_was_hit = false;
               set_action(Move);
           }
           else {
               update_speed(map);
               set_action(Move);
           }
           break;

       case Move:
           animate_move();
           speed = get_attribute("move_speed");
           if (abs(m_x - m_tx) <= speed && abs(m_y - m_ty) <= speed) {
               set_speed(0, 0);
               set_action(Still);
           }
           break;

       case Hit:
       case HitPerish:
           process_hit();
           break;

       default:
           break;
    }
}

