#include "postmortem.hpp"

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "init.hpp"
#include "render.hpp"
#include "input.hpp"
#include "actor_player.hpp"
#include "dungeon_master.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "menu_input.hpp"
#include "highscore.hpp"
#include "player_bon.hpp"
#include "text_format.hpp"
#include "feature_rigid.hpp"
#include "saving.hpp"

namespace postmortem
{

namespace
{

void mk_info_lines(std::vector<StrAndClr>& out)
{
    TRACE_FUNC_BEGIN;

    const Clr clr_heading  = clr_white_high;
    const Clr clr_info     = clr_white;

    const std::string offset            = "   ";
    const std::string bullet_point_str  = offset + "* ";

    TRACE << "Finding number of killed monsters" << std::endl;
    std::vector<std::string> unique_killed_names;
    int nr_kills_tot_all_mon = 0;

    for (const auto& d : actor_data::data)
    {
        if (d.id != ActorId::player && d.nr_kills > 0)
        {
            nr_kills_tot_all_mon += d.nr_kills;

            if (d.is_unique)
            {
                unique_killed_names.push_back(d.name_a);
            }
        }
    }

    const HighscoreEntry* const score = highscore::final_score();

    ASSERT(score);

    out.push_back({map::player->name_a(), clr_heading});

    const int dlvl = score->dlvl();

    if (dlvl == 0)
    {
        out.push_back(
        {
            bullet_point_str + "Died before entering the dungeon",
            clr_info
        });
    }
    else //DLVL is at least 1
    {
        out.push_back(
        {
            bullet_point_str + "Explored to the depth of dungeon level " + to_str(dlvl),
            clr_info
        });

    }

    out.push_back(
    {
        bullet_point_str + "Was " + to_str(score->ins()) + "% insane",
        clr_info
    });

    out.push_back(
    {
        bullet_point_str + "Killed " + to_str(nr_kills_tot_all_mon) + " monsters",
        clr_info
    });

    out.push_back(
    {
        bullet_point_str + "Gained " + to_str(score->xp()) + " experience points",
        clr_info
    });

    out.push_back(
    {
        bullet_point_str + "Gained a score of " + to_str(score->score()),
        clr_info
    });

    const std::vector<const InsSympt*> sympts = insanity::active_sympts();

    if (!sympts.empty())
    {
        for (const InsSympt* const sympt : sympts)
        {
            const std::string sympt_descr = sympt->char_descr_msg();

            if (!sympt_descr.empty())
            {
                out.push_back({bullet_point_str + sympt_descr, clr_info});
            }
        }
    }

    out.push_back({"", clr_info});
    out.push_back({"Traits gained:", clr_heading});
    std::string traits_line = player_bon::all_picked_traits_titles_line();

    if (traits_line.empty())
    {
        out.push_back({bullet_point_str + "None", clr_info});
    }
    else
    {
        std::vector<std::string> abilities_lines;
        text_format::split(traits_line, 60, abilities_lines);

        for (std::string& str : abilities_lines)
        {
            out.push_back({offset + str, clr_info});
        }
    }

    out.push_back({"", clr_info});
    out.push_back({"Unique monsters killed:", clr_heading});

    if (unique_killed_names.empty())
    {
        out.push_back({bullet_point_str + "None", clr_info});
    }
    else
    {
        for (std::string& monster_name : unique_killed_names)
        {
            out.push_back({bullet_point_str + "" + monster_name, clr_info});
        }
    }

    out.push_back({"", clr_info});
    out.push_back({"History of " + map::player->name_the(), clr_heading});

    const std::vector<HistoryEvent>& events = dungeon_master::history();

    for (const auto& event : events)
    {
        std::string ev_str = to_str(event.turn);

        const int turn_str_max_w = 10;

        text_format::pad_before_to(ev_str, turn_str_max_w);

        ev_str += ": " + event.msg;

        out.push_back({offset + ev_str, clr_info});
    }

    out.push_back({"", clr_info});
    out.push_back({"Last messages:", clr_heading});
    const std::vector< std::vector<Msg> >& history = msg_log::history();

    int history_element = std::max(0, int(history.size()) - 20);

    for (size_t i = history_element; i < history.size(); ++i)
    {
        std::string row = "";

        for (size_t ii = 0; ii < history[i].size(); ii++)
        {
            std::string msg_str = "";
            history[i][ii].str_with_repeats(msg_str);
            row += msg_str + " ";
        }

        out.push_back({offset + row, clr_info});
    }

    out.push_back({"", clr_info});

    TRACE << "Drawing the final map" << std::endl;
    out.push_back({"The final moment:", clr_heading});

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                for (int dy = -1; dy <= 1; ++dy)
                {
                    if (map::is_pos_inside_map(P(x + dx, y + dy)))
                    {
                        const auto* const f = map::cells[x + dx][y + dy].rigid;

                        if (f->is_los_passable())
                        {
                            map::cells[x][y].is_seen_by_player = true;
                        }
                    }
                }
            }
        }
    }

    render::draw_map(); //To set the glyph array

    for (int y = 0; y < map_h; ++y)
    {
        std::string current_row = "";

        for (int x = 0; x < map_w; ++x)
        {
            if (P(x, y) == map::player->pos)
            {
                current_row.push_back('@');
            }
            else //Not player pos
            {
                const auto& wall_d          = feature_data::data(FeatureId::wall);
                const auto& rubble_high_d   = feature_data::data(FeatureId::rubble_high);
                const auto& statue_d        = feature_data::data(FeatureId::statue);

                auto& current_render_data = render::render_array[x][y];

                if (
                    current_render_data.glyph == wall_d.glyph ||
                    current_render_data.glyph == rubble_high_d.glyph)
                {
                    current_row.push_back('#');
                }
                else if (current_render_data.glyph == statue_d.glyph)
                {
                    current_row.push_back('M');
                }
                else //Not wall, rubble or statue
                {
                    current_row.push_back(current_render_data.glyph);
                }
            }
        }

        out.push_back({current_row, clr_info});
        current_row.clear();
    }

    TRACE_FUNC_END;
    TRACE_FUNC_END;
}

