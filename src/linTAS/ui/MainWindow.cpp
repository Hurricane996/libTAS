/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "MainWindow.h"
#include "../main.h"
#include "../MovieFile.h"
#include <iostream>
#include <FL/x.H>

void MainWindow::build(Context* c)
{
    context = c;
    window = new Fl_Double_Window(600, 400);

    /* Menu */
    menu_bar = new Fl_Menu_Bar(0, 0, window->w(), 30);
    menu_bar->menu(menu_items);

    /* Game Executable */
    gamepath = new Fl_Output(10, 300, 500, 30, "Game Executable");
    gamepath->align(FL_ALIGN_TOP_LEFT);
    gamepath->color(FL_LIGHT1);

    browsegamepath = new Fl_Button(520, 300, 70, 30, "Browse...");
    browsegamepath->callback(browse_gamepath_cb);

    gamepathchooser = new Fl_Native_File_Chooser();
    gamepathchooser->title("Game path");

    /* Movie File */
    moviepath = new Fl_Output(10, 50, 500, 30, "Movie File");
    moviepath->align(FL_ALIGN_TOP_LEFT);
    moviepath->color(FL_LIGHT1);

    moviepathchooser = new Fl_Native_File_Chooser();
    moviepathchooser->title("Choose a movie file");
    moviepathchooser->filter("libTAS movie file \t*.ltm\n");

    browsemoviepath = new Fl_Button(520, 50, 70, 30, "Browse...");
    browsemoviepath->callback(browse_moviepath_cb);

    /* Movie File Status */
    moviepack = new Fl_Pack(10, 90, window->w()-10, 30);
    moviepack->type(Fl_Pack::HORIZONTAL);
    moviepack->box(FL_ENGRAVED_FRAME);
    movie_norec = new Fl_Radio_Round_Button(0, 0, 130, 0, "No recording");
    movie_norec->callback(recording_cb);
    movie_w = new Fl_Radio_Round_Button(0, 0, 130, 0, "Overwrite");
    movie_w->callback(recording_cb);
    movie_rw = new Fl_Radio_Round_Button(0, 0, 130, 0, "Read/Write");
    movie_rw->callback(recording_cb);
    movie_ro = new Fl_Radio_Round_Button(0, 0, 130, 0, "Read Only");
    movie_ro->callback(recording_cb);
    moviepack->end();

    /* Frames per second */
    logicalfps = new Fl_Int_Input(160, 200, 40, 30, "Frames per second");
    logicalfps->callback(set_fps_cb);

    /* Pause/FF */
    pausecheck = new Fl_Check_Button(240, 140, 80, 20, "Pause");
    pausecheck->callback(pause_cb);
    fastforwardcheck = new Fl_Check_Button(240, 180, 80, 20, "Fast-forward");
    fastforwardcheck->callback(fastforward_cb);

    /* Mute */
    mutecheck = new Fl_Check_Button(240, 220, 80, 20, "Mute");
    mutecheck->callback(mute_sound_cb);

    /* Frame count */
    framecount = new Fl_Output(80, 140, 60, 30, "Frames:");
    framecount->value("0");
    framecount->color(FL_LIGHT1);

    totalframecount = new Fl_Output(140, 140, 60, 30);
    totalframecount->color(FL_LIGHT1);
    MovieFile movie;
    std::string totalframestr = std::to_string(movie.nbFrames(context->config.moviefile));
    totalframecount->value(totalframestr.c_str());

    launch = new Fl_Button(10, 350, 70, 40, "Start");
    launch->callback(launch_cb);

    update(true);
    update_config();

    window->end();

    encode_window = new EncodeWindow(c);
    input_window = new InputWindow(c);

    window->show();
}

