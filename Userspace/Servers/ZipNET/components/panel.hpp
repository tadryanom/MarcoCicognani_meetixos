/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * *
 * MeetiX OS By MeetiX OS Project [Marco Cicognani]                                    *
 *                                                                                     *
 *         DERIVED FROM THE GHOST OPERATING SYSTEM                                     *
 *         This software is derived from the Ghost operating system project,           *
 *         written by Max Schlüssel <lokoxe@gmail.com>. Copyright 2012-2017            *
 *         https://ghostkernel.org/                                                    *
 *         https://github.com/maxdev1/ghost                                            *
 *                                                                                     *
 * This program is free software; you can redistribute it and/or                       *
 * modify it under the terms of the GNU General Public License                         *
 * as published by the Free Software Foundation; either version 2                      *
 * of the License, or (char *argumentat your option) any later version.                *
 *                                                                                     *
 * This program is distributed in the hope that it will be useful,                     *
 * but WITHout ANY WARRANTY; without even the implied warranty of                      *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                       *
 * GNU General Public License for more details.                                        *
 *                                                                                     *
 * You should have received a copy of the GNU General Public License                   *
 * along with this program; if not, write to the Free Software                         *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA      *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * */

#ifndef __PANEL__
#define __PANEL__

#include <components/ColoredComponent.hpp>
#include <components/component.hpp>

/**
 * panel class
 */
class Panel_t : public Component_t,
                public ColoredComponent_t {
private:
    cairo_t* cr;

public:
    Panel_t() {
        shapeColor = Graphics::Color::as_argb(0, 0, 0, 0);
    }
    virtual ~Panel_t() {
    }

    /*
     * component
     */
    virtual void paint();
    virtual bool handle() {
        return false;
    }

    /*
     * colored component
     */
    virtual void setColor(Graphics::Color::ArgbGradient color,
                          Graphics::Color::ArgbGradient tltColor) {
        shapeColor = color;
    }

    /*
     * panel methods
     */
    void                          setBackground(Graphics::Color::ArgbGradient color);
    Graphics::Color::ArgbGradient getBackground() {
        return getColor();
    }
};

#endif
