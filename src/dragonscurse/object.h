#ifndef _Object_H
#define _Object_H

#include <string>
#include <vector>
#include <map>
#include "tinyxml.h"
#include "phoboz/media_db.h"
#include "phoboz/map.h"

class World;

// Hidden
struct CollisionParts;

class Object {
public:
    enum Type {
        TypeNone,
        TypePlayer,
        TypeMonster,
        TypeBullet,
        TypeItem,
        TypeCollectable,
        TypeCurse,
        TypeMorph,
        TypeArea,
        TypeChest,
        TypeEvent
    };

    enum Direction { None, Keep, Right, Left, Up, Down };

    enum VerticalDirection { VerticalNone, VerticalUp, VerticalDown };

    enum HorizontalDirection {
        HorizontalNone,
        HorizontalForward,
        HorizontalBackward
    };

    enum AnimDirection { AnimUp, AnimDown };

    static void set_prefix(const char* prefix) {
        m_prefix = std::string(prefix);
    }
    static const char* get_prefix() { return m_prefix.c_str(); }

    Object(Type type, int x = 0, int y = 0, Direction dir = None)
        : m_x(x), m_y(y), m_dx(0), m_dy(0), m_frame(0),
          m_dir(dir), m_loaded(false), m_type(type),
          m_always_visible(false), m_sprite_hidden(false),
          m_reused(false),
          m_xref(0), m_yref(0) { }

    ~Object();
    bool load(const char *fn, MediaDB *media);
    bool get_loaded() const { return m_loaded; }
    virtual void initialize() { }

    // Only called when objects are initialized from world
    virtual void world_initialize(World *world) { }

    Type get_type() const { return m_type; }
    int get_x() const { return m_x; }
    int get_y() const { return m_y; }
    Direction get_dir() const { return m_dir; }
    int get_frame() const { return m_frame; }
    const Sprite* get_sprite() const { return m_spr; }
    int get_image_width() const { return m_spr->get_image_width(); }
    int get_image_height() const { return m_spr->get_image_height(); }
    virtual Direction get_reference() const;
    int get_front() const;
    int get_bottom() const;
    int get_medium_y() const;
    int get_top() const;
    bool get_reused() { return m_reused; }
    int get_left() const { return m_x + get_attribute("left"); }
    int get_right() const { return m_x + get_attribute("right"); }

    virtual void set_dir(Direction dir) { }
    void set_x(int value) { m_x = value; }
    void set_y(int value) { m_y = value + get_attribute("handle_y"); }
    void set_reference(int x, int y) { m_xref = x; m_yref = y; }

    void set_reused(bool reused) { m_reused = reused; }
    void set_always_visible(bool value) { m_always_visible = value; }
    void set_sprite_hidden(bool value) { m_sprite_hidden = value; }

    const char* get_filename() const { return m_fn.c_str(); }
    const char* get_name() const { return m_name.c_str(); }
    int  get_attribute(const char *name) const;
    void set_attribute(const char *name, int value) {
        m_attributes[std::string(name)] = value;
    }
    const char* get_string(const char *name, int index = 0) const;
    bool string_exists(const char *name, const char *check) const;
    void set_string(const char *name, const char *value) {
        m_strings.insert(std::pair<std::string, std::string>(name, value));
    }
    virtual bool get_visible(Map *map, int clip_x, int clip_y,
                             int clip_w, int clip_h) const;
    virtual bool check_collision(Object *object) const {
        return m_spr->check_collision(m_frame, m_x, m_y,
                                      object->m_spr, object->m_frame,
                                      object->m_x, object->m_y);
    }
    bool check_weak_collision(const Object *object) const;
    bool check_weak_collision(const Object *object,
                              int start_x1, int start_y1,
                              int end_x1, int end_y1) const;
    virtual bool check_shielded_collision(const Object *object) const;
    bool check_attack_collision(const Object *object) const;
    bool check_attack_collision(int *x, int *y, Map *map,
                                int start, int end) const;

    virtual void move(Map *map) = 0;
    virtual void draw(Surface *dest, Map *map,
                      int clip_x, int clip_y, int clip_w, int clip_h);

protected:
    bool check_collision(int x, int y, Map *map,
                         int start = 0, int end = 0) const;
    bool check_center(Map *map, int start = 0, int end = 0);
    bool check_below(Map *map);
    bool check_below(Map *map, int start, int end);
    int check_below(Map *map, int len, int start = 0, int end = 0);
    bool check_ahead(Map *map);
    bool check_ahead(Map *map, int start, int end);
    int check_ahead(Map *map, int len, int start, int end);
    int check_right(Map *map, int len, int start = 0, int end = 0);
    int check_left(Map *map, int len, int start = 0, int end = 0);
    bool check_behind(Map *map);
    bool check_behind(Map *map, int start, int end);
    bool check_above(Map *map);
    bool check_above(Map *map, int start, int end);
    int check_above(Map *map, int len, int start = 0, int end = 0);

    std::map<std::string, int> m_attributes;
    std::multimap<std::string, std::string> m_strings;
    std::vector<CollisionParts*> m_weak_parts;
    std::vector<CollisionParts*> m_shielded_parts;
    std::vector<CollisionParts*> m_attack_parts;

    int m_x, m_y;
    int m_dx, m_dy;
    int m_frame;
    Direction m_dir;
    int m_xref, m_yref;
    MediaDB *m_media;

private:
    bool load_object_attributes(TiXmlElement *elmt);
    void load_strings(TiXmlElement *elmt);
    void load_attributes(TiXmlElement *elmt);
    bool load_nodes(TiXmlNode *node);
    CollisionParts* find_collision_parts(std::vector<CollisionParts*> v) const;

    static std::string m_prefix;
    std::string m_fn;
    std::string m_name;
    bool m_loaded;
    Type m_type;
    bool m_always_visible;
    bool m_sprite_hidden;
    bool m_reused;
    Sprite *m_spr;
};

#endif

