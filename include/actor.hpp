#ifndef ACTOR_HPP
#define ACTOR_HPP

#include <string>
#include <vector>

#include "global.hpp"

#include "actor_data.hpp"
#include "sound.hpp"
#include "config.hpp"
#include "art.hpp"

class PropHandler;
class Inventory;

enum class ActorDied
{
    no,
    yes
};

class Actor
{
public:
    Actor();
    virtual ~Actor();

    PropHandler& prop_handler()
    {
        return *prop_handler_;
    }

    const PropHandler& prop_handler() const
    {
        return *prop_handler_;
    }

    //This is just a shortcut to the same function in the property handler
    bool has_prop(const PropId id) const;

    ActorDataT& data()
    {
        return *data_;
    }

    const ActorDataT& data() const
    {
        return *data_;
    }

    Inventory& inv()
    {
        return *inv_;
    }

    const Inventory& inv() const
    {
        return *inv_;
    }

    int ability(const AbilityId id, const bool is_affected_by_props) const;

    //NOTE: This function is not concerned with whether the parameter actor is
    //within FOV, or if the actor is actually hidden or not. It merely tests the
    //sneak skill of the actor, and various conditions such as light/dark. It
    //has no side effects - it merely does a randomized check.
    bool is_spotting_sneaking_actor(Actor& actor);

    void place(const P& pos_, ActorDataT& data);

    virtual void place_hook() {}

    ActorDied hit(int dmg,
                   const DmgType dmg_type,
                   const DmgMethod method = DmgMethod::END,
                   const AllowWound allow_wound = AllowWound::yes);

    ActorDied hit_spi(const int dmg, const Verbosity verbosity = Verbosity::verbose);

    bool restore_hp(const int hp_restored,
                    const bool is_allowed_above_max = false,
                    const Verbosity verbosity = Verbosity::verbose);

    bool restore_spi(const int spi_restored,
                     const bool is_allowed_above_max = false,
                     const Verbosity verbosity = Verbosity::verbose);

    void set_hp_and_spi_to_max();

    void change_max_hp(const int change,
                       const Verbosity verbosity = Verbosity::verbose);

    void change_max_spi(const int change,
                        const Verbosity verbosity = Verbosity::verbose);

    void die(const bool is_destroyed,
             const bool allow_gore,
             const bool allow_drop_items);

    //Used by Ghoul class and Ghoul monsters
    DidAction try_eat_corpse();
    void on_feed();

    void on_std_turn_common();

    virtual void act() {}

    virtual void move(Dir dir) = 0;

    virtual void on_actor_turn() {}
    virtual void on_std_turn() {}

    virtual void update_clr();

    void seen_actors(std::vector<Actor*>& out);

    void seen_foes(std::vector<Actor*>& out);

    ActorId id() const
    {
        return data_->id;
    }

    int hp() const
    {
        return hp_;
    }

    int spi() const
    {
        return spi_;
    }

    int hp_max(const bool with_modifiers) const;

    int spi_max() const
    {
        return spi_max_;
    }

    ActorSpeed speed() const;

    virtual std::string name_the() const
    {
        return data_->name_the;
    }

    virtual std::string name_a() const
    {
        return data_->name_a;
    }

    std::string corpse_name_a() const
    {
        return data_->corpse_name_a;
    }

    std::string corpse_name_the() const
    {
        return data_->corpse_name_the;
    }

    virtual std::string descr() const
    {
        return data_->descr;
    }

    bool is_humanoid() const
    {
        return data_->is_humanoid;
    }

    virtual char glyph() const
    {
        return glyph_;
    }

    virtual Clr clr()
    {
        return clr_;
    }

    virtual TileId tile() const
    {
        return tile_;
    }

    void add_light(bool light_map[map_w][map_h]) const;

    virtual void add_light_hook(bool light[map_w][map_h]) const
    {
        (void)light;
    }

    void teleport();

    bool is_alive() const
    {
        return state_ == ActorState::alive;
    }

    bool is_corpse() const
    {
        return state_ == ActorState::corpse;
    }

    ActorState state() const
    {
        return state_;
    }

    virtual bool is_leader_of(const Actor* const actor) const = 0;
    virtual bool is_actor_my_leader(const Actor* const actor) const = 0;

    bool is_player() const;

    P pos;

protected:
    //TODO: Try to get rid of these friend declarations
    friend class AbilityVals;
    friend class PropDiseased;
    friend class PropPossByZuul;
    friend class Trap;

    virtual void on_death() {}

    virtual std::string death_msg() const;

    virtual void on_hit(int& dmg,
                        const DmgType dmg_type,
                        const DmgMethod method,
                        const AllowWound allow_wound)
    {
        (void)dmg;
        (void)dmg_type;
        (void)method;
        (void)allow_wound;
    }

    virtual void mk_start_items() {}

    ActorState  state_;
    Clr clr_;
    char glyph_;
    TileId tile_;

    int hp_, hp_max_, spi_, spi_max_;

    P lair_pos_;

    PropHandler* prop_handler_;
    ActorDataT* data_;
    Inventory* inv_;
};

#endif
