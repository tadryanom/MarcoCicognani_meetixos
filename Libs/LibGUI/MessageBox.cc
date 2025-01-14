/**
 * @brief
 * This file is part of the MeetiX Operating System.
 * Copyright (c) 2017-2022, Marco Cicognani (marco.cicognani@meetixos.org)
 *
 * @developers
 * Marco Cicognani (marco.cicognani@meetixos.org)
 *
 * @license
 * GNU General Public License version 3
 */

#include <LibGUI/Application.hh>
#include <LibGUI/ButtonList.hh>
#include <LibGUI/MessageBox.hh>

/*
 * screen components
 */
static Window* box;
static Label*  message;

/*
 * library objects
 */
Graphics::Metrics::Dimension resolution;
ButtonList*                  btnControls;

// response for show with response
PressedButton response = BUTTON_NONE;
bool          waiter   = true;

/*
 * convert string length in window width
 */
static inline int getWidthFromStringLength(std::string s) {
    return s.length() * 10;
}

/*
 * garbage collector, called when show finish
 */
void garbageDelete() {
    // set unvisible the box
    box->set_visible(false);

    // delete the components
    delete btnControls;
    delete message;
    delete box;
}

/*
 * setup components
 */
void setupComponents(std::string msg, bool complete) {
    // get screen resolution and window width
    resolution   = UI::screen_dimension();
    int boxWidth = getWidthFromStringLength(msg);

    // instantiate and create window
    box = Window::create();
    box->on_close(garbageDelete);
    box->set_bounds(Graphics::Metrics::Rectangle((resolution.width() / 2) - (boxWidth / 2),
                                                 resolution.height() / 2 - 100,
                                                 boxWidth,
                                                 200));
    box->set_resizable(false);

    // instantiate and add label
    message = Label::create();
    message->set_bounds(Graphics::Metrics::Rectangle(10, 20, boxWidth, 30));

    // create okay button
    btnControls = new ButtonList();

    // if create all components instantiate also buttons
    if ( complete ) {
        btnControls->add("okay", [] {
            response = BUTTON_OKAY;
            waiter   = false;
        });
        btnControls->configure("okay",
                               Graphics::Metrics::Rectangle(boxWidth / 2 - 64, 110, 64, 30),
                               "Okay",
                               Graphics::Color::as_argb(180, 0, 200, 0),
                               Graphics::Color::as_rgb(0, 0, 0));

        // create cancel button
        btnControls->add("cancel", [] {
            response = BUTTON_CANCEL;
            waiter   = false;
        });
        btnControls->configure("cancel",
                               Graphics::Metrics::Rectangle(boxWidth / 2, 110, 64, 30),
                               "Cancel",
                               Graphics::Color::as_rgb(200, 0, 0),
                               Graphics::Color::as_rgb(0, 0, 0));
    }

    // create only the okay button
    else {
        btnControls->add("okay", [] { waiter = false; });
        btnControls->configure("okay",
                               Graphics::Metrics::Rectangle(boxWidth / 2 - 32, 110, 64, 30),
                               "Okay",
                               Graphics::Color::as_argb(180, 0, 200, 0),
                               Graphics::Color::as_rgb(0, 0, 0));
    }
}

/*
 * show components on screen
 */
void showComponents() {
    // show components on screen
    box->add_child(message);
    box->set_visible(true);

    // show buttons
    btnControls->show(box);
}

/*
 * show only the message in empty window
 */
void MessageBox::show(std::string msg) {
    // call component initialization
    setupComponents(msg, false);

    // set title of message
    message->set_title(msg);

    // show components
    showComponents();

    // wait response from box
    s_atomic_block(&waiter);

    // deallocate components
    garbageDelete();
}

/*
 * show the message in window titled with provided title
 */
void MessageBox::show(std::string msg, std::string title) {
    // call component initialization
    setupComponents(msg, false);

    // set title of message
    box->set_title(title);
    message->set_title(msg);

    // show components
    showComponents();

    // wait response from box
    s_atomic_block(&waiter);

    // deallocate components
    garbageDelete();
}

/*
 * show message with title and return pressed button
 */
PressedButton MessageBox::show_and_get_response(std::string msg, std::string title) {
    // call component initialization
    setupComponents(msg, true);

    // set title of message
    box->set_title(title);
    message->set_title(msg);

    // show components
    showComponents();

    // wait response from box
    while ( response == BUTTON_NONE )
        s_yield();

    // deallocate components
    garbageDelete();

    // return response
    return response;
}
