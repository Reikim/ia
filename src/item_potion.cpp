#include "item_potion.hpp"

#include "init.hpp"
#include "properties.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "map.hpp"
#include "actor_mon.hpp"
#include "player_spells.hpp"
#include "item_scroll.hpp"
#include "game_time.hpp"
#include "audio.hpp"
#include "render.hpp"
#include "inventory.hpp"
#include "map_parsing.hpp"
#include "feature_rigid.hpp"
#include "item_factory.hpp"
#include "saving.hpp"
#include "dungeon_master.hpp"

ConsumeItem Potion::activate(Actor* const actor)
{
    ASSERT(actor);

    if (actor->prop_handler().allow_eat(Verbosity::verbose))
    {
        if (actor->is_player())
        {
            data_->is_tried = true;

            audio::play(SfxId::potion_quaff);

            if (data_->is_identified)
            {
                const std::string potion_name = name(ItemRefType::a, ItemRefInf::none);
                msg_log::add("I drink " + potion_name + "...");
            }
            else //Not identified
            {
                const std::string potion_name = name(ItemRefType::plain, ItemRefInf::none);
                msg_log::add("I drink an unknown " + potion_name + "...");
            }

            map::player->incr_shock(ShockLvl::heavy, ShockSrc::use_strange_item);
        }

        quaff_impl(*actor);

        if (map::player->is_alive())
        {
            game_time::tick();
        }

        return ConsumeItem::yes;
    }

    return ConsumeItem::no;
}

void Potion::identify(const Verbosity verbosity)
{
    if (!data_->is_identified)
    {
        data_->is_identified = true;

        if (verbosity == Verbosity::verbose)
        {
            const std::string name_after = name(ItemRefType::a, ItemRefInf::none);

            msg_log::add("I have identified " + name_after + ".");

            dungeon_master::add_history_event("Identified " + name_after + ".");

            give_xp_for_identify();
        }
    }
}

std::vector<std::string> Potion::descr() const
{
    if (data_->is_identified)
    {
        return
        {
            descr_identified()
        };
    }
    else //Not identified
    {
        return data_->base_descr;
    }
}

void Potion::on_collide(const P& pos, Actor* const actor)
{
    if (!map::cells[pos.x][pos.y].rigid->is_bottomless() || actor)
    {
        //Render and print message
        const bool player_see_cell = map::cells[pos.x][pos.y].is_seen_by_player;

        if (player_see_cell)
        {
            //TODO: Use standard animation
            render::draw_glyph('*', Panel::map, pos, data_->clr);

            if (actor)
            {
                if (actor->is_alive())
                {
                    msg_log::add("The potion shatters on " + actor->name_the() + ".");
                }
            }
            else //No actor here
            {
                Feature* const f = map::cells[pos.x][pos.y].rigid;
                msg_log::add("The potion shatters on " + f->name(Article::the) + ".");
            }
        }

        if (actor)
        {
            //If the blow from the bottle didn't kill the actor, apply what's inside
            if (actor->is_alive())
            {
                collide_hook(pos, actor);

                if (actor->is_alive() && !data_->is_identified && player_see_cell)
                {
                    //This did not identify the potion
                    msg_log::add("It had no apparent effect...");
                }
            }
        }
    }
}

std::string Potion::name_inf() const
{
    return (data_->is_tried && !data_->is_identified) ? "{Tried}" : "";
}

