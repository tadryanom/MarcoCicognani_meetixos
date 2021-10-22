/**
 * @brief
 * This file is part of the MeetiX Operating System.
 * Copyright (c) 2017-2021, Marco Cicognani (marco.cicognani@meetixos.org)
 *
 * @developers
 * Marco Cicognani (marco.cicognani@meetixos.org)
 *
 * @license
 * GNU General Public License version 3
 */

#pragma once

#include "../../../../Toolchain/Local/i686-pc-meetix/include/c++/11.2.0/cstdint"
#include "ActionComponent.hh"
#include "ColoredComponent.hh"
#include "Component.hh"
#include "ImageComponent.hh"
#include "TitledComponent.hh"

/**
 *
 */
class Button : public Component,
               public TitledComponent,
               public ActionComponent,
               public ImageComponent,
               public ColoredComponent {
protected:
    /*
     *
     */
    Button(uint32_t id)
        : Component(id, UI_COMPONENT_TYPE_BUTTON), TitledComponent(id), ActionComponent(this, id),
          ImageComponent(id), ColoredComponent(id) {
    }

public:
    ~Button() override = default;

    static Button* create();

    void set_enabled(bool enabled);
    bool is_enabled();
};
