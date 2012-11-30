#include "CharacterInfo.h"

#include "Engine.h"
#include "PlayerBonuses.h"
#include "ActorPlayer.h"
#include "Render.h"
#include "TextFormatting.h"
#include "Input.h"

void CharacterInfo::makeLines() {
  lines.resize(0);

  const string offsetSpaces = "";
  const sf::Color colorHeader = clrCyanLight;

  lines.push_back(StringAndColor(" ", clrRedLight));

  lines.push_back(StringAndColor("Abilities gained", colorHeader));
  string abilitiesLine = "";
  bool isAnyBonusPicked = false;
  for(unsigned int i = 0; i < endOfPlayerBonuses; i++) {
    const PlayerBonuses_t bonus = static_cast<PlayerBonuses_t>(i);
    if(eng->playerBonusHandler->isBonusPicked(bonus)) {
      isAnyBonusPicked = true;
      const string currentTitle = eng->playerBonusHandler->getBonusTitle(bonus);
      lines.push_back(StringAndColor(offsetSpaces + currentTitle, clrRedLight));
      const string currentDescr = eng->playerBonusHandler->getBonusDescription(bonus);
      lines.push_back(StringAndColor(offsetSpaces + currentDescr, clrRed));
    }
  }
  if(isAnyBonusPicked == false) {
    lines.push_back(StringAndColor(offsetSpaces + "None", clrRedLight));
  }

  lines.push_back(StringAndColor(" ", clrRedLight));

  lines.push_back(StringAndColor("Mental conditions", colorHeader));
  const int NR_LINES_BEFORE_MENTAL = lines.size();
  if(eng->player->insanityPhobias[insanityPhobia_closedPlace])
    lines.push_back(StringAndColor(offsetSpaces + "Phobia of enclosed spaces", clrRedLight));
  if(eng->player->insanityPhobias[insanityPhobia_dog])
    lines.push_back(StringAndColor(offsetSpaces + "Phobia of dogs", clrRedLight));
  if(eng->player->insanityPhobias[insanityPhobia_rat])
    lines.push_back(StringAndColor(offsetSpaces + "Phobia of rats", clrRedLight));
  if(eng->player->insanityPhobias[insanityPhobia_undead])
    lines.push_back(StringAndColor(offsetSpaces + "Phobia of the dead", clrRedLight));
  if(eng->player->insanityPhobias[insanityPhobia_openPlace])
    lines.push_back(StringAndColor(offsetSpaces + "Phobia of open places", clrRedLight));
  if(eng->player->insanityPhobias[insanityPhobia_spider])
    lines.push_back(StringAndColor(offsetSpaces + "Phobia of spiders", clrRedLight));
  if(eng->player->insanityPhobias[insanityPhobia_deepPlaces])
    lines.push_back(StringAndColor(offsetSpaces + "Phobia of deep places", clrRedLight));

  if(eng->player->insanityCompulsions[insanityCompulsion_masochism])
    lines.push_back(StringAndColor(offsetSpaces + "Masochistic compulsion", clrRedLight));
  if(eng->player->insanityCompulsions[insanityCompulsion_sadism])
    lines.push_back(StringAndColor(offsetSpaces + "Sadistic compulsion", clrRedLight));
  const int NR_LINES_AFTER_MENTAL = lines.size();

  if(NR_LINES_BEFORE_MENTAL == NR_LINES_AFTER_MENTAL) {
    lines.push_back(StringAndColor(offsetSpaces + "No special symptoms", clrRedLight));
  }

  lines.push_back(StringAndColor(" ", clrRedLight));

  //TODO With the current implementation below, it's possible to know what each scroll, potion, etc
  //does, just by the order they are listed. If object knowledge should be included, the order they
  //are listed needs to be randomized
//    lines.push_back(StringAndColor("Object knowledge", colorHeader));
//    for(unsigned int i = 1; i < endOfItemDevNames; i++) {
//        const ItemDefinition* const d = eng->itemData->itemDefinitions[i];
//        if(d->abilityToIdentify != ability_empty) {
//            lines.push_back(StringAndColor(offsetSpaces + d->name.name, d->color));
//        }
//    }
//
//    lines.push_back(StringAndColor(" ", clrRedLight));
}

void CharacterInfo::drawInterface() {
  const string decorationLine(MAP_X_CELLS - 2, '-');

  eng->renderer->coverArea(renderArea_screen, coord(0, 1), MAP_X_CELLS, 2);
  eng->renderer->drawText(decorationLine, renderArea_screen, 1, 1, clrWhite);

  eng->renderer->drawText("Displaying character info", renderArea_screen, 3, 1, clrWhite);

  eng->renderer->drawText(decorationLine, renderArea_characterLines, 1, 1, clrWhite);

  eng->renderer->drawText(" [2/8, Down/Up] to navigate  [Space/Esc] to exit. ", renderArea_characterLines, 3, 1, clrWhite);
}

void CharacterInfo::run() {
  makeLines();

  eng->renderer->coverRenderArea(renderArea_screen);

  StringAndColor currentLine;

  int topElement = 0;
  int btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);

  drawInterface();

  int yCell = 2;
  for(int i = topElement; i <= btmElement; i++) {
    eng->renderer->drawText(lines.at(i).str , renderArea_screen, 1, yCell, lines.at(i).color);
    yCell++;
  }

  eng->renderer->updateWindow();

  //Read keys
  bool done = false;
  while(done == false) {
    const KeyboardReadReturnData& d = eng->input->readKeysUntilFound();

    if(d.key_ == '2' || d.sfmlKey_ == sf::Keyboard::Down) {
      topElement = max(0, min(topElement + static_cast<int>(MAP_Y_CELLS / 5), static_cast<int>(lines.size()) - static_cast<int>(MAP_Y_CELLS)));
      btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);
      eng->renderer->coverArea(renderArea_screen, coord(0, 2), MAP_X_CELLS, MAP_Y_CELLS);
      drawInterface();
      yCell = 2;
      for(int i = topElement; i <= btmElement; i++) {
        eng->renderer->drawText(lines.at(i).str , renderArea_screen, 1, yCell, lines.at(i).color);
        yCell++;
      }
      eng->renderer->updateWindow();
    }
    else if(d.key_ == '8' || d.sfmlKey_ == sf::Keyboard::Up) {
      topElement = max(0, min(topElement - static_cast<int>(MAP_Y_CELLS / 5), static_cast<int>(lines.size()) - static_cast<int>(MAP_Y_CELLS)));
      btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);
      eng->renderer->coverArea(renderArea_screen, coord(0, 2), MAP_X_CELLS, MAP_Y_CELLS);
      drawInterface();
      yCell = 2;
      for(int i = topElement; i <= btmElement; i++) {
        eng->renderer->drawText(lines.at(i).str , renderArea_screen, 1, yCell, lines.at(i).color);
        yCell++;
      }
      eng->renderer->updateWindow();
    }
    else if(d.sfmlKey_ == sf::Keyboard::Space || d.sfmlKey_ == sf::Keyboard::Escape) {
      done = true;
    }

  }
  eng->renderer->coverRenderArea(renderArea_screen);
  eng->renderer->drawMapAndInterface();
}