void PotionVitality::quaff_impl(Actor& actor)
{
    std::vector<PropId> props_can_heal =
    {
        PropId::blind,
        PropId::poisoned,
        PropId::infected,
        PropId::diseased,
        PropId::weakened,
        PropId::wound
    };

    for (PropId prop_id : props_can_heal)
    {
        actor.prop_handler().end_prop(prop_id);
    }

    //HP is always restored at least up to maximum hp, but can go beyond
    const int hp          = actor.hp();
    const int hp_max      = actor.hp_max(true);
    const int hp_restored = std::max(20, hp_max - hp);

    actor.restore_hp(hp_restored, true);

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionVitality::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionSpirit::quaff_impl(Actor& actor)
{
    //SPI is always restored at least up to maximum spi, but can go beyond
    const int spi           = actor.spi();
    const int spi_max       = actor.spi_max();
    const int spi_restored  = std::max(10, spi_max - spi);

    actor.restore_spi(spi_restored, true);

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionSpirit::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionBlindness::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new PropBlind(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionBlindness::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionParal::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new PropParalyzed(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionParal::collide_hook(const P& pos, Actor* const actor)
{

    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionDisease::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new PropDiseased(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionConf::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new PropConfused(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionConf::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionFortitude::quaff_impl(Actor& actor)
{
    PropHandler& prop_handler = actor.prop_handler();

    PropRFear* const rFear = new PropRFear(PropTurns::std);

    const int nr_turns_left = rFear->nr_turns_left();

    PropRConf* const   rConf   = new PropRConf(PropTurns::specific,  nr_turns_left);
    PropRSleep* const  rSleep  = new PropRSleep(PropTurns::specific, nr_turns_left);

    prop_handler.try_add(rFear);
    prop_handler.try_add(rConf);
    prop_handler.try_add(rSleep);

    prop_handler.end_prop(PropId::frenzied);

    //Remove a random insanity symptom if this is the player
    if (actor.is_player())
    {
        const std::vector<const InsSympt*> sympts = insanity::active_sympts();

        if (!sympts.empty())
        {
            const size_t        idx = rnd::range(0, sympts.size() - 1);
            const InsSymptId  id  = sympts[idx]->id();

            insanity::end_sympt(id);
        }

        map::player->restore_shock(999, false);

        msg_log::add("I feel more at ease.");
    }

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionFortitude::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionPoison::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new PropPoisoned(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionPoison::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionRFire::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new PropRFire(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionRFire::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionCuring::quaff_impl(Actor& actor)
{
    std::vector<PropId> props_can_heal =
    {
        PropId::blind,
        PropId::poisoned,
        PropId::infected,
        PropId::diseased,
        PropId::weakened,
    };

    bool is_noticable = false;

    for (PropId prop_id : props_can_heal)
    {
        if (actor.prop_handler().end_prop(prop_id))
        {
            is_noticable = true;
        }
    }

    if (actor.restore_hp(3, false /*Not allowed above max*/))
    {
        is_noticable = true;
    }

    if (!is_noticable && actor.is_player())
    {
        msg_log::add("I feel fine.");

        is_noticable = true;
    }

    if (is_noticable && map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionCuring::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionRElec::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new PropRElec(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionRElec::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionRAcid::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new PropRAcid(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionRAcid::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionInsight::quaff_impl(Actor& actor)
{
    (void)actor;

    auto& inv = map::player->inv();

    std::vector<Item*> identify_bucket;

    auto& slots = inv.slots_;

    for (InvSlot& slot : slots)
    {
        Item* const item = slot.item;

        if (item)
        {
            const ItemDataT& d = item->data();

            if (!d.is_identified)
            {
                identify_bucket.push_back(item);
            }
        }
    }

    std::vector<Item*>& general = inv.backpack_;

    for (Item* item : general)
    {
        if (item->id() != ItemId::potion_insight)
        {
            const ItemDataT& d = item->data();

            if (!d.is_identified)
            {
                identify_bucket.push_back(item);
            }
        }
    }

    if (identify_bucket.empty())
    {
        //No items to identify, give XP if Insight potion is identified
        if (data_->is_identified)
        {
            give_xp_for_identify(Verbosity::verbose);
        }
    }
    else //There are items to identify
    {
        Item* const item = rnd::element(identify_bucket);

        const std::string item_name_before =
            item->name(ItemRefType::a, ItemRefInf::none);

        msg_log::add("I gain intuitions about " + item_name_before + "...",
                     clr_white,
                     false,
                     MorePromptOnMsg::yes);

        item->identify(Verbosity::verbose);

        const std::string item_name_after =
            item->name(ItemRefType::a, ItemRefInf::none);
    }

    identify(Verbosity::verbose);
}

void PotionClairv::quaff_impl(Actor& actor)
{
    if (actor.is_player())
    {
        msg_log::add("I see far and wide!");

        std::vector<P> anim_cells;
        anim_cells.clear();

        bool blocked[map_w][map_h];
        map_parse::run(cell_check::BlocksLos(), blocked);

        for (int x = 0; x < map_w; ++x)
        {
            for (int y = 0; y < map_h; ++y)
            {
                Cell& cell = map::cells[x][y];

                if (!blocked[x][y] && !cell.is_dark)
                {
                    cell.is_explored = true;
                    cell.is_seen_by_player = true;
                    anim_cells.push_back(P(x, y));
                }
            }
        }

        render::draw_map_state(UpdateScreen::no);

        map::player->update_fov();

        render::draw_blast_at_cells(anim_cells, clr_white);

        render::draw_map_state(UpdateScreen::yes);
    }

    identify(Verbosity::verbose);
}

void PotionDescent::quaff_impl(Actor& actor)
{
    (void)actor;

    if (map::dlvl < dlvl_last - 1)
    {
        map::player->prop_handler().try_add(new PropDescend(PropTurns::std));
    }
    else
    {
        msg_log::add("I feel a faint sinking sensation, but it quickly disappears...");
    }

    identify(Verbosity::verbose);
}

void PotionInvis::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new PropInvisible(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionInvis::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionSeeInvis::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new PropSeeInvis(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionSeeInvis::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

namespace potion_handling
{

namespace
{

std::vector<PotionLook> potion_looks_;

} //namespace

void init()
{
    TRACE_FUNC_BEGIN;

    //Init possible potion colors and fake names
    potion_looks_.clear();
    potion_looks_.push_back(PotionLook {"Golden",   "a Golden",   clr_yellow});
    potion_looks_.push_back(PotionLook {"Yellow",   "a Yellow",   clr_yellow});
    potion_looks_.push_back(PotionLook {"Dark",     "a Dark",     clr_gray});
    potion_looks_.push_back(PotionLook {"Black",    "a Black",    clr_gray});
    potion_looks_.push_back(PotionLook {"Oily",     "an Oily",    clr_gray});
    potion_looks_.push_back(PotionLook {"Smoky",    "a Smoky",    clr_white});
    potion_looks_.push_back(PotionLook {"Slimy",    "a Slimy",    clr_green});
    potion_looks_.push_back(PotionLook {"Green",    "a Green",    clr_green_lgt});
    potion_looks_.push_back(PotionLook {"Fiery",    "a Fiery",    clr_red_lgt});
    potion_looks_.push_back(PotionLook {"Murky",    "a Murky",    clr_brown_drk});
    potion_looks_.push_back(PotionLook {"Muddy",    "a Muddy",    clr_brown});
    potion_looks_.push_back(PotionLook {"Violet",   "a Violet",   clr_violet});
    potion_looks_.push_back(PotionLook {"Orange",   "an Orange",  clr_orange});
    potion_looks_.push_back(PotionLook {"Watery",   "a Watery",   clr_blue_lgt});
    potion_looks_.push_back(PotionLook {"Metallic", "a Metallic", clr_gray});
    potion_looks_.push_back(PotionLook {"Clear",    "a Clear",    clr_white_high});
    potion_looks_.push_back(PotionLook {"Misty",    "a Misty",    clr_white_high});
    potion_looks_.push_back(PotionLook {"Bloody",   "a Bloody",   clr_red});
    potion_looks_.push_back(PotionLook {"Magenta",  "a Magenta",  clr_magenta});
    potion_looks_.push_back(PotionLook {"Clotted",  "a Clotted",  clr_green});
    potion_looks_.push_back(PotionLook {"Moldy",    "a Moldy",    clr_brown});
    potion_looks_.push_back(PotionLook {"Frothy",   "a Frothy",   clr_white});

    for (auto& d : item_data::data)
    {
        if (d.type == ItemType::potion)
        {
            //Color and false name
            const size_t idx = rnd::range(0, potion_looks_.size() - 1);

            PotionLook& look = potion_looks_[idx];

            d.base_name_un_id.names[int(ItemRefType::plain)]   = look.name_plain + " Potion";
            d.base_name_un_id.names[int(ItemRefType::plural)]  = look.name_plain + " Potions";
            d.base_name_un_id.names[int(ItemRefType::a)]       = look.name_a     + " Potion";
            d.clr = look.clr;

            potion_looks_.erase(potion_looks_.begin() + idx);

            //True name
            const Potion* const potion =
                static_cast<const Potion*>(item_factory::mk(d.id, 1));

            const std::string real_type_name = potion->real_name();

            delete potion;

            const std::string real_name        = "Potion of "    + real_type_name;
            const std::string real_name_plural = "Potions of "   + real_type_name;
            const std::string real_name_a      = "a Potion of "  + real_type_name;

            d.base_name.names[int(ItemRefType::plain)]  = real_name;
            d.base_name.names[int(ItemRefType::plural)] = real_name_plural;
            d.base_name.names[int(ItemRefType::a)]      = real_name_a;
        }
    }

    TRACE_FUNC_END;
}

void save()
{
    for (int i = 0; i < int(ItemId::END); ++i)
    {
        ItemDataT& d = item_data::data[i];

        if (d.type == ItemType::potion)
        {
            saving::put_str(d.base_name_un_id.names[int(ItemRefType::plain)]);
            saving::put_str(d.base_name_un_id.names[int(ItemRefType::plural)]);
            saving::put_str(d.base_name_un_id.names[int(ItemRefType::a)]);

            saving::put_int(d.clr.r);
            saving::put_int(d.clr.g);
            saving::put_int(d.clr.b);
        }
    }
}

void load()
{
    for (int i = 0; i < int(ItemId::END); ++i)
    {
        ItemDataT& d = item_data::data[i];

        if (d.type == ItemType::potion)
        {
            d.base_name_un_id.names[int(ItemRefType::plain)]  = saving::get_str();
            d.base_name_un_id.names[int(ItemRefType::plural)] = saving::get_str();
            d.base_name_un_id.names[int(ItemRefType::a)]      = saving::get_str();

            d.clr.r = saving::get_int();
            d.clr.g = saving::get_int();
            d.clr.b = saving::get_int();
        }
    }
}

} //potion_handling