Fl_Menu_Item MainWindow::menu_items[] = {
    {"File", 0, nullptr, nullptr, FL_SUBMENU},
        {"Open Executable...", 0, browse_gamepath_cb},
        {"Open Movie...", 0, browse_moviepath_cb},
        {nullptr},
    {"Sound", 0, nullptr, nullptr, FL_SUBMENU},
        {"Format", 0, nullptr, nullptr, FL_SUBMENU},
            {"8000 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(8000), FL_MENU_RADIO},
            {"11025 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(11025), FL_MENU_RADIO},
            {"12000 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(12000), FL_MENU_RADIO},
            {"16000 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(16000), FL_MENU_RADIO},
            {"22050 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(22050), FL_MENU_RADIO},
            {"24000 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(24000), FL_MENU_RADIO},
            {"32000 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(32000), FL_MENU_RADIO},
            {"44100 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(44100), FL_MENU_RADIO},
            {"48000 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(48000), FL_MENU_RADIO | FL_MENU_DIVIDER},
            {"8 bit", 0, sound_bitdepth_cb, reinterpret_cast<void*>(8), FL_MENU_RADIO},
            {"16 bit", 0, sound_bitdepth_cb, reinterpret_cast<void*>(16), FL_MENU_RADIO | FL_MENU_DIVIDER},
            {"Mono", 0, sound_channel_cb, reinterpret_cast<void*>(1), FL_MENU_RADIO},
            {"Stereo", 0, sound_channel_cb, reinterpret_cast<void*>(2), FL_MENU_RADIO},
            {nullptr},
        {"Mute Sound", 0, mute_sound_cb, nullptr, FL_MENU_TOGGLE},
        {nullptr},
    {"Runtime", 0, nullptr, nullptr, FL_SUBMENU},
        {"Debug Logging", 0, nullptr, nullptr, FL_SUBMENU},
            {"Disabled", 0, logging_status_cb, reinterpret_cast<void*>(SharedConfig::NO_LOGGING), FL_MENU_RADIO},
            {"Log to console", 0, logging_status_cb, reinterpret_cast<void*>(SharedConfig::LOGGING_TO_CONSOLE), FL_MENU_RADIO},
            {"Log to file", 0, logging_status_cb, reinterpret_cast<void*>(SharedConfig::LOGGING_TO_FILE), FL_MENU_RADIO | FL_MENU_DIVIDER},
            {"Print Categories", 0, nullptr, nullptr, FL_SUBMENU},
                {"Untested", 0, logging_print_cb, reinterpret_cast<void*>(LCF_UNTESTED), FL_MENU_TOGGLE},
                {"Desync", 0, logging_print_cb, reinterpret_cast<void*>(LCF_DESYNC), FL_MENU_TOGGLE},
                {"Frequent", 0, logging_print_cb, reinterpret_cast<void*>(LCF_FREQUENT), FL_MENU_TOGGLE},
                {"Error", 0, logging_print_cb, reinterpret_cast<void*>(LCF_ERROR), FL_MENU_TOGGLE},
                {"ToDo", 0, logging_print_cb, reinterpret_cast<void*>(LCF_TODO), FL_MENU_TOGGLE},
                {"Frame", 0, logging_print_cb, reinterpret_cast<void*>(LCF_FRAME), FL_MENU_TOGGLE | FL_MENU_DIVIDER},
                {"Hook", 0, logging_print_cb, reinterpret_cast<void*>(LCF_HOOK), FL_MENU_TOGGLE},
                {"Time", 0, logging_print_cb, reinterpret_cast<void*>(LCF_TIMEFUNC), FL_MENU_TOGGLE},
                {"Time Set", 0, logging_print_cb, reinterpret_cast<void*>(LCF_TIMESET), FL_MENU_TOGGLE},
                {"Time Get", 0, logging_print_cb, reinterpret_cast<void*>(LCF_TIMEGET), FL_MENU_TOGGLE},
                {"Wait", 0, logging_print_cb, reinterpret_cast<void*>(LCF_WAIT), FL_MENU_TOGGLE},
                {"Sleep", 0, logging_print_cb, reinterpret_cast<void*>(LCF_SLEEP), FL_MENU_TOGGLE},
                {"Socket", 0, logging_print_cb, reinterpret_cast<void*>(LCF_SOCKET), FL_MENU_TOGGLE},
                {"OpenGL", 0, logging_print_cb, reinterpret_cast<void*>(LCF_OGL), FL_MENU_TOGGLE},
                {"AV Dumping", 0, logging_print_cb, reinterpret_cast<void*>(LCF_DUMP), FL_MENU_TOGGLE},
                {"SDL", 0, logging_print_cb, reinterpret_cast<void*>(LCF_SDL), FL_MENU_TOGGLE},
                {"Memory", 0, logging_print_cb, reinterpret_cast<void*>(LCF_MEMORY), FL_MENU_TOGGLE},
                {"Keyboard", 0, logging_print_cb, reinterpret_cast<void*>(LCF_KEYBOARD), FL_MENU_TOGGLE},
                {"Mouse", 0, logging_print_cb, reinterpret_cast<void*>(LCF_MOUSE), FL_MENU_TOGGLE},
                {"Joystick", 0, logging_print_cb, reinterpret_cast<void*>(LCF_JOYSTICK), FL_MENU_TOGGLE},
                {"OpenAL", 0, logging_print_cb, reinterpret_cast<void*>(LCF_OPENAL), FL_MENU_TOGGLE},
                {"Sound", 0, logging_print_cb, reinterpret_cast<void*>(LCF_SOUND), FL_MENU_TOGGLE},
                {"Events", 0, logging_print_cb, reinterpret_cast<void*>(LCF_EVENTS), FL_MENU_TOGGLE},
                {"Windows", 0, logging_print_cb, reinterpret_cast<void*>(LCF_WINDOW), FL_MENU_TOGGLE},
                {"File IO", 0, logging_print_cb, reinterpret_cast<void*>(LCF_FILEIO), FL_MENU_TOGGLE},
                {"Steam", 0, logging_print_cb, reinterpret_cast<void*>(LCF_STEAM), FL_MENU_TOGGLE},
                {"Threads", 0, logging_print_cb, reinterpret_cast<void*>(LCF_THREAD), FL_MENU_TOGGLE},
                {"Timers", 0, logging_print_cb, reinterpret_cast<void*>(LCF_TIMERS), FL_MENU_TOGGLE | FL_MENU_DIVIDER},
                {"All", 0, logging_print_cb, reinterpret_cast<void*>(LCF_ALL)},
                {"None", 0, logging_print_cb, reinterpret_cast<void*>(LCF_NONE)},
                {nullptr},
            {"Exclude Categories", 0, nullptr, nullptr, FL_SUBMENU},
                {"Untested", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_UNTESTED), FL_MENU_TOGGLE},
                {"Desync", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_DESYNC), FL_MENU_TOGGLE},
                {"Frequent", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_FREQUENT), FL_MENU_TOGGLE},
                {"Error", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_ERROR), FL_MENU_TOGGLE},
                {"ToDo", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_TODO), FL_MENU_TOGGLE},
                {"Frame", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_FRAME), FL_MENU_TOGGLE | FL_MENU_DIVIDER},
                {"Hook", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_HOOK), FL_MENU_TOGGLE},
                {"Time", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_TIMEFUNC), FL_MENU_TOGGLE},
                {"Time Set", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_TIMESET), FL_MENU_TOGGLE},
                {"Time Get", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_TIMEGET), FL_MENU_TOGGLE},
                {"Wait", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_WAIT), FL_MENU_TOGGLE},
                {"Sleep", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_SLEEP), FL_MENU_TOGGLE},
                {"Socket", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_SOCKET), FL_MENU_TOGGLE},
                {"OpenGL", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_OGL), FL_MENU_TOGGLE},
                {"AV Dumping", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_DUMP), FL_MENU_TOGGLE},
                {"SDL", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_SDL), FL_MENU_TOGGLE},
                {"Memory", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_MEMORY), FL_MENU_TOGGLE},
                {"Keyboard", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_KEYBOARD), FL_MENU_TOGGLE},
                {"Mouse", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_MOUSE), FL_MENU_TOGGLE},
                {"Joystick", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_JOYSTICK), FL_MENU_TOGGLE},
                {"OpenAL", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_OPENAL), FL_MENU_TOGGLE},
                {"Sound", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_SOUND), FL_MENU_TOGGLE},
                {"Events", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_EVENTS), FL_MENU_TOGGLE},
                {"Windows", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_WINDOW), FL_MENU_TOGGLE},
                {"File IO", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_FILEIO), FL_MENU_TOGGLE},
                {"Steam", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_STEAM), FL_MENU_TOGGLE},
                {"Threads", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_THREAD), FL_MENU_TOGGLE},
                {"Timers", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_TIMERS), FL_MENU_TOGGLE | FL_MENU_DIVIDER},
                {"All", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_ALL)},
                {"None", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_NONE)},
                {nullptr},
            {nullptr},
        {nullptr},
    {"Tools", 0, nullptr, nullptr, FL_SUBMENU},
        {"Configure encode...", 0, config_encode_cb},
        {"Start encode", 0, toggle_encode_cb, nullptr, FL_MENU_DIVIDER},
        {"Slow Motion", 0, nullptr, nullptr, FL_SUBMENU},
            {"100% (normal speed)", 0, slowmo_cb, reinterpret_cast<void*>(1), FL_MENU_RADIO},
            {"50%", 0, slowmo_cb, reinterpret_cast<void*>(2), FL_MENU_RADIO},
            {"25%", 0, slowmo_cb, reinterpret_cast<void*>(4), FL_MENU_RADIO},
            {"12%", 0, slowmo_cb, reinterpret_cast<void*>(8), FL_MENU_RADIO},
            {nullptr},
        {nullptr},
    {"Input", 0, nullptr, nullptr, FL_SUBMENU},
        {"Configure mapping...", 0, config_input_cb, nullptr, FL_MENU_DIVIDER},
        {"Keyboard support", 0, input_keyboard_cb, nullptr, FL_MENU_TOGGLE},
        {"Mouse support", 0, input_mouse_cb, nullptr, FL_MENU_TOGGLE},
        {"Joystick support", 0, nullptr, nullptr, FL_SUBMENU | FL_MENU_DIVIDER},
            {"None", 0, input_joy_cb, reinterpret_cast<void*>(0), FL_MENU_RADIO},
            {"1", 0, input_joy_cb, reinterpret_cast<void*>(1), FL_MENU_RADIO},
            {"2", 0, input_joy_cb, reinterpret_cast<void*>(2), FL_MENU_RADIO},
            {"3", 0, input_joy_cb, reinterpret_cast<void*>(3), FL_MENU_RADIO},
            {"4", 0, input_joy_cb, reinterpret_cast<void*>(4), FL_MENU_RADIO},
            {nullptr},
        {"Enable inputs when", 0, nullptr, nullptr, FL_SUBMENU},
            {"Game has focus", 0, input_focus_game_cb, nullptr, FL_MENU_VALUE | FL_MENU_TOGGLE},
            {"UI has focus", 0, input_focus_ui_cb, nullptr, FL_MENU_TOGGLE},
            {nullptr},
        {nullptr},
    {nullptr}
};

