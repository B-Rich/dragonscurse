#include <string.h>
#include <string>
#include "SDL.h"
#include "statusbar.h"

int Statusbar::c_height = 40;

Statusbar::Statusbar(Status *status, MediaDB *media)
    : m_status(status), m_media(media)
{
    int max_hearts = status->get_max_hearts();
    m_hearts = new Heart*[max_hearts];
    for (int i = 0; i < max_hearts; i++) {
        m_hearts[i] = new Heart(media);
    }

    m_gold_label = new Text("Wonderfull_18", media);
    m_gold_label->add_line("Gold");

    m_gold_text = new Text("Wonderfull_18", media);
    m_gold_text->add_line("000000");
}

Statusbar::~Statusbar()
{
    for (int i = 0; i < m_status->get_max_hearts(); i++) {
        delete m_hearts[i];
    }
}

void Statusbar::draw(SDL_Surface *surface, int screen_width, int screen_height)
{
    SDL_Rect dest_rect;

    dest_rect.x = 0;
    dest_rect.y = 0;
    dest_rect.w = screen_width;
    dest_rect.h = c_height - 2;
    SDL_FillRect(surface, &dest_rect, 0x54545454);

    dest_rect.x = 0;
    dest_rect.y = c_height - 2;
    dest_rect.w = screen_width;
    dest_rect.h = 2;
    SDL_FillRect(surface, &dest_rect, 0x33333333);

    int w = m_hearts[0]->get_width();
    int num_full = m_status->get_hp() / Heart::get_hp_per_heart();
    int partial = m_status->get_hp() % Heart::get_hp_per_heart();

    for (int i = 0; i < m_status->get_hearts(); i++) {
        if (i < num_full) {
            m_hearts[i]->set_full();
        }
        else if (i == num_full && partial > 0) {
            m_hearts[i]->set_hp(partial);
        }
        else {
            m_hearts[i]->set_empty();
        }
        m_hearts[i]->draw(surface, 4 + i * (w + 4), 4,
                          0, 0, screen_width, screen_height);
    }

    static char str[6];
    sprintf(str, "%06d", m_status->get_gold());
    m_gold_text->replace_line(str);
    m_gold_label->draw(surface, screen_width - 120, 0,
                       0, 0, screen_width, screen_height);
    m_gold_text->draw(surface, screen_width - 80, c_height - 20,
                      0, 0, screen_width, screen_height);
}

