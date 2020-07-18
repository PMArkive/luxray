#include "time_error_screen.hpp"

#include "../debug.hpp"
#include "../ui/controller.hpp"
#include "main_screen.hpp"

TimeErrorScreen TimeErrorScreen::s_instance;

TimeErrorScreen::TimeErrorScreen() : ui::IScreen() {
    mp_errorMsgLabel = lv_label_create(getLvScreenObj(), nullptr);
    lv_label_set_text(mp_errorMsgLabel,
                      "To use the Date Advance\n"
                      "feature, turn on\n"
                      "\"Synchronize Clock via\n"
                      "Internet\" in System Settings.");
    lv_obj_align(mp_errorMsgLabel, nullptr, LV_ALIGN_CENTER, 0, 0);
    m_basicScreen.addLvObjPositionUpdater(mp_errorMsgLabel, lv_obj_realign);

    LOG("TimeErrorScreen initialized");
}

TimeErrorScreen::~TimeErrorScreen() {}

void TimeErrorScreen::renderScreen() { m_basicScreen.renderScreen(); }

void TimeErrorScreen::procFrame() {
    if (m_basicScreen.returnButtonPressed()) {
        ui::Controller::show(MainScreen::getInstance());
    }
}