void MainWindow::update_status()
{
    /* Update game status (active/inactive) */

    /* This function might be called by another thread */
    Fl::lock();

    Fl_Menu_Item *item;

    switch (context->status) {
        case Context::INACTIVE:
            launch->label("Start");
            launch->activate();
            moviepath->activate();
            browsemoviepath->activate();
            gamepath->activate();
            browsegamepath->activate();
            logicalfps->activate();
            moviepack->activate();
            item = const_cast<Fl_Menu_Item*>(menu_bar->find_item("Sound/Format"));
            if (item) item->activate();
            if (context->config.sc.av_dumping) {
                Fl_Menu_Item* encode_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(toggle_encode_cb));
                Fl_Menu_Item* config_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(config_encode_cb));
                context->config.sc.av_dumping = false;
                if (encode_item) encode_item->label("Start encode");
                if (config_item) config_item->activate();
            }
            break;
        case Context::STARTING:
            launch->deactivate();
            moviepath->deactivate();
            browsemoviepath->deactivate();
            gamepath->deactivate();
            browsegamepath->deactivate();
            logicalfps->deactivate();
            moviepack->deactivate();
            item = const_cast<Fl_Menu_Item*>(menu_bar->find_item("Sound/Format"));
            if (item) item->deactivate();
            break;
        case Context::ACTIVE:
            launch->activate();
            launch->label("Stop");
            break;
        case Context::QUITTING:
            launch->deactivate();
            break;
        default:
            break;
    }

    Fl::unlock();
    Fl::awake();
}