void render(const std::vector<StrAndClr>& lines, const int top_element)
{
    render::clear_screen();

    render::draw_info_scr_interface("Game summary",
                                    InfScreenType::scrolling);

    const int nr_lines_tot = int(lines.size());

    const int max_nr_lines_on_scr = screen_h - 2;

    int y_pos = 1;

    for (
        int i = top_element;
        i < nr_lines_tot && ((i - top_element) < max_nr_lines_on_scr);
        ++i)
    {
        render::draw_text(lines[i].str,
                          Panel::screen,
                          P(0, y_pos++),
                          lines[i].clr);
    }

    render::update_screen();
}

void run_info(const std::vector<StrAndClr>& lines)
{
    const int line_jump           = 3;
    const int max_nr_lines_on_scr = screen_h - 2;
    const int nr_lines_tot        = lines.size();

    int top_nr = 0;

    while (true)
    {
        render(lines, top_nr);

        const KeyData& d = input::input();

        if (d.sdl_key == SDLK_DOWN || d.key == '2' || d.key == 'j')
        {
            top_nr += line_jump;

            if (nr_lines_tot <= max_nr_lines_on_scr)
            {
                top_nr = 0;
            }
            else
            {
                top_nr = std::min(nr_lines_tot - max_nr_lines_on_scr, top_nr);
            }
        }
        else if (d.sdl_key == SDLK_UP || d.key == '8' || d.key == 'k')
        {
            top_nr = std::max(0, top_nr - line_jump);
        }
        else if (d.sdl_key == SDLK_SPACE || d.sdl_key == SDLK_ESCAPE)
        {
            break;
        }
    }
}

void mk_memorial_file(const std::vector<StrAndClr>& lines)
{
    const std::string time_stamp =
        dungeon_master::start_time().time_str(TimeType::second, false);

    const std::string file_name = map::player->name_a() + "_" + time_stamp + ".txt";

    const std::string file_path = "data/" + file_name;

    //Write memorial file
    std::ofstream file;
    file.open(file_path.data(), std::ios::trunc);

    for (const StrAndClr& line : lines)
    {
        file << line.str << std::endl;
    }

    file.close();

    render::draw_text("Wrote file: " + file_path, Panel::screen, P(1, 1), clr_white_high);
    render::update_screen();
}

void render_menu(const MenuBrowser& browser)
{
    render::cover_panel(Panel::screen);

    P pos(screen_w / 2, 10);

    //Draw options
    render::draw_text_center("Show game summary",
                             Panel::screen,
                             pos,
                             browser.is_at_idx(0) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_center("Write memorial file",
                             Panel::screen,
                             pos,
                             browser.is_at_idx(1) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_center("View High Scores",
                             Panel::screen,
                             pos,
                             browser.is_at_idx(2) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_center("View message log",
                             Panel::screen,
                             pos,
                             browser.is_at_idx(3) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_center("Return to main menu",
                             Panel::screen,
                             pos,
                             browser.is_at_idx(4) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_center("Quit the game",
                             Panel::screen,
                             pos,
                             browser.is_at_idx(5) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    if (config::is_tiles_mode())
    {
        render::draw_skull({10, 2});
    }

    render::draw_box({0, 0, screen_w - 1, screen_h - 1});

    render::update_screen();
}

} //namespace

void run(bool* const quit_game)
{
    std::vector<StrAndClr> lines;

    mk_info_lines(lines);

    MenuBrowser browser(6);

    render_menu(browser);

    while (true)
    {
        const MenuAction action = menu_input::action(browser);

        switch (action)
        {
        case MenuAction::esc:
        case MenuAction::space:
            break;

        case MenuAction::moved:
            render_menu(browser);
            break;

        case MenuAction::selected:
        case MenuAction::selected_shift:
            if (browser.is_at_idx(0))
            {
                run_info(lines);
                render_menu(browser);
            }
            else if (browser.is_at_idx(1))
            {
                mk_memorial_file(lines);
            }
            else if (browser.is_at_idx(2))
            {
                highscore::run_highscore_screen();
                render_menu(browser);
            }
            else if (browser.is_at_idx(3))
            {
                msg_log::display_history();
                render_menu(browser);
            }
            else if (browser.is_at_idx(4))
            {
                return;
            }
            else if (browser.is_at_idx(5))
            {
                *quit_game = true;
                return;
            }
            break;
        }
    }
}

} //Postmortem
