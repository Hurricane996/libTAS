/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBTAS_GAMEEVENTS_H_INCLUDED
#define LIBTAS_GAMEEVENTS_H_INCLUDED

#include <QtCore/QObject>
#include <memory>
#include <stdint.h>

#include "Context.h"
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

/* Forward declaration */
class MovieFile;

class GameEvents : public QObject {
    Q_OBJECT
public:
    GameEvents(Context *c, MovieFile* m);

    void init();

    /* Register and select events from the window handle */
    void registerGameWindow(uint32_t gameWindow);

    bool handleEvent();

    /* Determine if we are allowed to send inputs to the game, based on which
     * window has focus and our settings.
     */
    bool haveFocus();

private:
    Context* context;
    MovieFile* movie;

    /* Keyboard layout */
    std::unique_ptr<xcb_key_symbols_t, void(*)(xcb_key_symbols_t*)> keysyms;

    xcb_keycode_t last_pressed_key;
    xcb_generic_event_t *next_event;

    /* parent window of game window */
    xcb_window_t parent_game_window = 0;

    /* Frame advance auto-repeat variables.
     * If ar_ticks is >= 0 (auto-repeat activated), it increases by one every
     * iteration of the do loop.
     * If ar_ticks > ar_delay and ar_ticks % ar_freq == 0: trigger frame advance */
    int ar_ticks;
    int ar_delay;
    int ar_freq;

    enum EventType {
        EVENT_TYPE_NONE = 0,
        EVENT_TYPE_PRESS,
        EVENT_TYPE_RELEASE,
        EVENT_TYPE_FOCUS_OUT,
        EVENT_TYPE_EXPOSE,
    };

    void clearEventQueue();

    EventType nextEvent(struct HotKey &hk);

    void notifyControllerEvent(xcb_keysym_t ks, bool pressed);

    bool processEvent(EventType type, struct HotKey &hk);

signals:
    void alertToShow(QString str);
    void sharedConfigChanged();
    void askToShow(QString str, void* promise);

    void controllerButtonToggled(int controller_id, int button, bool pressed);

    /* Signals for notifying the input editor */
    void inputsToBeChanged();
    void inputsChanged();

    /* register a savestate */
    void savestatePerformed(int slot, unsigned long long frame);
};

#endif