void MainWindow::update(bool status)
{
    /* This function is called by another thread */
    Fl::lock();

    /* Update frame count */
    std::string framestr = std::to_string(context->framecount);
    framecount->value(framestr.c_str());
    if (context->recording == Context::RECORDING_WRITE) {
        std::string totalframestr = std::to_string(context->framecount - 1);
        totalframecount->value(totalframestr.c_str());
    }

    if (status) {
        /* Update pause status */
        pausecheck->value(!context->config.sc.running);

        /* Update fastforward status */
        fastforwardcheck->value(context->config.sc.fastforward);

        /* Update recording state */
        switch (context->recording) {
          case Context::NO_RECORDING:
              movie_norec->setonly();
              break;
          case Context::RECORDING_WRITE:
              movie_w->setonly();
              break;
          case Context::RECORDING_READ_WRITE:
              movie_rw->setonly();
              break;
          case Context::RECORDING_READ_ONLY:
              movie_ro->setonly();
              break;
        }
    }

    Fl::unlock();
    Fl::awake();
}

void MainWindow::update_config()
{
    gamepath->value(context->gamepath.c_str());
    gamepathchooser->preset_file(context->gamepath.c_str());
    moviepath->value(context->config.moviefile.c_str());
    moviepathchooser->preset_file(context->config.moviefile.c_str());
    std::string fpsstr = std::to_string(context->config.sc.framerate);
    logicalfps->value(fpsstr.c_str());

    Fl_Menu_Item* sound_freq_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(sound_frequency_cb));
    while (sound_freq_item) {
        if (sound_freq_item->argument() == context->config.sc.audio_frequency) {
            sound_freq_item->setonly();
            break;
        }
        sound_freq_item = sound_freq_item->next();
    }

    Fl_Menu_Item* sound_bitdepth_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(sound_bitdepth_cb));
    while (sound_bitdepth_item) {
        if (sound_bitdepth_item->argument() == context->config.sc.audio_bitdepth) {
            sound_bitdepth_item->setonly();
            break;
        }
        sound_bitdepth_item = sound_bitdepth_item->next();
    }

    Fl_Menu_Item* sound_channel_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(sound_channel_cb));
    while (sound_channel_item) {
        if (sound_channel_item->argument() == context->config.sc.audio_channels) {
            sound_channel_item->setonly();
            break;
        }
        sound_channel_item = sound_channel_item->next();
    }

    Fl_Menu_Item* sound_mute_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(mute_sound_cb));
    if (context->config.sc.audio_mute) {
        sound_mute_item->set();
        mutecheck->set();
    }
    else {
        sound_mute_item->clear();
        mutecheck->clear();
    }

    Fl_Menu_Item* logging_status_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(logging_status_cb));
    while (logging_status_item) {
        if (static_cast<SharedConfig::LogStatus>(logging_status_item->argument()) == context->config.sc.logging_status) {
            logging_status_item->setonly();
            break;
        }
        logging_status_item = logging_status_item->next();
    }

    Fl_Menu_Item* logging_print_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(logging_print_cb));
    while (logging_print_item && (logging_print_item->argument() != LCF_ALL)) {
        if (logging_print_item->argument() & context->config.sc.includeFlags)
            logging_print_item->set();
        else
            logging_print_item->clear();
      logging_print_item = logging_print_item->next();
    }

    Fl_Menu_Item* logging_exclude_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(logging_exclude_cb));
    while (logging_exclude_item && (logging_exclude_item->argument() != LCF_ALL)) {
        if (logging_exclude_item->argument() & context->config.sc.excludeFlags)
            logging_exclude_item->set();
        else
            logging_exclude_item->clear();
      logging_exclude_item = logging_exclude_item->next();
    }

    Fl_Menu_Item* speed_divisor_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(slowmo_cb));
    while (speed_divisor_item) {
      if (speed_divisor_item->argument() == context->config.sc.speed_divisor) {
          speed_divisor_item->setonly();
          break;
      }
      speed_divisor_item = speed_divisor_item->next();
    }

    Fl_Menu_Item* input_keyboard_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(input_keyboard_cb));
    if (context->config.sc.keyboard_support)
        input_keyboard_item->set();
    else
        input_keyboard_item->clear();

    Fl_Menu_Item* input_mouse_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(input_mouse_cb));
    if (context->config.sc.mouse_support)
        input_mouse_item->set();
    else
        input_mouse_item->clear();

    Fl_Menu_Item* input_joy_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(input_joy_cb));
    while (input_joy_item) {
        if (input_joy_item->argument() == context->config.sc.numControllers) {
            input_joy_item->setonly();
            break;
        }
        input_joy_item = input_joy_item->next();
    }
}

void launch_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();

    switch (mw.context->status) {
        case Context::INACTIVE:
            /* Check that there might be a thread from a previous game execution */
            if (mw.game_thread.joinable())
                mw.game_thread.join();

            /* Start game */
            mw.context->status = Context::STARTING;
            mw.update_status();
            mw.game_thread = std::thread{launchGame, nullptr};
            break;
        case Context::ACTIVE:
            mw.context->status = Context::QUITTING;
            mw.update_status();
            mw.game_thread.detach();
            break;
        default:
            break;
    }
}

void browse_gamepath_cb(Fl_Widget* w, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    int ret = mw.gamepathchooser->show();

    const char* filename = mw.gamepathchooser->filename();

    /* If the user picked a file */
    if (ret == 0) {
        mw.gamepath->value(filename);
        mw.context->gamepath = std::string(filename);

        /* Try to load the game-specific pref file */
        mw.context->config.init(mw.context->gamepath);

        /* Set a default movie file if any */
        if (mw.context->config.moviefile.empty()) {
            mw.context->config.moviefile = mw.context->gamepath;
            mw.context->config.moviefile += ".ltm";
        }

        /* Update the UI accordingly */
        mw.update_config();
        mw.encode_window->update_config();
    }
}

void browse_moviepath_cb(Fl_Widget* w, void*)
{
    // TODO: Almost duplicate of browse_gamepath_cb...
    MainWindow& mw = MainWindow::getInstance();
    int ret = mw.moviepathchooser->show();

    const char* filename = mw.moviepathchooser->filename();

    /* If the user picked a file */
    if (ret == 0) {
        mw.moviepath->value(filename);
        mw.context->config.moviefile = std::string(filename);
        MovieFile movie;
        std::string totalframestr = std::to_string(movie.nbFrames(mw.context->config.moviefile));
        mw.totalframecount->value(totalframestr.c_str());
    }
}

void set_fps_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Int_Input *ii = (Fl_Int_Input*) w;
    std::string fpsstr = ii->value();
    mw.context->config.sc.framerate = std::stoi(fpsstr);
}

void pause_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Check_Button *cb = (Fl_Check_Button*) w;
    int cb_val = static_cast<int>(cb->value());
    mw.context->config.sc.running = !cb_val;
    mw.context->config.sc_modified = true;
}

void fastforward_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Check_Button *cb = (Fl_Check_Button*) w;
    int cb_val = static_cast<int>(cb->value());
    mw.context->config.sc.fastforward = cb_val;
    mw.context->config.sc_modified = true;
}

void recording_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();
    if (mw.movie_norec->value() == 1)
        mw.context->recording = Context::NO_RECORDING;
    if (mw.movie_w->value() == 1)
        mw.context->recording = Context::RECORDING_WRITE;
    if (mw.movie_rw->value() == 1)
        mw.context->recording = Context::RECORDING_READ_WRITE;
    if (mw.movie_ro->value() == 1)
        mw.context->recording = Context::RECORDING_READ_ONLY;
}

void config_encode_cb(Fl_Widget* w, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    mw.encode_window->window->show();

    while (mw.encode_window->window->shown()) {
        Fl::wait();
    }
}

void toggle_encode_cb(Fl_Widget* w, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    /* Set encode status */
    Fl_Menu_Item* encode_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->mvalue());
    Fl_Menu_Item* config_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item(config_encode_cb));

    if (!mw.context->config.sc.av_dumping) {
        mw.context->config.sc.av_dumping = true;
        mw.context->config.sc_modified = true;
        mw.context->config.dumpfile_modified = true;
        encode_item->label("Stop encode");
        if (config_item) config_item->deactivate();
    }
    else {
        mw.context->config.sc.av_dumping = false;
        mw.context->config.sc_modified = true;
        encode_item->label("Start encode");
        if (config_item) config_item->activate();
    }
}

void config_input_cb(Fl_Widget* w, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    mw.input_window->window->show();

    while (mw.input_window->window->shown()) {
        Fl::wait();
    }
}

void sound_frequency_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int frequency = static_cast<int>(reinterpret_cast<intptr_t>(v));
    mw.context->config.sc.audio_frequency = frequency;
}

void sound_bitdepth_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int bitdepth = static_cast<int>(reinterpret_cast<intptr_t>(v));
    mw.context->config.sc.audio_bitdepth = bitdepth;
}

void sound_channel_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int channel = static_cast<int>(reinterpret_cast<intptr_t>(v));
    mw.context->config.sc.audio_channels = channel;
}

void mute_sound_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Menu_Item* mute_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item(mute_sound_cb));

    if (mw.context->config.sc.audio_mute) {
        if (mute_item) mute_item->clear();
        mw.context->config.sc.audio_mute = false;
        mw.context->config.sc_modified = true;
    }
    else {
        if (mute_item) mute_item->set();
        mw.context->config.sc.audio_mute = true;
        mw.context->config.sc_modified = true;
    }
    mw.mutecheck->value(mw.context->config.sc.audio_mute);
}

void logging_status_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    SharedConfig::LogStatus logstatus = static_cast<SharedConfig::LogStatus>(reinterpret_cast<intptr_t>(v));
    mw.context->config.sc.logging_status = logstatus;
}

void logging_print_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    LogCategoryFlag logcat = static_cast<LogCategoryFlag>(reinterpret_cast<intptr_t>(v));
    if (logcat == LCF_ALL) {
        /* Get the first item of the log categories */
        Fl_Menu_Item* log_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item(logging_print_cb));
        while (log_item->argument() != LCF_ALL) {
            log_item->set();
            log_item = log_item->next();
        }
        mw.context->config.sc.includeFlags = LCF_ALL;
    }
    else if (logcat == LCF_NONE) {
        /* Get the first item of the log categories */
        Fl_Menu_Item* log_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item(logging_print_cb));
        while (log_item->argument() != LCF_ALL) {
            log_item->clear();
            log_item = log_item->next();
        }
        mw.context->config.sc.includeFlags = LCF_NONE;
    }
    else {
        mw.context->config.sc.includeFlags ^= logcat;
    }
}

void logging_exclude_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    LogCategoryFlag logcat = static_cast<LogCategoryFlag>(reinterpret_cast<intptr_t>(v));
    if (logcat == LCF_ALL) {
        /* Get the first item of the log categories */
        Fl_Menu_Item* log_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item(logging_exclude_cb));
        while (log_item->argument() != LCF_ALL) {
            log_item->set();
            log_item = log_item->next();
        }
        mw.context->config.sc.excludeFlags = LCF_ALL;
    }
    else if (logcat == LCF_NONE) {
        /* Get the first item of the log categories */
        Fl_Menu_Item* log_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item(logging_exclude_cb));
        while (log_item->argument() != LCF_ALL) {
            log_item->clear();
            log_item = log_item->next();
        }
        mw.context->config.sc.excludeFlags = LCF_NONE;
    }
    else {
        mw.context->config.sc.excludeFlags ^= logcat;
    }
}

void input_keyboard_cb(Fl_Widget*, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Menu_Item* keyboard_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->mvalue());

    if (keyboard_item && (keyboard_item->value() == 0)) {
        mw.context->config.sc.keyboard_support = false;
    }
    else {
        mw.context->config.sc.keyboard_support = true;
    }
}

void input_mouse_cb(Fl_Widget*, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Menu_Item* mouse_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->mvalue());

    if (mouse_item && (mouse_item->value() == 0)) {
        mw.context->config.sc.mouse_support = false;
    }
    else {
        mw.context->config.sc.mouse_support = true;
    }
}

void input_joy_cb(Fl_Widget*, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int nb_joy = static_cast<int>(reinterpret_cast<intptr_t>(v));

    mw.context->config.sc.numControllers = nb_joy;
}

void input_focus_game_cb(Fl_Widget*, void* v)
{
    MainWindow& mw = MainWindow::getInstance();

    /* If the game was not launched, don't do anything */
    // TODO: We could save this in the context
    if (! mw.context->game_window ) return;

    Fl_Menu_Item* focus_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->mvalue());

    if (focus_item->value()) {
        XSelectInput(mw.context->display, mw.context->game_window, KeyPressMask | KeyReleaseMask | FocusChangeMask);
    }
    else {
        XSelectInput(mw.context->display, mw.context->game_window, FocusChangeMask);
    }
}

void input_focus_ui_cb(Fl_Widget*, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Menu_Item* focus_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->mvalue());

    Window main_ui = fl_xid(mw.window);

    if (focus_item->value()) {
        XSelectInput(mw.context->display, main_ui, KeyPressMask | KeyReleaseMask | FocusChangeMask);
    }
    else {
        XSelectInput(mw.context->display, main_ui, FocusChangeMask);
    }
}

void slowmo_cb(Fl_Widget*, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int spdiv = static_cast<int>(reinterpret_cast<intptr_t>(v));

    mw.context->config.sc.speed_divisor = spdiv;
    mw.context->config.sc_modified = true;
}
